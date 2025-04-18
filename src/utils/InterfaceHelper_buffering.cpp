/*******************************************************************/
/*    NAME: Oscar Viquez                                           */
/*    ORG:  Acbotics Research, LLC                                 */
/*    FILE: InterfaceHelper_buffering.cpp                          */
/*    DATE: Apr 4th 2024                                           */
/*                                                                 */
/*    For help, contact us at: support@acbotics.com                */
/*******************************************************************/

#include <glog/logging.h>
#include <pthread.h>

// includes from within project
#include "utils/InterfaceHelper.h"

void *InterfaceHelper::_run_thread_aco_helper(void *ptr) {

  InterfaceHelper *argPtr = static_cast<InterfaceHelper *>(ptr);
  prctl(PR_SET_NAME, "aco_helper_thr");
  VLOG(3) << "Starting acoustic data fork/buffer in thread " << pthread_self();

  bool &log_audio_csv = argPtr->logger_active[LOGGER::ACO_CSV];
  bool &log_audio_flac = argPtr->logger_active[LOGGER::ACO_FLAC];
  bool &log_audio_wav = argPtr->logger_active[LOGGER::ACO_WAV];

  size_t icol;
  size_t ncols;

  std::shared_ptr<UdpAcousticData> aco_pkt;
  std::vector<std::shared_ptr<UdpAcousticData>> new_packets;

  Eigen::MatrixX<int16_t> data_buffer(0, 0);

  bool initialized = false;

  while (argPtr->keep_alive && !initialized) {
    if (argPtr->q_aco->size() > 0) {
      aco_pkt = argPtr->q_aco->pop();
      initialized = true;
    } else {
      usleep(10000);
    }
  }

  VLOG(2) << "Setting sample rate from UDP data: " << aco_pkt->header.sample_rate;
  argPtr->set_sample_rate((double)aco_pkt->header.sample_rate);

  if (argPtr->want_all_ch) {
    data_buffer = aco_pkt->data;
  } else {
    data_buffer = aco_pkt->data.row(argPtr->curr_ch);
  }

  argPtr->latest_aco_header = aco_pkt->header;

  argPtr->history_aco = aco_pkt->header.sample_rate * argPtr->history_aco_sec;
  int nstep = argPtr->_fft_helper.get_nstep();
  argPtr->history_fft = aco_pkt->header.sample_rate / nstep * argPtr->history_fft_sec;

  VLOG(2) << "Buffer lengths: ACO = " << argPtr->history_aco
          << " samples, FFT = " << argPtr->history_fft << " rasters";

  while (argPtr->keep_alive) {

    if (argPtr->q_aco->pop_all(new_packets)) {

      argPtr->latest_aco_header = new_packets.back()->header;

      if (argPtr->_fft_helper.is_running()) {
        argPtr->q_aco_dup["fft"]->push_all(new_packets);
      }

      if (log_audio_csv) {
        argPtr->q_aco_dup["csv"]->push_all(new_packets);
      }

      if (log_audio_flac) {
        argPtr->q_aco_dup["flac"]->push_all(new_packets);
      }
      if (log_audio_wav) {
        argPtr->q_aco_dup["wav"]->push_all(new_packets);
      }

      if (argPtr->enable_thread_aco_buffer) {
        ncols = 0;
        for (int ii = 0; ii < new_packets.size(); ii++) {
          ncols += new_packets[ii]->data.cols();
        }

        icol = data_buffer.cols();
        data_buffer.conservativeResize(Eigen::NoChange, data_buffer.cols() + ncols);
        for (int ii = 0; ii < new_packets.size(); ii++) {
          aco_pkt = new_packets[ii];
          ncols = aco_pkt->data.cols();

          if (argPtr->want_all_ch) {
            data_buffer.block(0, icol, aco_pkt->data.rows(), ncols) = aco_pkt->data;
          } else {
            data_buffer.block(0, icol, 1, ncols) = aco_pkt->data.row(argPtr->curr_ch);
          }
          icol += ncols;
        }
        if (data_buffer.cols() > argPtr->history_aco * 2.0) { // using 2x as margin for hysteresis
          if (FLAGS_debug_interface_helper)
            VLOG(5) << "Long ACO buffer; trimming to " << argPtr->history_aco << " samples";
          data_buffer = data_buffer
                            .block(0, data_buffer.cols() - argPtr->history_aco, data_buffer.rows(),
                                   argPtr->history_aco)
                            .eval();
        }
        argPtr->buffer_has_data_aco = true;
      }
    }
    if (argPtr->q_request_aco->size() > 0) {
      if (FLAGS_debug_interface_helper)
        VLOG(5) << "Received request for aco data; buffer size " << data_buffer.rows() << " x "
                << data_buffer.cols();
      argPtr->q_request_aco->pop();
      argPtr->q_aco_out->push(data_buffer);
      argPtr->buffer_has_data_aco = false;
      data_buffer.resize(Eigen::NoChange, 0);
    }

    usleep(1000);
  }
  pthread_exit(NULL);
}

void *InterfaceHelper::_run_thread_fft_buffer(void *ptr) {

  InterfaceHelper *argPtr = static_cast<InterfaceHelper *>(ptr);
  prctl(PR_SET_NAME, "fft_buffer_thr");
  VLOG(3) << "Starting fft data buffer in thread " << pthread_self();

  size_t irow;

  Eigen::MatrixXd fft_buffer(0, 0);
  Eigen::MatrixXd fft_pkt(0, 0);

  std::vector<std::shared_ptr<IpcFFT>> new_packets;

  bool initialized = false;

  while (argPtr->keep_alive && !initialized) {
    if (argPtr->q_fft->size() > 0) {
      fft_buffer = argPtr->q_fft->pop()->fft.row(argPtr->curr_ch).array().abs().log10() * 20 -
                   argPtr->phone_sensitivity_V_uPa;
      initialized = true;

    } else {
      usleep(10000);
    }
  }

  while (argPtr->keep_alive) {

    if (argPtr->q_fft->pop_all(new_packets)) {

      irow = fft_buffer.rows();
      fft_buffer.conservativeResize(fft_buffer.rows() + new_packets.size(), Eigen::NoChange);
      for (int ii = 0; ii < new_packets.size(); ii++) {
        fft_pkt = new_packets[ii]->fft.row(argPtr->curr_ch).array().abs().log10() * 20 -
                  argPtr->phone_sensitivity_V_uPa;
        fft_buffer.block(irow + ii, 0, fft_pkt.rows(), fft_pkt.cols()) = fft_pkt;
      }
      if (fft_buffer.rows() > argPtr->history_fft * 2) { // using 2x as margin for hysteresis
        if (FLAGS_debug_interface_helper)
          VLOG(5) << "Long FFT buffer; trimming to " << argPtr->history_fft << " lines";
        fft_buffer = fft_buffer.block(fft_buffer.rows() - argPtr->history_fft, 0,
                                      argPtr->history_fft, fft_buffer.cols());
      }
      argPtr->buffer_has_data_fft = true;
    }

    if (argPtr->q_request_fft->size() > 0) {
      if (FLAGS_debug_interface_helper)
        VLOG(5) << "Received request for fft data; buffer size " << fft_buffer.rows() << " x "
                << fft_buffer.cols();
      argPtr->q_request_fft->pop();
      argPtr->q_fft_out->push(fft_buffer);

      argPtr->buffer_has_data_fft = false;

      fft_buffer = argPtr->q_fft->pop()->fft.row(argPtr->curr_ch).array().abs().log10() * 20 -
                   argPtr->phone_sensitivity_V_uPa;
    }

    usleep(1000);
  }
  pthread_exit(NULL);
}

void *InterfaceHelper::_run_thread_beamformer_buffer(void *ptr) {

  InterfaceHelper *argPtr = static_cast<InterfaceHelper *>(ptr);
  prctl(PR_SET_NAME, "cbf_buffer_thr");
  VLOG(3) << "Starting beamformer data buffer in thread " << pthread_self();

  size_t icol;
  size_t ncols;

  std::shared_ptr<UdpBeamform2D> cbf_pkt;
  std::vector<std::shared_ptr<UdpBeamform2D>> new_packets;

  Eigen::MatrixXd data_buffer(0, 0);

  bool initialized = false;

  while (argPtr->keep_alive && !initialized) {
    if (argPtr->q_beam2d->pop(cbf_pkt)) {
      initialized = true;
    } else {
      usleep(10000);
    }
  }

  data_buffer = cbf_pkt->get_1D();

  while (argPtr->keep_alive) {

    if (argPtr->q_beam2d->pop_all(new_packets)) {

      ncols = new_packets.size();

      icol = data_buffer.cols();
      data_buffer.conservativeResize(Eigen::NoChange, data_buffer.cols() + ncols);
      for (int ii = 0; ii < new_packets.size(); ii++) {
        cbf_pkt = new_packets[ii];
        data_buffer.block(0, icol, cbf_pkt->data.bearings_rad.size(), 1) = cbf_pkt->get_1D();
        icol += 1;
      }
      if (data_buffer.cols() > argPtr->history_aco * 2.0) { // using 2x as margin for hysteresis
        if (FLAGS_debug_interface_helper)
          VLOG(5) << "Long CBF buffer; trimming to " << argPtr->history_aco << " samples";
        Eigen::MatrixXd buff;
        buff = data_buffer.block(0, data_buffer.cols() - argPtr->history_aco, data_buffer.rows(),
                                 argPtr->history_aco);
        data_buffer = buff;
      }
      argPtr->buffer_has_data_cbf = true;
    }
    if (argPtr->q_request_cbf->size() > 0) {
      if (FLAGS_debug_interface_helper)
        VLOG(5) << "Received request for cbf data; buffer size " << data_buffer.rows() << " x "
                << data_buffer.cols();
      argPtr->q_request_cbf->pop();
      argPtr->q_cbf_out->push(data_buffer);

      argPtr->buffer_has_data_cbf = false;

      cbf_pkt = argPtr->q_beam2d->pop();
      data_buffer = cbf_pkt->get_1D();
    }

    usleep(1000);
  }
  pthread_exit(NULL);
}

void *InterfaceHelper::_run_thread_detections_buffer(void *ptr) {

  InterfaceHelper *argPtr = static_cast<InterfaceHelper *>(ptr);
  prctl(PR_SET_NAME, "detect_buf_thr");
  VLOG(3) << "Starting energy detector data buffer in thread " << pthread_self();

  std::vector<std::shared_ptr<IpcDetector>> new_packets;
  std::vector<int> detections;

  while (argPtr->keep_alive) {

    if (argPtr->q_detect->pop_all(new_packets)) {

      for (int ii = 0; ii < new_packets.size(); ii++) {
        detections.push_back(new_packets[ii]->detections);
      }
    }

    if (argPtr->q_request_detections->size() > 0) {
      if (FLAGS_debug_interface_helper)
        VLOG(5) << "Received request for detection data; buffer size " << detections.size();
      argPtr->q_request_detections->pop();
      argPtr->q_detections_out->push(detections);
      detections.clear();
    }

    usleep(1000);
  }
  pthread_exit(NULL);
}

void *InterfaceHelper::_run_thread_bno_state_buffer(void *ptr) {

  InterfaceHelper *argPtr = static_cast<InterfaceHelper *>(ptr);
  prctl(PR_SET_NAME, "bno_buf_thr");
  VLOG(3) << "Starting BNO state data buffer in thread " << pthread_self();

  std::vector<std::shared_ptr<UdpBnoData>> new_bno_packets;
  std::vector<std::shared_ptr<UdpBnrData>> new_bnr_packets;

  UdpBnoData new_bno_frame;
  UdpBnrData new_bnr_frame;

  while (argPtr->keep_alive) {

    if (argPtr->q_bno->pop_all(new_bno_packets)) {
      // BNO queue will have single-type packets (ACCEL / GYRO / MAG)
      // So we need to interate through the queue to properly capture the
      // latest known state

      for (int ii = 0; ii < new_bno_packets.size(); ii++) {
        new_bno_frame = *(new_bno_packets.at(ii));
        switch (new_bno_frame.sense_type) {
        case BNO_TYPE::ACCEL:
          argPtr->latest_bno_state.accel_x = new_bno_frame.sense_x;
          argPtr->latest_bno_state.accel_y = new_bno_frame.sense_y;
          argPtr->latest_bno_state.accel_z = new_bno_frame.sense_z;
          break;

        case BNO_TYPE::GYRO:
          argPtr->latest_bno_state.gyro_x = new_bno_frame.sense_x;
          argPtr->latest_bno_state.gyro_y = new_bno_frame.sense_y;
          argPtr->latest_bno_state.gyro_z = new_bno_frame.sense_z;
          break;

        case BNO_TYPE::MAG:
          argPtr->latest_bno_state.mag_x = new_bno_frame.sense_x;
          argPtr->latest_bno_state.mag_y = new_bno_frame.sense_y;
          argPtr->latest_bno_state.mag_z = new_bno_frame.sense_z;
          break;

        default:
          break;
        }
      }
      argPtr->latest_bno_state.header.start_time_nsec =
          new_bno_frame.header.start_time_nsec > argPtr->latest_bno_state.header.start_time_nsec
              ? new_bno_frame.header.start_time_nsec
              : argPtr->latest_bno_state.header.start_time_nsec;
    }

    if (argPtr->q_bnr->size() > 0) {
      // We just want the latest record here
      new_bnr_frame = *(argPtr->q_bnr->pop_all().back());

      argPtr->latest_bno_state.quat_i = new_bnr_frame.quat_i;
      argPtr->latest_bno_state.quat_j = new_bnr_frame.quat_j;
      argPtr->latest_bno_state.quat_k = new_bnr_frame.quat_k;
      argPtr->latest_bno_state.quat_r = new_bnr_frame.quat_r;

      argPtr->latest_bno_state.header.start_time_nsec =
          new_bnr_frame.header.start_time_nsec > argPtr->latest_bno_state.header.start_time_nsec
              ? new_bnr_frame.header.start_time_nsec
              : argPtr->latest_bno_state.header.start_time_nsec;
    }
    usleep(1000);
  }
  pthread_exit(NULL);
}
