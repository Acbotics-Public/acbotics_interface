/*******************************************************************/
/*    NAME: Oscar Viquez                                           */
/*    ORG:  Acbotics Research, LLC                                 */
/*    FILE: UdpSocketIn.cpp                                        */
/*    DATE: Apr 4th 2024                                           */
/*                                                                 */
/*    For help, contact us at: support@acbotics.com                */
/*******************************************************************/

#include <arpa/inet.h>
#include <cstring>
#include <fstream>
#include <gflags/gflags.h>
#include <glog/logging.h>
#include <iomanip>
#include <pthread.h>
#include <sys/socket.h>
#include <vector>

// includes from within project
#include "utils/UdpSocketIn.h"

DEFINE_bool(debug_socket_in, false, "Enable expanded debug for UdpSocketIn");

void *UdpSocketIn::_run_socket_thread(void *ptr) {

  UdpSocketIn *argPtr = static_cast<UdpSocketIn *>(ptr);
  prctl(PR_SET_NAME, argPtr->thread_name.substr(0, 15).c_str());
  VLOG(3) << "Starting socket in thread " << pthread_self() << std::endl << *argPtr;
  argPtr->_is_running = true;

  argPtr->m_socket = configure_socket(*argPtr);
  int retry_counter = 0;
  while (argPtr->keep_alive && argPtr->m_socket < 0 && retry_counter < 10) {
    sleep(5);
    LOG(INFO) << "Retrying socket @ " << pthread_self();
    argPtr->m_socket = configure_socket(*argPtr);
  }
  if (argPtr->m_socket < 0) {
    LOG(WARNING) << "Could not open socket! Terminating thread for:" << std::endl << *argPtr;
    argPtr->_is_running = false;
    pthread_exit(NULL);
  }

  std::vector<int8_t> buff;
  std::vector<int8_t> msg;
  buff.resize(65535);

  ssize_t msg_len;
  std::shared_ptr<UdpAcousticData> aco_data;
  std::shared_ptr<UdpBeamform2D> beam_2d;
  std::shared_ptr<UdpBeamformRaw> beam_raw_0;
  std::shared_ptr<UdpBeamformRaw> beam_raw_c;
  std::shared_ptr<UdpPtsData> pts_data;
  std::shared_ptr<UdpImuData> imu_data;
  std::shared_ptr<UdpEptData> ept_data;
  std::shared_ptr<UdpRtcData> rtc_data;
  std::shared_ptr<UdpBnoData> bno_data;
  std::shared_ptr<UdpBnrData> bnr_data;

  while (argPtr->keep_alive) {
    msg_len = recv(argPtr->m_socket, buff.data(), 65535, 0);
    msg = std::vector<int8_t>(buff.begin(), buff.begin() + msg_len);

    switch (check_msg_id(msg)) {
    case MSG_ID::ACB2:
      if (FLAGS_debug_socket_in)
        VLOG(3) << "Received ACB2";
      beam_2d = std::make_shared<UdpBeamform2D>(msg);

      for (auto q_beam2d : argPtr->v_q_beam2d) {
        q_beam2d->push(beam_2d);
      }
      break;
    case MSG_ID::ACBR:
      if (FLAGS_debug_socket_in)
        VLOG(3) << "Received ACBR";
      beam_raw_0 = std::make_shared<UdpBeamformRaw>(msg);
      if (beam_raw_0->header.start_time_nsec >= 0)
        for (auto q_beamraw : argPtr->v_q_beamraw) {
          q_beamraw->push(beam_raw_0);
        }

      break;
    case MSG_ID::ACBC:
      if (FLAGS_debug_socket_in)
        VLOG(3) << "Received ACBC";
      beam_raw_c = std::make_shared<UdpBeamformRaw>(msg);
      if (beam_raw_c->header.packet_num == beam_raw_0->header.packet_num) {
        if (beam_raw_0->add_cont_packet(msg, beam_raw_c->header.num_subpackets)) {
          // if primary packet is complete, add it to queue for downstream clients
          for (auto q_beamraw : argPtr->v_q_beamraw) {
            q_beamraw->push(beam_raw_0);
          }
        }
      }
      break;
    case MSG_ID::ACO:
      aco_data = std::make_shared<UdpAcousticData>(msg);
      if (check_aco_data(*aco_data) == 0) {
        if (FLAGS_debug_socket_in)
          VLOG(3) << "Received AC; socket thread heartbeat" << " : ID " << pthread_self()
                  << " : latest packet num : " << aco_data->header.packet_num;
        for (auto q_aco : argPtr->v_q_aco) {
          q_aco->push(aco_data);
        }
      }

      break;
    case MSG_ID::PTS:
      if (FLAGS_debug_socket_in)
        VLOG(3) << "Received PTS";
      pts_data = std::make_shared<UdpPtsData>(msg);
      for (auto q_pts : argPtr->v_q_pts) {
        q_pts->push(pts_data);
      }
      break;
    case MSG_ID::IMU:
      if (FLAGS_debug_socket_in)
        VLOG(3) << "Received IMU";
      imu_data = std::make_shared<UdpImuData>(msg);
      for (auto q_imu : argPtr->v_q_imu) {
        q_imu->push(imu_data);
      }
      break;
    case MSG_ID::BNO:
      if (FLAGS_debug_socket_in)
        VLOG(3) << "Received BNO";
      bno_data = std::make_shared<UdpBnoData>(msg);
      for (auto q_bno : argPtr->v_q_bno) {
        q_bno->push(bno_data);
      }
      break;
    case MSG_ID::BNR:
      if (FLAGS_debug_socket_in)
        VLOG(3) << "Received BNR";
      bnr_data = std::make_shared<UdpBnrData>(msg);
      for (auto q_bnr : argPtr->v_q_bnr) {
        q_bnr->push(bnr_data);
      }
      break;

    case MSG_ID::EPT:
      if (FLAGS_debug_socket_in)
        VLOG(3) << "Received EPT";
      ept_data = std::make_shared<UdpEptData>(msg);
      for (auto q_ept : argPtr->v_q_ept) {
        q_ept->push(ept_data);
      }
      break;
    case MSG_ID::RTC:
      if (FLAGS_debug_socket_in)
        VLOG(3) << "Received RTC";
      rtc_data = std::make_shared<UdpRtcData>(msg);
      for (auto q_rtc : argPtr->v_q_rtc) {
        q_rtc->push(rtc_data);
      }
      break;

    default:
      break;
    }
    if (FLAGS_debug_socket_in)
      VLOG(2) << "Socket thread heartbeat : ID " << pthread_self();
  }

  argPtr->_is_running = false;
  pthread_exit(NULL);
}

void UdpSocketIn::run_socket_thread() {
  pthread_t thread;
  pthread_create(&thread, NULL, _run_socket_thread, this);
  this->own_thread = thread;
}

void UdpSocketIn::stop() {
  bool was_running = this->_is_running;
  this->keep_alive = false;
  if (this->_is_running) {
    // small delay before pthread_tryjoin_np()
    usleep(1000);
    int s;
    if ((s = pthread_tryjoin_np(this->own_thread, NULL)) != 0) {
      VLOG(1) << "(socket port: " << this->port << ") waiting for thread...";
      sleep(2);
      if ((s = pthread_tryjoin_np(this->own_thread, NULL)) != 0) {
        VLOG(1) << "(socket port: " << this->port << ") cancelling hanging thread ";
        pthread_cancel(this->own_thread);
        this->_is_running = false;
      }
    }
  }
  if (this->own_thread > 0 && was_running)
    VLOG(1) << "Socket thread closed (port " << this->port << ")";
}

void UdpSocketIn::run_socket_main_thread() {
  VLOG(4) << "Starting socket in main thread ";

  UdpSocketIn *argPtr = static_cast<UdpSocketIn *>(this);
  this->m_socket = configure_socket(*argPtr);
  int retry_counter = 0;
  while (this->m_socket < 0 && retry_counter < 10) {
    sleep(5);
    LOG(INFO) << "Retrying socket (main thread)";
    this->m_socket = configure_socket(*argPtr);
  }
  if (this->m_socket < 0) {
    LOG(WARNING) << "Could not open socket! Terminating thread for:" << std::endl << *argPtr;
    return;
  }

  std::vector<int8_t> buff;
  std::vector<int8_t> msg;
  buff.resize(65535);

  ssize_t msg_len;
  std::shared_ptr<UdpAcousticData> aco_data;
  while (true) {

    msg_len = recv(this->m_socket, buff.data(), 65535, 0);
    msg = std::vector<int8_t>(buff.begin(), buff.begin() + msg_len);

    aco_data = std::make_shared<UdpAcousticData>(msg);
    if (check_aco_data(*aco_data) == 0) {
      VLOG_EVERY_N(5, 1000) << "App heartbeat : latest packet num : "
                            << aco_data->header.packet_num;
      for (auto q_aco : argPtr->v_q_aco) {
        q_aco->push(aco_data);
      }
    }
  }
}

int UdpSocketIn::check_aco_data(UdpAcousticData aco_data) {
  if (aco_data.data.cols() == aco_data.header.num_values / aco_data.header.num_channels) {
    return 0;
  } else {
    return -1;
  };
}

MSG_ID UdpSocketIn::check_msg_id(std::vector<int8_t> &msg) {
  MSG_ID id = MSG_ID::UNKNOWN;

  if (msg.size() < 4) {
    return id;
  }

  std::string str_id(msg.begin(), msg.begin() + 10);

  if (str_id.substr(0, 4) == "ACB2") {
    id = MSG_ID::ACB2;
  } else if (str_id.substr(0, 4) == "ACBR") {
    id = MSG_ID::ACBR;
  } else if (str_id.substr(0, 4) == "ACBC") {
    id = MSG_ID::ACBC;
  } else if (str_id.substr(0, 2) == "AC") {
    id = MSG_ID::ACO;
  } else if (str_id.substr(8, 2) == "PT") {
    id = MSG_ID::PTS;
  } else if (str_id.substr(8, 2) == "IM") {
    id = MSG_ID::IMU;
  } else if (str_id.substr(8, 2) == "EP") {
    id = MSG_ID::EPT;
  } else if (str_id.substr(8, 2) == "ED") {
    id = MSG_ID::EPT;
  } else if (str_id.substr(8, 2) == "RT") {
    id = MSG_ID::RTC;
  } else if (str_id.substr(8, 2) == "BN") {
    id = MSG_ID::BNO;
  } else if (str_id.substr(8, 2) == "BR") {
    id = MSG_ID::BNR;

  } else {
    LOG(WARNING) << "Unknown packet received : " << str_id << "(" << std::hex << str_id << ")";
  }

  return id;
}

int UdpSocketIn::configure_socket(UdpSocketIn &args) {
  int sock;
  sockaddr_in m_addr;
  const int enable = 1;

  if ((sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) < 0) {
    LOG(ERROR) << "Could not create socket!";
    return -1;
  }

  if (setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &enable, sizeof(enable)) < 0) {
    LOG(ERROR) << "Could not set socket option to reuse address!";
    return -1;
  }

  if (args.use_mcast) {
    VLOG(5) << "Enabling multicast";
    // inet_pton(AF_INET, , &m_addr);
    m_addr.sin_family = AF_INET;
    m_addr.sin_addr.s_addr = INADDR_ANY;
    m_addr.sin_port = htons(args.port);
    VLOG(5) << "Binding...";

    if (bind(sock, (struct sockaddr *)&m_addr, sizeof(m_addr)) < -1) {
      LOG(ERROR) << "Could not bind socket! (multicast UDP)" << args.iface_ip;

      return -1;
    }

    VLOG(5) << "Adding Multicast address...";
    ip_mreq mreq;
    mreq.imr_multiaddr.s_addr = inet_addr("224.1.1.1");
    mreq.imr_interface.s_addr = inet_addr(args.iface_ip.c_str());

    if (setsockopt(sock, IPPROTO_IP, IP_ADD_MEMBERSHIP, &mreq, sizeof(mreq)) < 0) {
      LOG(ERROR) << "Could not add multicast address : " << args.mcast_group;
      LOG(ERROR) << "This may occur if the interface IP " << args.iface_ip << " is not available";

      return -1;
    }
  } else {
    m_addr.sin_family = AF_INET;
    m_addr.sin_addr.s_addr = inet_addr(args.iface_ip.c_str());
    m_addr.sin_port = htons(args.port);

    if (bind(sock, (struct sockaddr *)&m_addr, sizeof(m_addr)) < -1) {
      LOG(ERROR) << "Could not bind socket! (point-to-point UDP) " << args.iface_ip;

      return -1;
    }
  }

  return sock;
}

void UdpSocketIn::register_client(QueueClient &client) {
  LOG(INFO) << "Registering " << client.get_name();

  if (client.q_aco != nullptr) {
    auto init_count = client.q_aco.use_count();
    v_q_aco.push_back(client.q_aco);
    VLOG(5) << "Q_ACO count changed from " << init_count << " to " << client.q_aco.use_count();
  } else {
    LOG(WARNING) << "Cannot register acoustic data queue; received nullptr!";
  }

  if (client.q_beam2d != nullptr) {
    v_q_beam2d.push_back(client.q_beam2d);
  } else {
    LOG(WARNING) << "Cannot register 2D beamformed data queue; received nullptr!";
  }

  if (client.q_beamraw != nullptr) {
    v_q_beamraw.push_back(client.q_beamraw);
  } else {
    LOG(WARNING) << "Cannot register raw beamformed data queue; received nullptr!";
  }

  if (client.q_pts != nullptr) {
    v_q_pts.push_back(client.q_pts);
  } else {
    LOG(WARNING) << "Cannot register PTS data queue; received nullptr!";
  }

  if (client.q_imu != nullptr) {
    v_q_imu.push_back(client.q_imu);
  } else {
    LOG(WARNING) << "Cannot register IMU data queue; received nullptr!";
  }

  if (client.q_bno != nullptr) {
    v_q_bno.push_back(client.q_bno);
  } else {
    LOG(WARNING) << "Cannot register BNO data queue; received nullptr!";
  }

  if (client.q_bnr != nullptr) {
    v_q_bnr.push_back(client.q_bnr);
  } else {
    LOG(WARNING) << "Cannot register BNR data queue; received nullptr!";
  }

  if (client.q_ept != nullptr) {
    v_q_ept.push_back(client.q_ept);
  } else {
    LOG(WARNING) << "Cannot register EPT data queue; received nullptr!";
  }

  if (client.q_rtc != nullptr) {
    v_q_rtc.push_back(client.q_rtc);
  } else {
    LOG(WARNING) << "Cannot register RTC data queue; received nullptr!";
  }
}

bool UdpSocketIn::is_connected() { return this->m_socket > 0; }

std::ostream &operator<<(std::ostream &os, const UdpSocketIn &st) {
  std::ostringstream oss;
  oss << st;
  os << oss.str();
  return os;
}
std::ostringstream &operator<<(std::ostringstream &os, const UdpSocketIn &st) {
  int width = 20;

  os << "Socket parameters : " << std::endl
     << std::left << std::setw(width) << std::setfill('.') << "use_mcast" << ": " << std::boolalpha
     << st.use_mcast << std::endl
     << std::left << std::setw(width) << std::setfill('.') << "mcast_group" << ": "
     << st.mcast_group << std::endl
     << std::left << std::setw(width) << std::setfill('.') << "iface_ip" << ": " << st.iface_ip
     << std::endl
     << std::left << std::setw(width) << std::setfill('.') << "port" << ": " << st.port << std::endl
     << std::endl;

  return os;
}
