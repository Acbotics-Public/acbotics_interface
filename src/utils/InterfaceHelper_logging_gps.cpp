/*******************************************************************/
/*    NAME: Oscar Viquez                                           */
/*    ORG:  Acbotics Research, LLC                                 */
/*    FILE: InterfaceHelper_logging.cpp                            */
/*    DATE: Apr 4th 2024                                           */
/*                                                                 */
/*    For help, contact us at: support@acbotics.com                */
/*******************************************************************/

#include <fstream>
#include <glog/logging.h>
#include <iomanip>
#include <pthread.h>

// includes from within project
#include "utils/InterfaceHelper.h"

void *InterfaceHelper::_run_logger_thread_gps_csv(void *ptr) {

  InterfaceHelper *argPtr = static_cast<InterfaceHelper *>(ptr);
  prctl(PR_SET_NAME, "ac_log_gps");
  VLOG(3) << "Starting GPS CSV logger in thread " << pthread_self();

  bool &logging_active = argPtr->logger_active[LOGGER::GPS];

  argPtr->logger_running[LOGGER::GPS] = true;

  std::string output_filename;
  std::ostringstream csv_header;

  csv_header << "host_epoch_sec,gps_epoch_sec,lat,lon";

  //   struct gps_data_t latest_gps_data;

  while (argPtr->keep_alive) {
    if (logging_active) {

      gpsmm gps_rec("localhost", DEFAULT_GPSD_PORT);

      if (gps_rec.stream(WATCH_ENABLE | WATCH_JSON) == nullptr) {
        // std::cerr << "No GPSD running.\n";
        // return 1;
        VLOG(3) << "Could not connect to GPSD; service might not be running.";
        sleep(1);
        continue;
      }

      FilenameTime fntime = FilenameTime(argPtr->rollover_min[LOGGER::GPS]);
      output_filename = argPtr->logger_outdir + "GPS_" + fntime.fname_str + ".csv";

      VLOG(5) << "=========== NEW FILE =========== ";
      LOG(INFO) << "Writing to file : " << output_filename;
      std::ofstream ofil(output_filename);
      ofil << csv_header.str() << std::endl;
      int default_precision = ofil.precision();

      while (argPtr->keep_alive && logging_active && ofil.is_open() &&
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

        if (((new_gps_data->fix.latitude == argPtr->latest_gps_data.fix.latitude) ||
             (std::isnan(new_gps_data->fix.latitude) &&
              std::isnan(argPtr->latest_gps_data.fix.latitude))) &&
            ((new_gps_data->fix.longitude == argPtr->latest_gps_data.fix.longitude) ||
             (std::isnan(new_gps_data->fix.longitude) &&
              std::isnan(argPtr->latest_gps_data.fix.longitude))) &&
            (new_gps_data->fix.time.tv_sec == argPtr->latest_gps_data.fix.time.tv_sec) &&
            (new_gps_data->fix.time.tv_nsec == argPtr->latest_gps_data.fix.time.tv_nsec)) {
          //   std::cout << "old ref " << std::endl;
          continue;
        }

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
          ofil << std::setprecision(default_precision) << std::defaultfloat;
          ofil << std::endl;
        }

        argPtr->latest_gps_data = *new_gps_data;

        ofil.flush();
      }
      LOG(INFO) << "Closing file : " << output_filename;
      ofil.close();
    } else {
      usleep(100000);
    }
  }

  argPtr->logger_running[LOGGER::GPS] = false;
  pthread_exit(NULL);
}
