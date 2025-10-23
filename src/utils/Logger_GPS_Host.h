#ifndef LOGGER_GPS_HOST_HEADER
#define LOGGER_GPS_HOST_HEADER
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
#include <libgpsmm.h>

class Logger_GPS_Host_Block{
public:
  Logger_GPS_Host_Block();

  void start_logging();
  void stop_logging();
  void run();

  virtual void set_outdir(std::string logger_outdir);
  static void *_run_csv_logger_thread(void *ptr);
  void run_csv_logger_thread();

protected:
    bool logging_active = false;
  std::vector<pthread_t> threads;
  float rollover_min;
  std::string logger_outdir;

};



  #endif