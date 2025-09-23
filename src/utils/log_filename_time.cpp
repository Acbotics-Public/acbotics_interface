#include "utils/log_filename_time.h"


  FilenameTime::FilenameTime(int rollover_min) {
  // Identify timestamp for file name

  this->reset(rollover_min);
}
void FilenameTime::reset(int rollover_min) {
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