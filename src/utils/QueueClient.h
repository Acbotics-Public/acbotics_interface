/*******************************************************************/
/*    NAME: Oscar Viquez                                           */
/*    ORG:  Acbotics Research, LLC                                 */
/*    FILE: QueueClient.h                                          */
/*    DATE: May 21th 2024                                          */
/*                                                                 */
/*    For help, contact us at: support@acbotics.com                */
/*******************************************************************/

#ifndef queue_client_HEADER
#define queue_client_HEADER

#include <sys/prctl.h>

#include "utils/Types.h"

#include "ipc_protocols/IpcDetector.h"
#include "ipc_protocols/IpcFFT.h"

#include "udp_protocols/UdpAcousticData.h"
#include "udp_protocols/UdpBeamform2D.h"
#include "udp_protocols/UdpBeamformRaw.h"
#include "udp_protocols/UdpBnoData.h"
#include "udp_protocols/UdpBnrData.h"
#include "udp_protocols/UdpEptData.h"
#include "udp_protocols/UdpImuData.h"
#include "udp_protocols/UdpPtsData.h"
#include "udp_protocols/UdpRtcData.h"

#include "utils/thread_safe_queue.h"

template <typename T> using tsQ_T = tsQueue<std::shared_ptr<T>>;
template <typename T> using ptr_tsQ = std::shared_ptr<tsQ_T<T>>;

struct QueueClient {

  ptr_tsQ<UdpAcousticData> q_aco;

  ptr_tsQ<UdpBeamform2D> q_beam2d;
  ptr_tsQ<UdpBeamformRaw> q_beamraw;

  ptr_tsQ<UdpPtsData> q_pts;
  ptr_tsQ<UdpImuData> q_imu;
  ptr_tsQ<UdpEptData> q_ept;
  ptr_tsQ<UdpRtcData> q_rtc;
  ptr_tsQ<UdpBnoData> q_bno;
  ptr_tsQ<UdpBnrData> q_bnr;

  ptr_tsQ<IpcFFT> q_fft;

  ptr_tsQ<IpcDetector> q_detect;

  std::unordered_map<QUEUE, std::shared_ptr<tsQueueBase>> queue;

  QueueClient() {
    this->q_aco = std::make_shared<tsQ_T<UdpAcousticData>>();

    this->q_beam2d = std::make_shared<tsQ_T<UdpBeamform2D>>();
    this->q_beamraw = std::make_shared<tsQ_T<UdpBeamformRaw>>();

    this->q_pts = std::make_shared<tsQ_T<UdpPtsData>>();
    this->q_imu = std::make_shared<tsQ_T<UdpImuData>>();
    this->q_ept = std::make_shared<tsQ_T<UdpEptData>>();
    this->q_rtc = std::make_shared<tsQ_T<UdpRtcData>>();
    this->q_bno = std::make_shared<tsQ_T<UdpBnoData>>();
    this->q_bnr = std::make_shared<tsQ_T<UdpBnrData>>();

    this->q_fft = std::make_shared<tsQ_T<IpcFFT>>();

    this->q_detect = std::make_shared<tsQ_T<IpcDetector>>();

    this->queue[QUEUE::ACO] = this->q_aco;
    this->queue[QUEUE::PTS] = this->q_pts;
    this->queue[QUEUE::IMU] = this->q_imu;
    this->queue[QUEUE::EPT] = this->q_ept;
    this->queue[QUEUE::RTC] = this->q_rtc;
    this->queue[QUEUE::BNO] = this->q_bno;
    this->queue[QUEUE::BNR] = this->q_bnr;

    this->keep_alive = true;
    this->_is_running = false;
    this->thread_name = "q_client_thr";
  };

  ~QueueClient() {
    VLOG(1) << "Cleaning up QueueClient " << this->thread_name;
    this->stop();
  }

  virtual void run() {
    this->keep_alive = true;
    LOG(WARNING) << "Not implemented";
  };
  virtual void stop() {
    VLOG(1) << "Calling STOP on " << this->thread_name;
    bool was_running = this->_is_running;
    this->keep_alive = false;
    if (this->_is_running) {
      pthread_join(this->own_thread, NULL);
    }
    if (this->own_thread > 0 && was_running)
      VLOG(1) << "Thread " << this->thread_name << " (" << this->own_thread << ")" << " exited";
  };
  virtual bool is_running() { return this->_is_running; }
  std::string get_name() { return this->thread_name; }

protected:
  pthread_t own_thread;
  bool keep_alive;
  bool _is_running;
  std::string thread_name;
};

#endif
