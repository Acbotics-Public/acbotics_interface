/*******************************************************************/
/*    NAME: Sam Fladung                                            */
/*    ORG:  Acbotics Research, LLC                                 */
/*    FILE: UdpBnoData.cpp                                         */
/*    DATE: Jan 22 2025                                            */
/*                                                                 */
/*    For help, contact us at: support@acbotics.com                */
/*******************************************************************/

#include <byteswap.h>
#include <cstring>
#include <glog/logging.h>
#include <iomanip>

// includes from within project
#include "UdpBnoData.h"

UdpBnoData::UdpBnoData(std::vector<int8_t> &buff) {
  std::string buff_start(buff.begin(), buff.end());
  buff_start = buff_start.substr(0, 6);

  if (buff[8] == 'B' && buff[9] == 'N' && buff.size() >= sizeof(Header)) {
    this->header = Header(buff);

    if (buff.size() - sizeof(Header) == 8) {
      this->unpack_data(buff);
    } else {
      LOG(INFO) << "Incomplete buffer; ignoring data payload";
    }
  } else {
    log_invalid_buffer(buff_start);
  }
}

bool UdpBnoData::unpack_data(std::vector<int8_t> &buff) {
  size_t offset = sizeof(Header);

  char sense_key;
  uint8_t status;
  int16_t sense_x;
  int16_t sense_y;
  int16_t sense_z;
  std::memcpy(&sense_key, buff.data() + offset, sizeof(sense_key));
  offset += sizeof(sense_key);
  std::memcpy(&status, buff.data() + offset, sizeof(status));
  offset += sizeof(status);
  std::memcpy(&sense_x, buff.data() + offset, sizeof(sense_x));
  offset += sizeof(sense_x);
  std::memcpy(&sense_y, buff.data() + offset, sizeof(sense_y));
  offset += sizeof(sense_y);
  std::memcpy(&sense_z, buff.data() + offset, sizeof(sense_z));
  offset += sizeof(sense_z);

  auto it = BNO_TYPE_CHAR.find(sense_key);
  if (it != BNO_TYPE_CHAR.end()) {
    this->sense_type = it->second;
  } else {
    this->sense_type = BNO_TYPE::UNKNOWN;
  }

  this->sense_char = sense_key;
  this->status = status;

  this->sense_x = (float)sense_x / (1 << 8);
  this->sense_y = (float)sense_y / (1 << 8);
  this->sense_z = (float)sense_z / (1 << 8);

  return true;
}

void UdpBnoData::csv_header(std::ostream &oss) {
  oss << "host_epoch_sec,data_epoch_nsec,sense_type,status,sense_x,sense_y,sense_z" << std::endl;
}

void UdpBnoData::csv_serialize(std::ostream &oss) {
  oss << std::time(nullptr) << "," << std::fixed << this->header.start_time_nsec << ","
      << (char)this->sense_char << "," << (int)this->status << "," << this->sense_x << ","
      << this->sense_y << "," << this->sense_z << std::endl;
}

std::ostream &operator<<(std::ostream &os, const UdpBnoData &st) {
  std::ostringstream oss;
  oss << st;
  os << oss.str();
  return os;
}
std::ostringstream &operator<<(std::ostringstream &os, const UdpBnoData &st) {
  os << "AcSense UDP protocol : IMU BNO Data" << std::endl << st.header << std::endl;
  os << std::endl;

  int width = 20;

  std::string sense_name;

  switch (st.sense_type) {
  case BNO_TYPE::ACCEL:
    sense_name = "ACCEL";
    break;

  case BNO_TYPE::GYRO:
    sense_name = "GYRO";
    break;

  case BNO_TYPE::MAG:
    sense_name = "MAG";
    break;
  }

  os << "IMU BNO Payload:" << std::endl
     << std::left << std::setw(width) << std::setfill('.') << (sense_name + "_X") << ": "
     << st.sense_x << std::endl
     << std::left << std::setw(width) << std::setfill('.') << (sense_name + "_Y") << ": "
     << st.sense_y << std::endl
     << std::left << std::setw(width) << std::setfill('.') << (sense_name + "_Z") << ": "
     << st.sense_z << std::endl
     << std::endl;

  return os;
}
