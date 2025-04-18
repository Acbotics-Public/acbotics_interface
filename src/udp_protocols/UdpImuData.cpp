/*******************************************************************/
/*    NAME: Oscar Viquez                                           */
/*    ORG:  Acbotics Research, LLC                                 */
/*    FILE: UdpImuData.cpp                                         */
/*    DATE: Apr 26th 2024                                          */
/*                                                                 */
/*    For help, contact us at: support@acbotics.com                */
/*******************************************************************/

#include <byteswap.h>
#include <cstring>
#include <glog/logging.h>
#include <iomanip>

// includes from within project
#include "UdpImuData.h"

UdpImuData::UdpImuData(std::vector<int8_t> &buff) {
  std::string buff_start(buff.begin(), buff.end());
  buff_start = buff_start.substr(0, 6);

  if (buff[8] == 'I' && buff[9] == 'M' && buff.size() >= sizeof(Header)) {
    this->header = Header(buff);

    if (buff.size() - sizeof(Header) == 20) {
      this->unpack_data(buff);
    } else {
      LOG(INFO) << "Incomplete buffer; ignoring data payload";
    }
  } else {
    log_invalid_buffer(buff_start);
  }
}

bool UdpImuData::unpack_data(std::vector<int8_t> &buff) {
  size_t offset = sizeof(Header);

  int32_t pitch_ned_deg_x100;
  int32_t roll_ned_deg_x100;

  int16_t accel_x;
  int16_t accel_y;
  int16_t accel_z;
  int16_t gyro_x;
  int16_t gyro_y;
  int16_t gyro_z;

  std::memcpy(&pitch_ned_deg_x100, buff.data() + offset, sizeof(pitch_ned_deg_x100));
  offset += sizeof(pitch_ned_deg_x100);
  std::memcpy(&roll_ned_deg_x100, buff.data() + offset, sizeof(roll_ned_deg_x100));
  offset += sizeof(roll_ned_deg_x100);

  std::memcpy(&accel_x, buff.data() + offset, sizeof(accel_x));
  offset += sizeof(accel_x);
  std::memcpy(&accel_y, buff.data() + offset, sizeof(accel_y));
  offset += sizeof(accel_y);
  std::memcpy(&accel_z, buff.data() + offset, sizeof(accel_z));
  offset += sizeof(accel_z);

  std::memcpy(&gyro_x, buff.data() + offset, sizeof(gyro_x));
  offset += sizeof(gyro_x);
  std::memcpy(&gyro_y, buff.data() + offset, sizeof(gyro_y));
  offset += sizeof(gyro_y);
  std::memcpy(&gyro_z, buff.data() + offset, sizeof(gyro_z));
  offset += sizeof(gyro_z);

  this->pitch_ned_deg = pitch_ned_deg / 100.0;
  this->roll_ned_deg = roll_ned_deg / 100.0;

  this->accel_x = accel_x;
  this->accel_y = accel_y;
  this->accel_z = accel_z;

  this->gyro_x = gyro_x;
  this->gyro_y = gyro_y;
  this->gyro_z = gyro_z;

  return true;
}

void UdpImuData::csv_header(std::ostream &oss) {
  oss << "host_epoch_sec,data_epoch_nsec,accel_x,accel_y,accel_z,gyro_x,gyro_y,gyro_z" << std::endl;
}

void UdpImuData::csv_serialize(std::ostream &oss) {
  oss << std::time(nullptr) << "," << std::fixed << this->header.start_time_nsec << ","
      << this->accel_x << "," << this->accel_y << "," << this->accel_z << "," << this->gyro_x << ","
      << this->gyro_y << "," << this->gyro_z << std::endl;
}

std::ostream &operator<<(std::ostream &os, const UdpImuData &st) {
  std::ostringstream oss;
  oss << st;
  os << oss.str();
  return os;
}
std::ostringstream &operator<<(std::ostringstream &os, const UdpImuData &st) {
  os << "AcSense UDP protocol : IMU Data" << std::endl << st.header << std::endl;
  os << std::endl;

  int width = 20;

  os << "IMU Payload:" << std::endl
     << std::left << std::setw(width) << std::setfill('.') << "PITCH_NED_DEG"
     << ": " << st.pitch_ned_deg << " deg" << std::endl
     << std::left << std::setw(width) << std::setfill('.') << "ROLL_NED_DEG"
     << ": " << st.roll_ned_deg << " deg" << std::endl
     << std::endl
     << std::left << std::setw(width) << std::setfill('.') << "ACCEL_X"
     << ": " << st.accel_x << std::endl
     << std::left << std::setw(width) << std::setfill('.') << "ACCEL_Y"
     << ": " << st.accel_y << std::endl
     << std::left << std::setw(width) << std::setfill('.') << "ACCEL_Z"
     << ": " << st.accel_z << std::endl
     << std::endl
     << std::left << std::setw(width) << std::setfill('.') << "GYRO_X"
     << ": " << st.gyro_x << std::endl
     << std::left << std::setw(width) << std::setfill('.') << "GYRO_Y"
     << ": " << st.gyro_y << std::endl
     << std::left << std::setw(width) << std::setfill('.') << "GYRO_Z"
     << ": " << st.gyro_z << std::endl

     << std::endl
     << std::endl;

  return os;
}
