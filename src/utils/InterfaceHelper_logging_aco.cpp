/*******************************************************************/
/*    NAME: Oscar Viquez                                           */
/*    ORG:  Acbotics Research, LLC                                 */
/*    FILE: InterfaceHelper_logging_aco.cpp                        */
/*    DATE: Apr 4th 2024                                           */
/*                                                                 */
/*    For help, contact us at: support@acbotics.com                */
/*******************************************************************/

#include <fstream>
#include <glog/logging.h>
#include <pthread.h>

// used for logging to audio files -- i.e. WAV, FLAC
#include <sndfile.h>
#include <sndfile.hh>

// includes from within project
#include "utils/InterfaceHelper.h"

void *InterfaceHelper::_run_logger_thread_audio_csv(void *ptr) {

  InterfaceHelper *argPtr = static_cast<InterfaceHelper *>(ptr);
  prctl(PR_SET_NAME, "ac_log_aco_csv");
  VLOG(3) << "Starting audio CSV logger in thread " << pthread_self();

  bool &logging_active = argPtr->logger_active[LOGGER::ACO_CSV];

  argPtr->logger_running[LOGGER::ACO_CSV] = true;

  std::string output_filename;

  std::vector<std::shared_ptr<UdpAcousticData>> aco_data_vec;
  std::shared_ptr<UdpAcousticData> aco_data;
  int8_t num_channels;
  std::ostringstream csv_header;
  std::ofstream ofil;

  bool initialized = false;

  while (argPtr->keep_alive && !initialized) {
    if (logging_active && argPtr->q_aco_dup["csv"]->size() > 0) {
      // custom queue's front() peeks into the first element without popping
      aco_data = argPtr->q_aco_dup["csv"]->front();
      num_channels = aco_data->header.num_channels;

      csv_header << "epoch_nsec,tick_time_nsec,adc_count,packet_num,";
      for (int ii = 0; ii < num_channels - 1; ii++) {
        csv_header << ii << ",";
      }
      csv_header << num_channels - 1;
      initialized = true;
    } else {
      usleep(10000);
    }
  }

  while (argPtr->keep_alive) {
    if (logging_active) {
      FilenameTime fntime = FilenameTime(argPtr->rollover_min[LOGGER::ACO_CSV]);
      output_filename = argPtr->logger_outdir + "ACO_" + fntime.fname_str + ".csv";

      VLOG(5) << "=========== NEW FILE =========== ";
      LOG(INFO) << "Writing to file : " << output_filename;
      ofil = std::ofstream(output_filename);
      ofil << csv_header.str() << std::endl;

      while (argPtr->keep_alive && logging_active && ofil.is_open() &&
             (std::time(nullptr) < fntime.rollover_time)) {
        aco_data_vec = argPtr->q_aco_dup["csv"]->pop_all();
        // aco_data = argPtr->q_aco_dup["csv"]->pop();
        for (int iac = 0; iac < aco_data_vec.size(); iac++) {
          aco_data = aco_data_vec.at(iac);
          if (aco_data->data.size() > 0) {
            for (int ii = 0; ii < (aco_data->header.num_values / aco_data->header.num_channels);
                 ii++) {
              ofil << std::fixed
                   << aco_data->header.start_time_nsec +
                          (int64_t)(ii / (double)aco_data->header.sample_rate * 1e9)
                   << ",";
              ofil << std::fixed
                   << aco_data->header.tick_time_nsec +
                          (int64_t)(ii / (double)aco_data->header.sample_rate * 1e9)
                   << ",";
              ofil << std::fixed << aco_data->header.adc_count + ii << ",";

              ofil << aco_data->header.packet_num << ",";
              for (int ch = 0; ch < num_channels - 1; ch++) {
                ofil << aco_data->data(ch, ii) << ",";
              }
              ofil << aco_data->data(num_channels - 1, ii) << std::endl;
            }
            ofil.flush();
          }
        }
        // rest here, to allow for external control switch
        usleep(10000);
      }
      LOG(INFO) << "Closing file : " << output_filename;
      ofil.close();
    } else {
      // if (argPtr->q_aco_dup["csv"]->size() > 0)
      argPtr->q_aco_dup["csv"]->clear();
      usleep(100000);
    }
  }

  argPtr->logger_running[LOGGER::ACO_CSV] = false;
  pthread_exit(NULL);
}

void *InterfaceHelper::_run_logger_thread_audio_flac(void *ptr) {

  InterfaceHelper *argPtr = static_cast<InterfaceHelper *>(ptr);
  prctl(PR_SET_NAME, "ac_log_aco_flac");
  VLOG(3) << "Starting audio FLAC logger in thread " << pthread_self();

  bool &logging_active = argPtr->logger_active[LOGGER::ACO_FLAC];

  argPtr->logger_running[LOGGER::ACO_FLAC] = true;

  std::vector<std::string> output_filenames;
  std::string output_filename;
  std::vector<SndfileHandle> output_files;

  std::vector<std::shared_ptr<UdpAcousticData>> aco_data_vec;
  std::shared_ptr<UdpAcousticData> aco_data;

  int8_t num_channels;
  int sample_rate;
  int num_files;
  std::vector<int> start_idx_per_file;
  std::vector<int> ch_per_file;

  Eigen::MatrixX<int16_t> buff_mat;

  bool initialized = false;

  while (argPtr->keep_alive && !initialized) {
    if (logging_active && argPtr->q_aco_dup["flac"]->size() > 0) {
      // custom queue's front() peeks into the first element without popping
      aco_data = argPtr->q_aco_dup["flac"]->front();
      num_channels = aco_data->header.num_channels;
      sample_rate = int(aco_data->header.sample_rate);

      // Enforce max allowable of 8ch per FLAC file
      num_files = std::ceil((double)num_channels / 8);
      for (int ii = 0; ii < num_files; ii++) {
        start_idx_per_file.push_back(8 * ii);
        ch_per_file.push_back(8);
      }
      if (num_channels % 8 > 0) {
        ch_per_file.at(ch_per_file.size() - 1) = num_channels % 8;
      }
      initialized = true;
    } else {
      usleep(10000);
    }
  }

  while (argPtr->keep_alive) {
    if (logging_active) {
      FilenameTime fntime = FilenameTime(argPtr->rollover_min[LOGGER::ACO_FLAC]);
      output_filenames.clear();
      output_files.clear();

      for (int ii = 0; ii < num_files; ii++) {
        output_filename =
            argPtr->logger_outdir + "ACO_" + fntime.fname_str + "_" + std::to_string(ii) + ".flac";
        output_filenames.push_back(output_filename);

        output_files.push_back(SndfileHandle(output_filename, SFM_WRITE,
                                             SF_FORMAT_FLAC | SF_FORMAT_PCM_16, ch_per_file.at(ii),
                                             sample_rate));
      }

      VLOG(5) << "=========== NEW FILE(s) =========== ";
      LOG(INFO) << "Writing to files starting with : " << output_filenames.at(0);

      while (argPtr->keep_alive && logging_active && (std::time(nullptr) < fntime.rollover_time)) {

        aco_data_vec = argPtr->q_aco_dup["flac"]->pop_all();
        // aco_data = argPtr->q_aco_dup["flac"]->pop();
        for (int iac = 0; iac < aco_data_vec.size(); iac++) {
          aco_data = aco_data_vec.at(iac);
          for (int ff = 0; ff < num_files; ff++) {

            buff_mat = aco_data->data.block(start_idx_per_file.at(ff), 0, ch_per_file.at(ff),
                                            aco_data->data.cols());
            size_t num_its = buff_mat.rows() * buff_mat.cols() * sizeof(aco_data->data(0));
            int16_t buffer[num_its] = {0};
            memcpy(&buffer, &buff_mat(0), num_its);
            output_files.at(ff).writef(buffer, buff_mat.cols());
          }
        }
        // rest here, to allow for external control switch
        usleep(10000);
      }

      for (int ff = 0; ff < num_files; ff++) {
        output_files.at(ff).writeSync();
        LOG(INFO) << "Closing file : " << output_filenames.at(ff);
      }
      output_filenames.clear();
      output_files.clear();
    } else {
      // if (argPtr->q_aco_dup["flac"]->size() > 0)
      argPtr->q_aco_dup["flac"]->clear();
      usleep(100000);
    }
  }

  argPtr->logger_running[LOGGER::ACO_FLAC] = false;
  pthread_exit(NULL);
}

void *InterfaceHelper::_run_logger_thread_audio_wav(void *ptr) {

  InterfaceHelper *argPtr = static_cast<InterfaceHelper *>(ptr);
  prctl(PR_SET_NAME, "ac_log_aco_wav");
  VLOG(3) << "Starting audio WAV logger in thread " << pthread_self();

  bool &logging_active = argPtr->logger_active[LOGGER::ACO_WAV];

  argPtr->logger_running[LOGGER::ACO_WAV] = true;

  std::string output_filename;

  std::vector<std::shared_ptr<UdpAcousticData>> aco_data_vec;
  std::shared_ptr<UdpAcousticData> aco_data;

  int8_t num_channels;
  int sample_rate;

  bool initialized = false;

  while (argPtr->keep_alive && !initialized) {
    if (logging_active && argPtr->q_aco_dup["wav"]->size() > 0) {
      // custom queue's front() peeks into the first element without popping
      aco_data = argPtr->q_aco_dup["wav"]->front();
      num_channels = aco_data->header.num_channels;
      sample_rate = int(aco_data->header.sample_rate);
      initialized = true;
    } else {
      usleep(10000);
    }
  }

  while (argPtr->keep_alive) {
    if (logging_active) {
      FilenameTime fntime = FilenameTime(argPtr->rollover_min[LOGGER::ACO_WAV]);
      output_filename = argPtr->logger_outdir + "ACO_" + fntime.fname_str + ".wav";

      VLOG(5) << "=========== NEW FILE =========== ";
      LOG(INFO) << "Writing to file : " << output_filename;
      SndfileHandle ofil;

      ofil = SndfileHandle(output_filename, SFM_WRITE, SF_FORMAT_WAV | SF_FORMAT_PCM_16,
                           num_channels, sample_rate);

      while (argPtr->keep_alive && logging_active && (std::time(nullptr) < fntime.rollover_time)) {

        aco_data_vec = argPtr->q_aco_dup["wav"]->pop_all();
        // aco_data = argPtr->q_aco_dup["wav"]->pop();
        for (int iac = 0; iac < aco_data_vec.size(); iac++) {
          aco_data = aco_data_vec.at(iac);
          size_t num_its =
              aco_data->data.rows() * aco_data->data.cols() * sizeof(aco_data->data(0));
          int16_t buffer[num_its] = {0};
          memcpy(&buffer, &aco_data->data(0), num_its);
          ofil.writef(buffer, aco_data->data.cols());
        }
        // rest here, to allow for external control switch
        usleep(10000);
      }

      ofil.writeSync();
      LOG(INFO) << "Closing file : " << output_filename;
      ofil = SndfileHandle();
    } else {
      // if (argPtr->q_aco_dup["wav"]->size() > 0)
      argPtr->q_aco_dup["wav"]->clear();
      usleep(100000);
    }
  }

  argPtr->logger_running[LOGGER::ACO_WAV] = false;
  pthread_exit(NULL);
}
