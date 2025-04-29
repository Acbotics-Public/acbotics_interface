/*******************************************************************/
/*    NAME: Oscar Viquez                                           */
/*    ORG:  Acbotics Research, LLC                                 */
/*    FILE: FFT.cpp                                                */
/*    DATE: May 21th 2024                                          */
/*                                                                 */
/*    For help, contact us at: support@acbotics.com                */
/*******************************************************************/

#include <arpa/inet.h>
#include <cstring>
#include <fstream>
#include <gflags/gflags.h>
#include <glog/logging.h>
#include <iomanip>
#include <pthread.h>
#include <sys/socket.h>
#include <vector>

#include "pocketfft_hdronly.h"

// includes from within project
#include "utils/FFT.h"

DEFINE_bool(debug_fft, false, "Enable expanded debug for FFT");

void *FFT::_run_fft_thread(void *ptr) {

  FFT *argPtr = static_cast<FFT *>(ptr);
  prctl(PR_SET_NAME, argPtr->thread_name.substr(0, 15).c_str());
  VLOG(3) << "Starting FFT utility in thread " << pthread_self();
  argPtr->_is_running = true;

  std::shared_ptr<UdpAcousticData> aco_pkt;
  // size_t num_channels;
  size_t icol;
  size_t ncols;

  int32_t packet_num = 0;

  double audio_scale = 1.0 / pow(2, 15) * argPtr->adc_scale;

  Eigen::MatrixXd data_buffer;
  Eigen::Matrix<int64_t, Eigen::Dynamic, 1> data_timestamps;

  Eigen::MatrixXd _fft_data_in(0, 0);
  std::shared_ptr<IpcFFT> fft_frame;

  aco_pkt = argPtr->q_aco->pop();

  bool use_ch_filter = argPtr->use_channel_filter && argPtr->channel_filter.size() > 0;

  argPtr->num_channels =
      use_ch_filter ? argPtr->channel_filter.size() : aco_pkt->header.num_channels;

  if (use_ch_filter) {
    data_buffer = (aco_pkt->data(argPtr->channel_filter, Eigen::all).cast<double>() * audio_scale);
  } else {
    data_buffer = (aco_pkt->data.cast<double>() * audio_scale);
  }

  data_timestamps =
      Eigen::Matrix<int64_t, Eigen::Dynamic, 1>::LinSpaced(
          aco_pkt->data.cols(), 0, 1e9 / aco_pkt->header.sample_rate * (aco_pkt->data.cols() - 1))
          .array() +
      aco_pkt->header.start_time_nsec;

  // aco_pkt->header.sample_rate is a float; make sure we cast
  if (argPtr->sample_rate != (double)aco_pkt->header.sample_rate) {
    argPtr->sample_rate = (double)aco_pkt->header.sample_rate;
    LOG(INFO) << "Sampling rate initialized: " << argPtr->sample_rate << " Hz";
  }

  // Note that win is 2D for element-wise scaling across all channels
  Eigen::ArrayXXd win = get_hann(argPtr->num_channels, argPtr->NFFT);

  // Scaling modes:
  // PSD : 2 / (Fs * sum(win*win) )
  // Spectrum : 2 / (sum(win)^2)
  // >> adapted to directly yield re 1V or re 1uPa: sqrt(2) / sum(win)
  //
  double scale_coeff = std::sqrt(2 / (argPtr->sample_rate * (win.row(0) * win.row(0)).sum()));
  // double scale_coeff = std::sqrt(2 / ( pow(win.row(0).sum(),2)));
  // double scale_coeff = std::sqrt(2) / win.row(0).sum();
  win *= scale_coeff;

  const pocketfft::shape_t shape_{static_cast<size_t>(argPtr->num_channels),
                                  static_cast<size_t>(argPtr->NFFT)};

  const pocketfft::stride_t stride_in_{sizeof(double),
                                       (ptrdiff_t)(sizeof(double) * argPtr->num_channels)};
  const pocketfft::stride_t stride_out_{
      sizeof(std::complex<double>),
      (ptrdiff_t)(sizeof(std::complex<double>) * argPtr->num_channels)};

  std::vector<std::shared_ptr<UdpAcousticData>> new_packets;

  size_t offset;

  while (argPtr->keep_alive) {

    if (argPtr->_rx_runtime_update) {
      audio_scale = 1.0 / pow(2, 15) * argPtr->adc_scale;
    }

    if (argPtr->q_aco->size() > 0) {

      new_packets = argPtr->q_aco->pop_limit(10);
      if (FLAGS_debug_fft)
        VLOG(3) << "New ACO packet count : " << new_packets.size();
      ncols = 0;
      for (int ii = 0; ii < new_packets.size(); ii++) {
        ncols += new_packets[ii]->data.cols();
      }

      icol = data_buffer.cols();
      if (FLAGS_debug_fft)
        VLOG(4) << "Expading buffer of size " << data_buffer.cols() << " by " << ncols << " to "
                << data_buffer.cols() + ncols;
      data_buffer.conservativeResize(Eigen::NoChange, data_buffer.cols() + ncols);
      data_timestamps.conservativeResize(data_timestamps.size() + ncols);

      for (int ii = 0; ii < new_packets.size(); ii++) {
        aco_pkt = new_packets.at(ii);
        ncols = aco_pkt->data.cols();

        if (use_ch_filter) {
          data_buffer.block(0, icol, argPtr->num_channels, ncols) =
              (aco_pkt->data(argPtr->channel_filter, Eigen::all).cast<double>() * audio_scale);
        } else {
          data_buffer.block(0, icol, argPtr->num_channels, ncols) =
              (aco_pkt->data.cast<double>() * audio_scale);
        }

        data_timestamps.segment(icol, ncols) =
            Eigen::Matrix<int64_t, Eigen::Dynamic, 1>::LinSpaced(aco_pkt->data.cols(), 0,
                                                                 1e9 / aco_pkt->header.sample_rate *
                                                                     (aco_pkt->data.cols() - 1))
                .array() +
            aco_pkt->header.start_time_nsec;
        icol += ncols;
      }
    }

    if (data_buffer.cols() >= argPtr->NFFT) {

      offset = 0;
      while ((data_buffer.cols() - offset) >= argPtr->NFFT) {

        fft_frame = std::make_shared<IpcFFT>(argPtr->num_channels, argPtr->nfreq);
        fft_frame->header.start_time_nsec = data_timestamps[offset];

        fft_frame->header.packet_num = packet_num;
        packet_num = packet_num == std::numeric_limits<int>::max() ? 0 : packet_num + 1;

        // Compute FFT:
        // 1 - get NFFT snapshot
        _fft_data_in = data_buffer.block(0, offset, argPtr->num_channels, argPtr->NFFT);

        // 2 - detrend by mean & enforce window to manage ringing
        _fft_data_in = (_fft_data_in.colwise() - _fft_data_in.rowwise().mean()).array() * win;

        pocketfft::r2c(shape_, stride_in_, stride_out_, 1, pocketfft::FORWARD, &_fft_data_in(0, 0),
                       &((*fft_frame).fft(0, 0)), static_cast<double>(1), 0);

        // fft_frame *= scale_coeff; // scale_coeff built into win

        offset += argPtr->nstep;

        for (auto q_fft : argPtr->v_q_fft) {
          q_fft->push(fft_frame);
        }
      }

      // From Eigen: use .eval() to prevent aliasing!
      data_buffer =
          data_buffer.block(0, offset, argPtr->num_channels, data_buffer.cols() - offset).eval();
      data_timestamps = data_timestamps.segment(offset, data_timestamps.size() - offset).eval();

      if (FLAGS_debug_fft)
        VLOG(4) << "Buffer is of size " << data_buffer.cols() << " after dropping " << offset
                << " samples";
    }

    usleep(1000);
  }

  argPtr->_is_running = false;
  pthread_exit(NULL);
}

void FFT::run() {
  pthread_t thread;
  this->keep_alive = true;
  if (!this->is_running()) {
    pthread_create(&thread, NULL, _run_fft_thread, this);
    this->own_thread = thread;
  }
}

void FFT::register_client(QueueClient &client) {
  LOG(INFO) << "Registering " << client.get_name();
  if (client.q_fft != nullptr) {
    auto init_count = v_q_fft.size();
    v_q_fft.push_back(client.q_fft);
    if (FLAGS_debug_fft)
      VLOG(1) << "Q_FFT clients changed from " << init_count << " to " << v_q_fft.size();
  } else {
    LOG(WARNING) << "Cannot register FFT data queue; received nullptr!";
  }
}

void FFT::set_adc_scale(double new_adc_scale) {
  this->adc_scale = new_adc_scale;
  this->_rx_runtime_update = true;
}

void FFT::set_channel_filter(int num_ch) {
  this->use_channel_filter = num_ch > 0;
  this->channel_filter.clear();
  for (int ii = 0; ii < num_ch; ii++) {
    this->channel_filter.push_back(ii);
  }
}
void FFT::set_channel_filter(std::vector<int> channel_filter) {
  this->use_channel_filter = channel_filter.size() > 0;
  this->channel_filter = channel_filter;
}
