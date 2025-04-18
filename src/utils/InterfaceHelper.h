/*******************************************************************/
/*    NAME: Oscar Viquez                                           */
/*    ORG:  Acbotics Research, LLC                                 */
/*    FILE: InterfaceHelper.h                                      */
/*    DATE: Apr 4th 2024                                           */
/*                                                                 */
/*    For help, contact us at: support@acbotics.com                */
/*******************************************************************/

#ifndef gui_helper_HEADER
#define gui_helper_HEADER

#include <Eigen/Dense>
// #include <iostream>
// #include <pthread.h>
#include <fstream>
#include <libgpsmm.h>

// includes from within project
#include "utils/EnergyDetector.h"
#include "utils/FFT.h"
#include "utils/QueueClient.h"
#include "utils/UdpSocketIn.h"

#include "ipc_protocols/IpcBnoState.h"

DECLARE_bool(debug_interface_helper);

class InterfaceHelper : virtual public QueueClient {
public:
  std::unordered_map<LOGGER, bool> logger_running;

  std::shared_ptr<tsQueue<Eigen::MatrixX<int16_t>>> q_aco_out;
  std::shared_ptr<tsQueue<bool>> q_request_aco;

  std::shared_ptr<tsQueue<Eigen::MatrixXd>> q_fft_out;
  std::shared_ptr<tsQueue<bool>> q_request_fft;

  std::shared_ptr<tsQueue<Eigen::MatrixXd>> q_cbf_out;
  std::shared_ptr<tsQueue<bool>> q_request_cbf;

  std::shared_ptr<tsQueue<std::vector<int>>> q_detections_out;
  std::shared_ptr<tsQueue<bool>> q_request_detections;

  int8_t curr_ch;

  std::vector<UdpSocketIn> sockets;
  std::vector<pthread_t> threads;

  int64_t latest_time_nsec = 0;

  FFT _fft_helper;
  EnergyDetector _detector;

  UdpAcousticData::Header latest_aco_header;

  gps_data_t latest_gps_data;

  UdpPtsData latest_pts_data;
  UdpEptData latest_ept_data;
  UdpImuData latest_imu_data;
  UdpRtcData latest_rtc_data;
  UdpBnoData latest_bno_data;
  UdpBnrData latest_bnr_data;

  IpcBnoState latest_bno_state;

  std::unordered_map<LOGGER, std::shared_ptr<UdpData>> latest_data;

  InterfaceHelper() : QueueClient() {
    LOG(INFO) << "Creating InterfaceHelper Instance";
    this->thread_name = "iface_hlpr";

    this->q_aco_dup["fft"] = this->_fft_helper.q_aco;
    this->q_aco_dup["csv"] = std::make_shared<tsQueue<std::shared_ptr<UdpAcousticData>>>();
    this->q_aco_dup["flac"] = std::make_shared<tsQueue<std::shared_ptr<UdpAcousticData>>>();
    this->q_aco_dup["wav"] = std::make_shared<tsQueue<std::shared_ptr<UdpAcousticData>>>();

    this->q_aco_out = std::make_shared<tsQueue<Eigen::MatrixX<int16_t>>>();
    this->q_fft_out = std::make_shared<tsQueue<Eigen::MatrixXd>>();
    this->q_cbf_out = std::make_shared<tsQueue<Eigen::MatrixXd>>();
    this->q_detections_out = std::make_shared<tsQueue<std::vector<int>>>();

    this->q_request_aco = std::make_shared<tsQueue<bool>>();
    this->q_request_fft = std::make_shared<tsQueue<bool>>();
    this->q_request_cbf = std::make_shared<tsQueue<bool>>();
    this->q_request_detections = std::make_shared<tsQueue<bool>>();

    this->history_aco_sec = 0.25;
    this->history_aco = 50000;
    this->history_fft_sec = 0.25;
    this->history_fft = 100;

    this->curr_ch = 0;
    this->adc_scale = 2.5;
    this->_fft_helper.set_adc_scale(this->adc_scale);
    this->phone_sensitivity_V_uPa = -167;

    this->want_all_ch = false;

    this->enable_thread_aco_fork = false;
    this->enable_thread_aco_buffer = false;
    this->enable_thread_fft_buffer = false;
    this->enable_thread_beamformer_buffer = false;
    this->enable_thread_detector = false;

    this->logger_outdir = "/tmp/";

    this->rollover_min[LOGGER::ACO_CSV] = 5;
    this->rollover_min[LOGGER::ACO_FLAC] = 5;
    this->rollover_min[LOGGER::ACO_WAV] = 5;

    this->rollover_min[LOGGER::GPS] = 60;
    this->rollover_min[LOGGER::PTS] = 60;
    this->rollover_min[LOGGER::EPT] = 60;
    this->rollover_min[LOGGER::IMU] = 60;
    this->rollover_min[LOGGER::RTC] = 60;
    this->rollover_min[LOGGER::BNO] = 60;
    this->rollover_min[LOGGER::BNR] = 60;

    this->buffer_has_data_aco = false;
    this->buffer_has_data_fft = false;
    this->buffer_has_data_cbf = false;
  };

  ~InterfaceHelper() { this->stop_threads(); }

  void enable_aco_fork(bool enable);

  void enable_buffer_all_ch(bool enable_all);
  void enable_buffer_aco(bool enable);
  void enable_buffer_fft(bool enable);
  void enable_buffer_beamformer(bool enable);
  void enable_detector(bool enable);

  void add_socket(bool use_mcast, std::string iface_ip, int32_t port, std::string mcast_group);

  int8_t set_curr_ch(int8_t new_ch);
  void set_outdir(std::string logger_outdir);

  void set_rollover(LOGGER logger, float rollover_min);

  void enable_logger(LOGGER, bool);
  void start_logging(LOGGER);
  void stop_logging(LOGGER);

  virtual void run_threads();
  virtual void stop_threads();
  bool check_sockets();

  bool has_data_aco() { return this->buffer_has_data_aco; }
  bool has_data_fft() { return this->buffer_has_data_fft; }
  bool has_data_cbf() { return this->buffer_has_data_cbf; }

  void set_adc_scale(double new_adc_scale);
  void set_phone_sensitivity_V_uPa(double new_phone_sensitivity_V_uPa);

  virtual void set_NFFT(size_t NFFT);
  virtual void set_noverlap(size_t noverlap);
  virtual void set_sample_rate(double sample_rate);

  virtual void set_frequency_boundaries(double fmask_low, double fmask_high) {
    this->_fft_helper.add_frequency_band_min_max(fmask_low, fmask_high);
    this->_detector.add_frequency_band_min_max(fmask_low, fmask_high);
  };

  void set_channel_filter(int num_ch);
  void set_channel_filter(std::vector<int> channel_filter);

  double get_adc_scale() { return this->adc_scale; };
  double get_phone_sensitivity_V_uPa() { return this->phone_sensitivity_V_uPa; };

  template <typename T, LOGGER L> T get_latest_data();

protected:
  struct FilenameTime {
    std::time_t fname_time;
    std::time_t rollover_time;
    std::string fname_str;

    FilenameTime() : FilenameTime(-1) {}
    FilenameTime(int rollover_min);

  protected:
    int rollover_min_default = 5;
  };

  std::unordered_map<std::string, std::shared_ptr<tsQueue<std::shared_ptr<UdpAcousticData>>>>
      q_aco_dup;

  double adc_scale;
  double phone_sensitivity_V_uPa;
  double history_aco_sec;
  size_t history_aco;
  double history_fft_sec;
  size_t history_fft;

  std::string logger_outdir;

  bool want_all_ch;

  // Controls for thread spawning
  // Set desired value before starting threads
  bool enable_thread_aco_fork;
  bool enable_thread_aco_buffer;
  bool enable_thread_fft_buffer;
  bool enable_thread_beamformer_buffer;
  bool enable_thread_detector;

  std::unordered_map<LOGGER, bool> logger_enabled;

  // Controls for runtime logic
  // Threads may sleep / stand by (i.e. for logging from GUI)
  std::unordered_map<LOGGER, bool> logger_active;

  std::unordered_map<LOGGER, float> rollover_min;

  bool buffer_has_data_aco;
  bool buffer_has_data_fft;
  bool buffer_has_data_cbf;

  static void *_run_thread_aco_helper(void *arg); // aco helper : fork + buffer
  static void *_run_thread_fft_buffer(void *arg);
  static void *_run_thread_beamformer_buffer(void *arg);
  static void *_run_thread_detections_buffer(void *arg);

  static void *_run_logger_thread_audio_csv(void *arg);
  static void *_run_logger_thread_audio_flac(void *arg);
  static void *_run_logger_thread_audio_wav(void *arg);

  static void *_run_logger_thread_gps_csv(void *arg);

  static void *_run_thread_bno_state_buffer(void *arg);

  template <typename T, LOGGER L> static void *_run_csv_logger_thread(void *ptr);
};

template <typename T, LOGGER L> void *InterfaceHelper::_run_csv_logger_thread(void *ptr) {

  InterfaceHelper *argPtr = static_cast<InterfaceHelper *>(ptr);
  prctl(PR_SET_NAME, ("ac_log_" + LOGGER_NAME[L]).c_str());
  VLOG(3) << "Starting " << LOGGER_NAME[L] << " CSV logger in thread " << pthread_self();

  bool &logging_active = argPtr->logger_active[L];

  argPtr->logger_running[L] = true;

  std::string output_filename;

  std::vector<std::shared_ptr<T>> _data_vec;
  std::shared_ptr<T> _data;

  ptr_tsQ<T> queue = std::static_pointer_cast<tsQ_T<T>>(argPtr->queue[LOGGER_QUEUE[L]]);

  bool wrote_header = false;

  while (argPtr->keep_alive) {
    if (logging_active) {
      FilenameTime fntime = FilenameTime(argPtr->rollover_min[L]);
      output_filename = argPtr->logger_outdir + LOGGER_NAME[L] + "_" + fntime.fname_str + ".csv";

      VLOG(5) << "=========== NEW FILE =========== ";
      LOG(INFO) << "Writing to file : " << output_filename;
      std::ofstream ofil(output_filename);
      wrote_header = false;

      while (argPtr->keep_alive && logging_active && ofil.is_open() &&
             (std::time(nullptr) < fntime.rollover_time)) {
        if (queue->pop_all(_data_vec)) {
          if (!wrote_header) {
            // Check for header bool when queue data is available, to
            // allow data-driven header (e.g. num_ch in acoustic data)
            _data_vec.front()->csv_header(ofil);
            wrote_header = true;
          }

          argPtr->latest_data[L] = _data_vec.back();
          for (int ii = 0; ii < _data_vec.size(); ii++) {
            _data = _data_vec.at(ii);
            if (_data->header.start_time_nsec >= 0) {
              _data->csv_serialize(ofil);
            }
          }
          ofil.flush();
        }
        // rest here, to allow for external control switch
        usleep(10000);
      }
      LOG(INFO) << "Closing file : " << output_filename;
      ofil.close();
    } else {
      if (queue->size() > 0)
        argPtr->latest_data[L] = queue->pop_all().back();
      usleep(100000);
    }
  }

  argPtr->logger_running[LOGGER::PTS] = false;
  pthread_exit(NULL);
}

template <typename T, LOGGER L> T InterfaceHelper::get_latest_data() {
  T _data;
  ptr_tsQ<T> queue = std::static_pointer_cast<tsQ_T<T>>(this->queue[LOGGER_QUEUE[L]]);

  auto ptr = this->latest_data[L].get();

  if (this->logger_running[L] && ptr != nullptr) {
    _data = *static_cast<T *>(ptr);
  } else if (queue->size() > 0) {
    _data = *(queue->pop_all().back());
  }

  return _data;
}

#endif
