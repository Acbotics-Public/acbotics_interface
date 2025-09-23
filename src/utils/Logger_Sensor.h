#ifndef LOGGER_SENSOR_HEADER
#define LOGGER_SENSOR_HEADER
#include "udp_protocols/UdpPtsData.h"
#include "utils/Types.h"
#include <ctime>
#include <fstream>
#include <glog/logging.h>
#include <memory>
#include <sndfile.h>
#include <sndfile.hh>
#include <time.h>
#include "utils/QueueClient.h"
#include "utils/log_filename_time.h"

class Logger_Sensor_Block : public QueueClient{
public:
  Logger_Sensor_Block();

  void start_logging();
  void stop_logging();
  void run_threads();

  virtual void set_outdir(std::string logger_outdir);
  template <typename T, LOGGER L> static void *_run_csv_logger_thread(void *ptr);
  template <typename T, LOGGER L> void run_csv_logger_thread();

protected:
    bool logging_active = false;
  std::vector<pthread_t> threads;
  std::unordered_map<LOGGER, float> rollover_min;
  std::string logger_outdir;

};



  #endif