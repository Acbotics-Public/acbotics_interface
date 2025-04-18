/*******************************************************************/
/*    NAME: Oscar Viquez                                           */
/*    ORG:  Acbotics Research, LLC                                 */
/*    FILE: InterfaceHelper.cpp                                    */
/*    DATE: Apr 4th 2024                                           */
/*                                                                 */
/*    For help, contact us at: support@acbotics.com                */
/*******************************************************************/

#include <glog/logging.h>
#include <pthread.h>

// includes from within project
#include "utils/InterfaceHelper.h"

DEFINE_bool(debug_interface_helper, false, "Enable expanded debug for Interface Helper");

void InterfaceHelper::run_threads() {

  pthread_t _thread;
  bool _enable_fft_helper = false;

  if (this->enable_thread_aco_fork) {
    pthread_create(&_thread, NULL, _run_thread_aco_helper, this);
    this->threads.push_back(_thread);
  }
  if (this->enable_thread_fft_buffer) {
    pthread_create(&_thread, NULL, _run_thread_fft_buffer, this);
    this->threads.push_back(_thread);
    this->_fft_helper.register_client(*this);
    _enable_fft_helper = true;
  }
  if (this->enable_thread_beamformer_buffer) {
    pthread_create(&_thread, NULL, _run_thread_beamformer_buffer, this);
    this->threads.push_back(_thread);
    // _enable_cbf_helper = true;
  }
  if (this->enable_thread_detector) {
    this->_detector.run();
    pthread_create(&_thread, NULL, _run_thread_detections_buffer, this);
    this->threads.push_back(_thread);
    this->_fft_helper.register_client(this->_detector);
    this->_detector.register_client(*this);
    _enable_fft_helper = true;
  }

  if (_enable_fft_helper) {
    this->_fft_helper.run();
  }

  // if (this->enable_bno_state_buffer) {
  pthread_create(&_thread, NULL, _run_thread_bno_state_buffer, this);
  this->threads.push_back(_thread);
  // }

  if (this->logger_enabled[LOGGER::ACO_CSV]) {
    pthread_create(&_thread, NULL, _run_logger_thread_audio_csv, this);
    this->threads.push_back(_thread);
  }
  if (this->logger_enabled[LOGGER::ACO_FLAC]) {
    pthread_create(&_thread, NULL, _run_logger_thread_audio_flac, this);
    this->threads.push_back(_thread);
  }
  if (this->logger_enabled[LOGGER::ACO_WAV]) {
    pthread_create(&_thread, NULL, _run_logger_thread_audio_wav, this);
    this->threads.push_back(_thread);
  }
  if (this->logger_enabled[LOGGER::GPS]) {
    pthread_create(&_thread, NULL, _run_logger_thread_gps_csv, this);
    this->threads.push_back(_thread);
  }
  if (this->logger_enabled[LOGGER::PTS]) {
    // pthread_create(&_thread, NULL, _run_logger_thread_pts_csv, this);
    pthread_create(&_thread, NULL, _run_csv_logger_thread<UdpPtsData, LOGGER::PTS>, this);
    this->threads.push_back(_thread);
  }
  if (this->logger_enabled[LOGGER::EPT]) {
    pthread_create(&_thread, NULL, _run_csv_logger_thread<UdpEptData, LOGGER::EPT>, this);
    this->threads.push_back(_thread);
  }
  if (this->logger_enabled[LOGGER::IMU]) {
    pthread_create(&_thread, NULL, _run_csv_logger_thread<UdpImuData, LOGGER::IMU>, this);
    this->threads.push_back(_thread);
  }
  if (this->logger_enabled[LOGGER::RTC]) {
    pthread_create(&_thread, NULL, _run_csv_logger_thread<UdpRtcData, LOGGER::RTC>, this);
    this->threads.push_back(_thread);
  }
  if (this->logger_enabled[LOGGER::BNO]) {
    pthread_create(&_thread, NULL, _run_csv_logger_thread<UdpBnoData, LOGGER::BNO>, this);
    this->threads.push_back(_thread);
  }
  if (this->logger_enabled[LOGGER::BNR]) {
    pthread_create(&_thread, NULL, _run_csv_logger_thread<UdpBnrData, LOGGER::BNR>, this);
    this->threads.push_back(_thread);
  }

  for (int ii = 0; ii < this->sockets.size(); ii++) {
    // using auto here causes issues with pthread; use explicit indexing
    // this->threads.push_back(this->sockets[ii].run_socket_thread());
    this->sockets[ii].run_socket_thread();
  }
}

void InterfaceHelper::stop_threads() {
  this->keep_alive = false;

  if (this->threads.size() > 0) {
    LOG(INFO) << "Closing InterfaceHelper threads";
    for (int ii = 0; ii < this->threads.size(); ii++) {
      LOG(INFO) << "Waiting for thread " << this->threads.at(ii) << " (" << ii + 1 << " of "
                << this->threads.size() << ")";
      pthread_join(this->threads.at(ii), NULL);
    }
    this->threads.clear();
  }

  if (this->_fft_helper.is_running()) {
    LOG(INFO) << "Closing FFT computation thread";
    this->_fft_helper.stop();
  }

  if (this->sockets.size() > 0) {
    LOG(INFO) << "Closing sockets";
    for (int ii = 0; ii < this->sockets.size(); ii++) {
      this->sockets.at(ii).stop();
    }
  }
}

void InterfaceHelper::add_socket(bool use_mcast, std::string iface_ip, int32_t port,
                                 std::string mcast_group) {
  UdpSocketIn new_sock(use_mcast, iface_ip, port, mcast_group);
  new_sock.register_client(*this);
  this->sockets.push_back(new_sock);
  VLOG(5) << "Adding port:" << std::endl << new_sock;
};

int8_t InterfaceHelper::set_curr_ch(int8_t ch) {
  if (ch < this->latest_aco_header.num_channels && !this->_fft_helper.use_channel_filter) {
    this->curr_ch = ch;
    VLOG(3) << "Updating current channel to " << (int)ch;
  } else if (ch < this->latest_aco_header.num_channels &&
             std::find(this->_fft_helper.channel_filter.begin(),
                       this->_fft_helper.channel_filter.end(),
                       ch) != this->_fft_helper.channel_filter.end()) {
    this->curr_ch = ch;
    VLOG(3) << "Updating current channel to " << (int)ch;
  } else {
    LOG(WARNING) << "Received request for ch index " << (int)ch
                 << " but ch is not available or is filtered by config; leaving as "
                 << this->curr_ch;
  }
  return this->curr_ch;
}

bool InterfaceHelper::check_sockets() {
  bool all_connected = true;
  for (int ii = 0; ii < this->sockets.size(); ii++) {
    all_connected = all_connected && this->sockets.at(ii).is_connected();
  }
  return all_connected;
}

void InterfaceHelper::enable_aco_fork(bool enable) { this->enable_thread_aco_fork = enable; }

// Enable bufferers
void InterfaceHelper::enable_buffer_all_ch(bool enable_all) { this->want_all_ch = enable_all; }
void InterfaceHelper::enable_buffer_aco(bool enable) {
  if (enable && !this->enable_thread_aco_fork) {
    VLOG(5) << "Acoustic data helper thread required for acoustic data loggers; enabling";
    this->enable_thread_aco_fork = true;
  }
  this->enable_thread_aco_buffer = enable;
}
void InterfaceHelper::enable_buffer_fft(bool enable) { this->enable_thread_fft_buffer = enable; }
void InterfaceHelper::enable_buffer_beamformer(bool enable) {
  this->enable_thread_beamformer_buffer = enable;
}

void InterfaceHelper::enable_detector(bool enable) { this->enable_thread_detector = enable; }

// Enable loggers
void InterfaceHelper::enable_logger(LOGGER logger, bool enable) {

  // this->logger_active[logger] = true;
  // Assume auto-start if enable==true
  if ((logger == LOGGER::ACO_CSV || logger == LOGGER::ACO_FLAC || logger == LOGGER::ACO_WAV) &&
      enable && !this->enable_thread_aco_fork) {
    VLOG(5) << "Acoustic data helper thread required for acoustic data loggers; enabling";
    this->enable_thread_aco_fork = true;
  }
  this->logger_enabled[logger] = enable;
  this->logger_active[logger] = enable;
  if (enable)
    LOG(INFO) << LOGGER_NAME[logger] + " logging is now enabled.";
}

// Start loggers
void InterfaceHelper::start_logging(LOGGER logger) { this->logger_active[logger] = true; }

// Stop loggers
void InterfaceHelper::stop_logging(LOGGER logger) { this->logger_active[logger] = false; }

void InterfaceHelper::set_rollover(LOGGER logger, float rollover_min) {
  this->rollover_min[logger] = rollover_min;
  LOG(INFO) << LOGGER_NAME[logger] << " log rollover time is " << rollover_min << " minutes";
}

void InterfaceHelper::set_outdir(std::string logger_outdir) {
  this->logger_outdir = logger_outdir;
};

void InterfaceHelper::set_adc_scale(double new_adc_scale) {
  this->_fft_helper.set_adc_scale(new_adc_scale);
  this->adc_scale = new_adc_scale;
}

void InterfaceHelper::set_phone_sensitivity_V_uPa(double new_phone_sensitivity_V_uPa) {
  this->phone_sensitivity_V_uPa = new_phone_sensitivity_V_uPa;
}

InterfaceHelper::FilenameTime::FilenameTime(int rollover_min) {
  // Identify timestamp for file name

  std::time_t ts_now;
  std::tm tm_now;
  std::tm tm_fname{};
  char fname_chr[std::size("YYYYMMDD-hhmmss")];
  std::time_t tzoffset;

  if (rollover_min == -1) {
    VLOG(5) << "Using default rollover time: " << this->rollover_min_default << " min";
    rollover_min = this->rollover_min_default;
  }
  if (rollover_min == 0) {
    LOG(WARNING) << "Rollover time (min) must be greater than zero; using default: 5 min";
    rollover_min = this->rollover_min_default;
  }

  ts_now = std::time(nullptr);
  tm_now = *std::gmtime(&ts_now);

  // <ctime> gives local time via std::time(nullptr) ;
  // get adjustment for time zone correction:
  tzoffset = ts_now - std::mktime(&tm_now);

  // Use rollover_min windows for logfile chunking
  tm_fname = tm_now;
  tm_fname.tm_min = tm_now.tm_min - (tm_now.tm_min % rollover_min);
  tm_fname.tm_sec = 0;

  this->rollover_time = (std::mktime(&tm_fname) + (int)(rollover_min * 60) + tzoffset);

  std::strftime(fname_chr, sizeof(fname_chr), "%Y%m%d-%H%M%S", &tm_now);
  this->fname_time = std::mktime(&tm_now);
  this->fname_str = std::string(fname_chr);
}

void InterfaceHelper::set_NFFT(size_t NFFT) {
  this->_fft_helper.set_NFFT(NFFT);
  this->_detector.set_NFFT(NFFT);
}

void InterfaceHelper::set_noverlap(size_t noverlap) {
  this->_fft_helper.set_noverlap(noverlap);
  this->_detector.set_noverlap(noverlap);
}

void InterfaceHelper::set_sample_rate(double sample_rate) {
  this->_fft_helper.set_sample_rate(sample_rate);
  this->_detector.set_sample_rate(sample_rate);
}

void InterfaceHelper::set_channel_filter(int num_ch) {
  this->_fft_helper.set_channel_filter(num_ch);
}

void InterfaceHelper::set_channel_filter(std::vector<int> channel_filter) {
  this->_fft_helper.set_channel_filter(channel_filter);
}
