/*******************************************************************/
/*    NAME: Oscar Viquez                                           */
/*    ORG:  Acbotics Research, LLC                                 */
/*    FILE: UdpPtsData.cpp                                         */
/*    DATE: Apr 26th 2024                                          */
/*                                                                 */
/*    For help, contact us at: support@acbotics.com                */
/*******************************************************************/

#include <byteswap.h>
#include <cstring>
#include <glog/logging.h>
#include <iomanip>

// includes from within project
#include "UdpPtsData.h"

UdpPtsData::UdpPtsData(std::vector<int8_t> &buff) {
  std::string buff_start(buff.begin(), buff.end());
  buff_start = buff_start.substr(0, 6);

  if (buff[8] == 'P' && buff[9] == 'T' && buff.size() >= sizeof(Header)) {
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

bool UdpPtsData::unpack_data(std::vector<int8_t> &buff) {
  size_t offset = sizeof(Header);

  u_int32_t pressure_mbar_x100;
  int32_t temperature_c_x100;

  std::memcpy(&pressure_mbar_x100, buff.data() + offset, sizeof(pressure_mbar_x100));
  offset += sizeof(pressure_mbar_x100);
  std::memcpy(&temperature_c_x100, buff.data() + offset, sizeof(temperature_c_x100));
  offset += sizeof(temperature_c_x100);

  this->pressure_mbar = pressure_mbar_x100 / 100.0;
  this->temperature_c = temperature_c_x100 / 100.0;

  return true;
}

void UdpPtsData::csv_header(std::ostream &oss) {
  oss << "host_epoch_sec,data_epoch_nsec,pressure_mbar,temperature_C" << std::endl;
}

void UdpPtsData::csv_serialize(std::ostream &oss) {
  oss << std::time(nullptr) << "," << std::fixed << this->header.start_time_nsec << ","
      << this->pressure_mbar << "," << this->temperature_c << std::endl;
}

std::ostream &operator<<(std::ostream &os, const UdpPtsData &st) {
  std::ostringstream oss;
  oss << st;
  os << oss.str();
  return os;
}
std::ostringstream &operator<<(std::ostringstream &os, const UdpPtsData &st) {
  os << "AcSense UDP protocol : PTS Data" << std::endl << st.header << std::endl;
  os << std::endl;

  int width = 20;

  os << "PTS Payload:" << std::endl
     << std::left << std::setw(width) << std::setfill('.') << "PRESSURE_MBAR"
     << ": " << st.pressure_mbar << " mbar" << std::endl
     << std::left << std::setw(width) << std::setfill('.') << "TEMPERATURE_C"
     << ": " << st.temperature_c << " C" << std::endl
     << std::endl
     << std::endl;

  return os;
}
