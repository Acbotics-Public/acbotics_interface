#ifndef LoggerBlock_HEADER
#define LoggerBlock_HEADER

#include "utils/QueueClient.h"

#include "utils/Logger_Acoustic.h"
#include "utils/Types.h"
#include <chrono>
class LoggerBlock : public QueueClient {
public:
  void set_outdir(std::string logger_outdir);
  // void set_rollover(LOGGER logger, float rollover_min);
  // void enable_logger(LOGGER, bool);
  void start_logging();
  void stop_logging();
  void run_log_thread_audio();
  void set_rollover_min(float min);
  std::vector<std::string> get_current_paths(void);
  static void *_run_logger_thread_audio(void *arg);

  static void *_run_thread_bno_state_buffer(void *arg);

  void run() override;
  void stop_threads();
  std::shared_ptr<tsQueue<std::shared_ptr<UdpAcousticData>>>get_input_queue()
  {
    return this->q_aco;
  };

  //   template <typename T, LOGGE    R L> T get_latest_data();


protected:
  Logger_Acoustic_CSV csv_logger;
  Logger_Acoustic_FLAC flac_logger;
  Logger_Acoustic_WAV wav_logger;
  std::string logger_outdir;
  pthread_t _thread;

  struct FilenameTime {
    std::time_t fname_time;
    std::time_t rollover_time;
    std::string fname_str;

    FilenameTime() : FilenameTime(-1) {}
    FilenameTime(int rollover_min);

  protected:
    int rollover_min_default = 5;
  };

  // template <typename T, LOGGER L> static void *_run_csv_logger_thread(void *ptr);
  bool initialized=false;
};

// template <typename T, LOGGER L> void *LoggerBlock::_run_csv_logger_thread(void *ptr) {

//   LoggerBlock *argPtr = static_cast<LoggerBlock *>(ptr);
//   prctl(PR_SET_NAME, ("ac_log_" + LOGGER_NAME[L]).c_str());
//   VLOG(3) << "Starting " << LOGGER_NAME[L] << " CSV logger in thread " << pthread_self();

//   bool &logging_active = argPtr->logger_active[L];

//   argPtr->logger_running[L] = true;

//   std::string output_filename;

//   std::vector<std::shared_ptr<T>> _data_vec;
//   std::shared_ptr<T> _data;

//   ptr_tsQ<T> queue = std::static_pointer_cast<tsQ_T<T>>(argPtr->queue[LOGGER_QUEUE[L]]);

//   bool wrote_header = false;

//   while (argPtr->keep_alive) {
//     if (logging_active) {
//       FilenameTime fntime = FilenameTime(argPtr->rollover_min[L]);
//       output_filename = argPtr->logger_outdir + LOGGER_NAME[L] + "_" + fntime.fname_str + ".csv";

//       VLOG(5) << "=========== NEW FILE =========== ";
//       LOG(INFO) << "Writing to file : " << output_filename;
//       std::ofstream ofil(output_filename);
//       wrote_header = false;

//       while (argPtr->keep_alive && logging_active && ofil.is_open() &&
//              (std::time(nullptr) < fntime.rollover_time)) {
//         if (queue->pop_all(_data_vec)) {
//           if (!wrote_header) {
//             // Check for header bool when queue data is available, to
//             // allow data-driven header (e.g. num_ch in acoustic data)
//             _data_vec.front()->csv_header(ofil);
//             wrote_header = true;
//           }

//           //   argPtr->latest_data[L] = _data_vec.back();
//           for (int ii = 0; ii < _data_vec.size(); ii++) {
//             _data = _data_vec.at(ii);
//             if (_data->header.start_time_nsec >= 0) {
//               _data->csv_serialize(ofil);
//             }
//           }
//           ofil.flush();
//         }
//         // rest here, to allow for external control switch
//         usleep(10000);
//       }
//       LOG(INFO) << "Closing file : " << output_filename;
//       ofil.close();
//     } else {
//       //   if (queue->size() > 0)
//       // argPtr->latest_data[L] = queue->pop_all().back();
//       usleep(100000);
//     }
//   }

//   argPtr->logger_running[LOGGER::PTS] = false;
//   pthread_exit(NULL);
// }

// template <typename T, LOGGER L> T LoggerBlock::get_latest_data() {
//   T _data;
//   ptr_tsQ<T> queue = std::static_pointer_cast<tsQ_T<T>>(this->queue[LOGGER_QUEUE[L]]);

//   auto ptr = this->latest_data[L].get();

//   if (this->logger_running[L] && ptr != nullptr) {
//     _data = *static_cast<T *>(ptr);
//   } else if (queue->size() > 0) {
//     _data = *(queue->pop_all().back());
//   }

//   return _data;
// }

#endif