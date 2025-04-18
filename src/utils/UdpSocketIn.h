/*******************************************************************/
/*    NAME: Oscar Viquez                                           */
/*    ORG:  Acbotics Research, LLC                                 */
/*    FILE: UdpSocketIn.h                                          */
/*    DATE: Apr 4th 2024                                           */
/*                                                                 */
/*    For help, contact us at: support@acbotics.com                */
/*******************************************************************/

#ifndef udp_socket_in_HEADER
#define udp_socket_in_HEADER

#include <iostream>

// includes from within project
#include "utils/QueueClient.h"

DECLARE_bool(debug_socket_in);

enum class MSG_ID { UNKNOWN, ACB2, ACBR, ACBC, ACO, PTS, IMU, EPT, RTC, BNO, BNR };

struct UdpSocketIn {
  bool use_mcast;
  std::string iface_ip;
  int32_t port;
  std::string mcast_group;
  std::vector<ptr_tsQ<UdpAcousticData>> v_q_aco;
  std::vector<ptr_tsQ<UdpBeamform2D>> v_q_beam2d;
  std::vector<ptr_tsQ<UdpBeamformRaw>> v_q_beamraw;
  std::vector<ptr_tsQ<UdpPtsData>> v_q_pts;
  std::vector<ptr_tsQ<UdpImuData>> v_q_imu;
  std::vector<ptr_tsQ<UdpEptData>> v_q_ept;
  std::vector<ptr_tsQ<UdpRtcData>> v_q_rtc;
  std::vector<ptr_tsQ<UdpBnoData>> v_q_bno;
  std::vector<ptr_tsQ<UdpBnrData>> v_q_bnr;

  UdpSocketIn() : UdpSocketIn(false, "127.0.0.1", 9760, "224.1.1.1") {};
  UdpSocketIn(bool use_mcast, std::string iface_ip, int32_t port, std::string mcast_group) {
    this->use_mcast = use_mcast;
    this->iface_ip = iface_ip;
    this->port = port;
    this->mcast_group = mcast_group;

    this->thread_name = "socket_thr";
  };
  ~UdpSocketIn() { this->stop(); }

  void run_socket_main_thread();
  void run_socket_thread();

  void register_client(QueueClient &client);
  bool is_connected();
  void stop();

protected:
  pthread_t own_thread;
  int m_socket;
  bool keep_alive = true;
  bool _is_running = false;
  std::string thread_name;

  static void *_run_socket_thread(void *arg);
  static int configure_socket(UdpSocketIn &args);
  static int check_aco_data(UdpAcousticData aco_data);
  static MSG_ID check_msg_id(std::vector<int8_t> &msg);
};

std::ostream &operator<<(std::ostream &os, const UdpSocketIn &st);
std::ostringstream &operator<<(std::ostringstream &os, const UdpSocketIn &st);

#endif
