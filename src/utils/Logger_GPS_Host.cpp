#include "utils/Logger_GPS_Host.h"
#include <iomanip>
#include <chrono>

  void Logger_GPS_Host_Block::start_logging(){
    this->logging_active=true;
  }
  void Logger_GPS_Host_Block::stop_logging()
  {
    this->logging_active=false;
  }

void Logger_GPS_Host_Block::set_outdir(std::string logger_outdir)
{
  this->logger_outdir = logger_outdir;
}


void Logger_GPS_Host_Block::run()
{
    pthread_t _thread;
    // pthread_create(&_thread, NULL, _run_logger_thread_gps_csv, this);
    // this->threads.push_back(_thread);

    // pthread_create(&_thread, NULL, _run_logger_thread_pts_csv, this);
    pthread_create(&_thread, NULL, _run_csv_logger_thread, this);
    this->threads.push_back(_thread);
}

void Logger_GPS_Host_Block::run_csv_logger_thread() {
  prctl(PR_SET_NAME, "ac_log_gps");
  VLOG(3) << "Starting GPS CSV logger in thread " << pthread_self();

  bool &logging_active = this->logging_active;

//   this->logger_running = true;

  std::ostringstream csv_header;

  csv_header << "host_epoch_sec,gps_epoch_sec,lat,lon,sats,mode,status,altHAE,epx,epy,epv,epd,track,speed,eps,eph,climb,epc";

  //   struct gps_data_t latest_gps_data;

  while (true) {
    if (logging_active) {

      gpsmm gps_rec("localhost", DEFAULT_GPSD_PORT);

      if (gps_rec.stream(WATCH_ENABLE | WATCH_JSON) == nullptr) {
        // std::cerr << "No GPSD running.\n";
        // return 1;
        VLOG(3) << "Could not connect to GPSD; service might not be running.";
        //sleep(1);
	std::this_thread::sleep_for(std::chrono::seconds(1));
        continue;
      }

      FilenameTime fntime = FilenameTime(this->rollover_min);
      output_filename = this->logger_outdir + "GPS_" + fntime.fname_str + ".csv";

      VLOG(5) << "=========== NEW FILE =========== ";
      LOG(INFO) << "Writing to file : " << output_filename;
      std::ofstream ofil(output_filename);
      ofil << csv_header.str() << std::endl;
      int default_precision = ofil.precision();

      while ( logging_active && ofil.is_open() &&
             (std::time(nullptr) < fntime.rollover_time)) {

        struct gps_data_t *new_gps_data;

        if (!gps_rec.waiting(5000000)) {
          VLOG(5) << "Timeout waiting for GPSD data.\n";
          continue;
        }

        if ((new_gps_data = gps_rec.read()) == nullptr) {
          VLOG(5) << "Error reading GPSD data.\n";
          continue;
        }

        // if (((new_gps_data->fix.latitude == this->latest_gps_data.fix.latitude) ||
        //      (std::isnan(new_gps_data->fix.latitude) &&
        //       std::isnan(this->latest_gps_data.fix.latitude))) &&
        //     ((new_gps_data->fix.longitude == this->latest_gps_data.fix.longitude) ||
        //      (std::isnan(new_gps_data->fix.longitude) &&
        //       std::isnan(this->latest_gps_data.fix.longitude))) &&
        //     (new_gps_data->fix.time.tv_sec == this->latest_gps_data.fix.time.tv_sec) &&
        //     (new_gps_data->fix.time.tv_nsec == this->latest_gps_data.fix.time.tv_nsec)) {
        //   //   std::cout << "old ref " << std::endl;
        //   continue;
        // }

        if (new_gps_data->fix.mode >= MODE_2D) {

          //  std::string(buff) + "." + nsec;

          ofil << std::time(nullptr) << "," << std::to_string(new_gps_data->fix.time.tv_sec);
          if (new_gps_data->fix.time.tv_nsec > 0) {
            std::string nsec = std::to_string(new_gps_data->fix.time.tv_nsec);
            size_t len_nsec_padding = 9 - nsec.length();
            nsec.insert(0, len_nsec_padding, '0');
            ofil << "." << nsec;
          }
          ofil << ",";
          ofil << std::fixed << std::setprecision(8);
          ofil << new_gps_data->fix.latitude << "," << new_gps_data->fix.longitude;

          ofil << "," << new_gps_data->satellites_used << "," << new_gps_data->fix.mode;
          ofil << "," << new_gps_data->fix.status << "," << new_gps_data->fix.altHAE;
          ofil << "," << new_gps_data->fix.epx << "," << new_gps_data->fix.epy;
          ofil << "," << new_gps_data->fix.epv << "," << new_gps_data->fix.epd;
          ofil << "," << new_gps_data->fix.track << "," << new_gps_data->fix.speed;
          ofil << "," << new_gps_data->fix.eps << "," << new_gps_data->fix.eph;
          ofil << "," << new_gps_data->fix.climb << "," << new_gps_data->fix.epc;


          ofil << std::setprecision(default_precision) << std::defaultfloat;
          ofil << std::endl;
        }

        // this->latest_gps_data = *new_gps_data;

        ofil.flush();
      }
      LOG(INFO) << "Closing file : " << output_filename;
      ofil.close();
    } else {
      //usleep(100000);
      std::this_thread::sleep_for(std::chrono::microseconds(100000));
    }
  }

  pthread_exit(NULL);
}
std::vector<std::string> Logger_GPS_Host_Block::get_current_paths(void)
{
  std::vector<std::string> vec;
  vec.push_back(this->output_filename);
  return vec;
}


void *Logger_GPS_Host_Block::_run_csv_logger_thread(void *ptr) {
  Logger_GPS_Host_Block *argPtr = static_cast<Logger_GPS_Host_Block *>(ptr);
  argPtr->run_csv_logger_thread();
  pthread_exit(NULL);
}


Logger_GPS_Host_Block::Logger_GPS_Host_Block()
{
    this->rollover_min = 60;
    this->logger_outdir = "/TMP/";
}
