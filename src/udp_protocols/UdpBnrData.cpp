/*******************************************************************/
/*    NAME: Sam Fladung                                            */
/*    ORG:  Acbotics Research, LLC                                 */
/*    FILE: UdpBnrData.cpp                                         */
/*    DATE: Feb 18 2025                                            */
/*                                                                 */
/*    For help, contact us at: support@acbotics.com                */
/*******************************************************************/

#include <byteswap.h>
#include <cstring>
#include <glog/logging.h>
#include <iomanip>

// includes from within project
#include "UdpBnrData.h"

UdpBnrData::UdpBnrData(std::vector<int8_t> &buff) {
  std::string buff_start(buff.begin(), buff.end());
  buff_start = buff_start.substr(0, 6);

  if (buff[8] == 'B' && buff[9] == 'R' && buff.size() >= sizeof(Header)) {
    this->header = Header(buff);

    if (buff.size() - sizeof(Header) == 11) {
      this->unpack_data(buff);
    } else {
      LOG(INFO) << "Incomplete buffer; ignoring data payload";
    }
  } else {
    log_invalid_buffer(buff_start);
  }
}

bool UdpBnrData::unpack_data(std::vector<int8_t> &buff) {
  size_t offset = sizeof(Header);

  uint8_t status;
  int16_t quat_i;
  int16_t quat_j;
  int16_t quat_k;
  int16_t quat_r;
  int16_t accuracy;
  std::memcpy(&status, buff.data() + offset, sizeof(status));
  offset += sizeof(status);
  std::memcpy(&quat_i, buff.data() + offset, sizeof(quat_i));
  offset += sizeof(quat_i);
  std::memcpy(&quat_j, buff.data() + offset, sizeof(quat_j));
  offset += sizeof(quat_j);
  std::memcpy(&quat_k, buff.data() + offset, sizeof(quat_k));
  offset += sizeof(quat_k);
  std::memcpy(&quat_r, buff.data() + offset, sizeof(quat_r));
  offset += sizeof(quat_r);
  std::memcpy(&accuracy, buff.data() + offset, sizeof(accuracy));
  offset += sizeof(accuracy);

  this->status = status;

  this->quat_i = (float)quat_i / (1 << 14);
  this->quat_j = (float)quat_j / (1 << 14);
  this->quat_k = (float)quat_k / (1 << 14);
  this->quat_r = (float)quat_r / (1 << 14);
  this->accuracy = (float)accuracy / (1 << 12);

  return true;
}

void UdpBnrData::csv_header(std::ostream &oss) {
  oss << "host_epoch_sec,data_epoch_nsec,status,quat_i,quat_j,quat_k,quat_r,accuracy" << std::endl;
}

void UdpBnrData::csv_serialize(std::ostream &oss) {
  oss << std::time(nullptr) << "," << std::fixed << this->header.start_time_nsec << ","
      << this->status << "," << this->quat_i << "," << this->quat_j << "," << this->quat_k << ","
      << this->quat_r << "," << this->accuracy << std::endl;
}

std::ostream &operator<<(std::ostream &os, const UdpBnrData &st) {
  std::ostringstream oss;
  oss << st;
  os << oss.str();
  return os;
}
std::ostringstream &operator<<(std::ostringstream &os, const UdpBnrData &st) {
  os << "AcSense UDP protocol : IMU BNR Data" << std::endl << st.header << std::endl;
  os << std::endl;

  int width = 20;

  os << "IMU BNR Payload:" << std::endl
     << std::left << std::setw(width) << std::setfill('.') << "QUAT_I"
     << ": " << st.quat_i << std::endl
     << std::left << std::setw(width) << std::setfill('.') << "QUAT_J"
     << ": " << st.quat_j << std::endl
     << std::left << std::setw(width) << std::setfill('.') << "QUAT_K"
     << ": " << st.quat_k << std::endl
     << std::left << std::setw(width) << std::setfill('.') << "QUAT_R"
     << ": " << st.quat_k << std::endl
     << std::endl;

  return os;
}
