#ifndef LOG_FILENAME_TIME_HEADER
#define LOG_FILENAME_TIME_HEADER

#include <string>
#include <ctime>
#include <glog/logging.h>
  struct FilenameTime {
    std::time_t fname_time;
    std::time_t rollover_time;
    std::string fname_str;

    FilenameTime() : FilenameTime(-1) {}
    FilenameTime(int rollover_min);
    void reset(int rollover_min);

  protected:
    int rollover_min_default = 5;

  };

  #endif

