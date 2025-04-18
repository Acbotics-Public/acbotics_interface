/*******************************************************************/
/*    NAME: Oscar Viquez                                           */
/*    ORG:  Acbotics Research, LLC                                 */
/*    FILE: EnergyDetector.cpp                                     */
/*    DATE: Feb 14th 2025                                          */
/*                                                                 */
/*    For help, contact us at: support@acbotics.com                */
/*******************************************************************/

#include <glog/logging.h>

// includes from within project
#include "utils/EnergyDetector.h"

DEFINE_bool(debug_energy_detector, false, "Enable expanded debug for Energy Detector");

void *EnergyDetector::_run_detector_thread(void *ptr) {

  EnergyDetector *argPtr = static_cast<EnergyDetector *>(ptr);
  prctl(PR_SET_NAME, argPtr->thread_name.substr(0, 15).c_str());
  VLOG(3) << "Starting energy detector utility in thread " << pthread_self();
  argPtr->_is_running = true;

  std::vector<std::shared_ptr<IpcFFT>> new_packets;
  std::shared_ptr<IpcFFT> ipc_fft;
  Eigen::ArrayXXd fft_buffer_in;
  Eigen::ArrayXXd fft_buffer;

  // Sxx is using peak-over-mean per ch
  // (NOT median; due to Eigen built-in convenience)
  Eigen::ArrayXd Sxx_ratio;

  // Use distict prior / new; Eigen does not like in-place/self-referencing assignment
  Eigen::ArrayXd ema_st_old;
  Eigen::ArrayXd ema_lt_old;
  Eigen::ArrayXd ema_st_new;
  Eigen::ArrayXd ema_lt_new;

  std::shared_ptr<IpcDetector> detect;

  double alpha_st = 0.1;    // 2/(N+1) : N=19 (20 points produce >90% of weight distrib)
  double alpha_lt = 0.0001; // 2/(N+1) : N~20k (20k points produce >90% of weight distrib)

  double threshold = 1.2;

  bool initialized = false;

  while (argPtr->sample_rate <= 0 && argPtr->keep_alive) {
    usleep(100000);
  }

  while (argPtr->keep_alive && !initialized) {
    if (argPtr->q_fft->size() > 0) {
      fft_buffer_in = (*argPtr->q_fft->front()).fft.array().abs();
      argPtr->_rx_runtime_update = true;
      initialized = true;
    } else {
      usleep(10000);
    }
  }

  while (argPtr->keep_alive) {
    if (argPtr->_rx_runtime_update) {
      fft_buffer = fft_buffer_in(Eigen::all, argPtr->active_frequencies) *
                   std::pow(10, -argPtr->phone_sensitivity_V_uPa / 20);
      if (argPtr->active_frequencies.size() > 0) {
        Sxx_ratio = fft_buffer.rowwise().maxCoeff() / fft_buffer.rowwise().mean();
        ema_st_old = Sxx_ratio;
        ema_lt_old = Sxx_ratio;
      }
      argPtr->_rx_runtime_update = false;
    }

    if (argPtr->sample_rate > 0 && argPtr->q_fft->pop_all(new_packets)) {

      for (int ii = 0; ii < new_packets.size(); ii++) {

        ipc_fft = new_packets.at(ii);
        fft_buffer_in = ipc_fft->fft.array().abs();
        fft_buffer = fft_buffer_in(Eigen::all, argPtr->active_frequencies) *
                     std::pow(10, -argPtr->phone_sensitivity_V_uPa / 20);

        detect = std::make_shared<IpcDetector>();
        detect->header.start_time_nsec = ipc_fft->header.start_time_nsec;
        detect->header.packet_num = ipc_fft->header.packet_num;

        if (argPtr->active_frequencies.size() > 0) {
          Sxx_ratio = fft_buffer.rowwise().maxCoeff() / fft_buffer.rowwise().mean();

          ema_st_new = (alpha_st * Sxx_ratio) + (1.0 - alpha_st) * ema_st_old;
          ema_lt_new = (alpha_lt * Sxx_ratio) + (1.0 - alpha_lt) * ema_lt_old;

          detect->detections = ((ema_st_new / ema_lt_new) > threshold).cast<int>().sum();

          ema_st_old = ema_st_new;
          ema_lt_old = ema_lt_new;
        } else {
          detect->detections = 0;
        }
        for (auto q_detect : argPtr->v_q_detect) {
          q_detect->push(detect);
        }
      }
    }
    usleep(1000);
  }

  argPtr->_is_running = false;
  pthread_exit(NULL);
}

void EnergyDetector::run() {
  pthread_t thread;
  this->keep_alive = true;
  if (!this->is_running()) {
    pthread_create(&thread, NULL, this->_run_detector_thread, this);
    this->own_thread = thread;
  }
}

// Register output clients
// =======================
void EnergyDetector::register_client(QueueClient &client) {
  if (client.q_detect != nullptr) {
    auto init_count = v_q_detect.size();
    v_q_detect.push_back(client.q_detect);
    if (FLAGS_debug_energy_detector)
      VLOG(1) << "Q_DETECT clients changed from " << init_count << " to " << v_q_detect.size();
  } else {
    LOG(WARNING) << "Cannot register DETECT data queue; received nullptr!";
  }
}
