/*******************************************************************/
/*    NAME: Oscar Viquez                                           */
/*    ORG:  Acbotics Research, LLC                                 */
/*    FILE: IpcData.cpp                                            */
/*    DATE: Apr 26th 2024                                          */
/*                                                                 */
/*    For help, contact us at: support@acbotics.com                */
/*******************************************************************/

#include <byteswap.h>
#include <cstring>
#include <glog/logging.h>
#include <iomanip>

// includes from within project
#include "IpcData.h"

IpcData::IpcData() {}

void IpcData::csv_header(std::ostream &oss) {
  oss << "host_epoch_sec,data_epoch_nsec,header_packetNum" << std::endl;
}

void IpcData::csv_serialize(std::ostream &oss) {
  oss << std::time(nullptr) << "," << std::fixed << this->header.start_time_nsec << ","
      << this->header.packet_num << std::endl;
}

std::ostream &operator<<(std::ostream &os, const IpcData::Header &st) {
  std::ostringstream oss;
  oss << st;
  os << oss.str();
  return os;
}
std::ostringstream &operator<<(std::ostringstream &os, const IpcData::Header &st) {
  int width = 20;

  time_t start_time_sec = st.start_time_nsec / 1e9;

  struct tm *start_time_tm = std::gmtime(&start_time_sec);
  auto start_time_str = std::put_time(start_time_tm, "%Y-%m-%d %H:%M:%S %Z");

  os << "Generic IPC Data Header:" << std::endl
     << std::left << std::setw(width) << std::setfill('.') << "START_TIME" << ": "
     << st.start_time_nsec << " nsec" << std::endl
     << std::left << std::setw(width) << std::setfill('.') << "" << ": " << start_time_str
     << std::endl
     << std::left << std::setw(width) << std::setfill('.') << "PACKET_NUM" << ": " << st.packet_num
     << std::endl;

  return os;
}

std::ostream &operator<<(std::ostream &os, const IpcData &st) {
  std::ostringstream oss;
  oss << st;
  os << oss.str();
  return os;
}
std::ostringstream &operator<<(std::ostringstream &os, const IpcData &st) {
  os << "AcSense UDP protocol : Generic Data" << std::endl << st.header << std::endl;
  os << std::endl;

  return os;
}
