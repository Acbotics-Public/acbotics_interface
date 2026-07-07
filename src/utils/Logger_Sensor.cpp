#include "utils/Logger_Sensor.h"
#include <chrono>

  void Logger_Sensor_Block::start_logging(){
    this->logging_active=true;
  }
  void Logger_Sensor_Block::stop_logging()
  {
    this->logging_active=false;
  }

void Logger_Sensor_Block::set_outdir(std::string logger_outdir)
{
  this->logger_outdir = logger_outdir;
}


void Logger_Sensor_Block::run()
{
    pthread_t _thread;
    // pthread_create(&_thread, NULL, _run_logger_thread_gps_csv, this);
    // this->threads.push_back(_thread);

    // pthread_create(&_thread, NULL, _run_logger_thread_pts_csv, this);
    pthread_create(&_thread, NULL, _run_csv_logger_thread<UdpPtsData, LOGGER::PTS>, this);
    this->threads.push_back(_thread);

    pthread_create(&_thread, NULL, _run_csv_logger_thread<UdpEptData, LOGGER::EPT>, this);
    this->threads.push_back(_thread);

    pthread_create(&_thread, NULL, _run_csv_logger_thread<UdpImuData, LOGGER::IMU>, this);
    this->threads.push_back(_thread);

    pthread_create(&_thread, NULL, _run_csv_logger_thread<UdpRtcData, LOGGER::RTC>, this);
    this->threads.push_back(_thread);

    pthread_create(&_thread, NULL, _run_csv_logger_thread<UdpBnoData, LOGGER::BNO>, this);
    this->threads.push_back(_thread);

    pthread_create(&_thread, NULL, _run_csv_logger_thread<UdpBnrData, LOGGER::BNR>, this);
    this->threads.push_back(_thread);

}

template <typename T, LOGGER L> void Logger_Sensor_Block::run_csv_logger_thread() {
  prctl(PR_SET_NAME, ("ac_log_" + LOGGER_NAME[L]).c_str());
  VLOG(3) << "Starting " << LOGGER_NAME[L] << " CSV logger in thread " << pthread_self();

  std::vector<std::shared_ptr<T>> _data_vec;
  std::shared_ptr<T> _data;

  ptr_tsQ<T> queue = std::static_pointer_cast<tsQ_T<T>>(this->queue[LOGGER_QUEUE[L]]);

  bool wrote_header = false;

  while (this->keep_alive) {
    if (this->logging_active) {
      FilenameTime fntime = FilenameTime(this->rollover_min[L]);
      this->output_filename[L] = this->logger_outdir + LOGGER_NAME[L] + "_" + fntime.fname_str + ".csv";

      VLOG(5) << "=========== NEW FILE =========== ";
      LOG(INFO) << "Writing to file : " << this->output_filename[L];
      std::ofstream ofil(this->output_filename[L]);
      wrote_header = false;

      while (this->keep_alive && logging_active && ofil.is_open() &&
             (std::time(nullptr) < fntime.rollover_time)) {
        if (queue->pop_all(_data_vec)) {
          if (!wrote_header) {
            // Check for header bool when queue data is available, to
            // allow data-driven header (e.g. num_ch in acoustic data)
            _data_vec.front()->csv_header(ofil);
            wrote_header = true;
          }

            if (this->logging_active) {

            for (int ii = 0; ii < _data_vec.size(); ii++) {
                _data = _data_vec.at(ii);
                if (_data->header.start_time_nsec >= 0) {
                _data->csv_serialize(ofil);
                }
            }
            ofil.flush();
            }
        }
        // rest here, to allow for external control switch
        //usleep(10000);
	std::this_thread::sleep_for(std::chrono::microseconds(10000));
      }
      LOG(INFO) << "Closing file : " << this->output_filename[L];
      ofil.close();
    } else {
      //usleep(100000);
      std::this_thread::sleep_for(std::chrono::microseconds(100000));
    } 
  }
}


template <typename T, LOGGER L> void *Logger_Sensor_Block::_run_csv_logger_thread(void *ptr) {
  Logger_Sensor_Block *argPtr = static_cast<Logger_Sensor_Block *>(ptr);
  argPtr->run_csv_logger_thread<T, L>();
  pthread_exit(NULL);
}


std::vector<std::string> Logger_Sensor_Block::get_current_paths(void)
{
  std::vector<std::string> vec;
  vec.push_back(this->output_filename[LOGGER::GPS]);
  vec.push_back(this->output_filename[LOGGER::PTS]);
  vec.push_back(this->output_filename[LOGGER::EPT]);
  vec.push_back(this->output_filename[LOGGER::IMU]);
  vec.push_back(this->output_filename[LOGGER::RTC]);
  vec.push_back(this->output_filename[LOGGER::BNO]);
  vec.push_back(this->output_filename[LOGGER::BNR]);
  return vec;
}


Logger_Sensor_Block::Logger_Sensor_Block()
{
    this->rollover_min[LOGGER::GPS] = 60;
    this->rollover_min[LOGGER::PTS] = 60;
    this->rollover_min[LOGGER::EPT] = 60;
    this->rollover_min[LOGGER::IMU] = 60;
    this->rollover_min[LOGGER::RTC] = 60;
    this->rollover_min[LOGGER::BNO] = 60;
    this->rollover_min[LOGGER::BNR] = 60;
    this->logger_outdir = "/TMP/";
}
