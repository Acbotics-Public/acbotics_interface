/*******************************************************************/
/*    NAME: Oscar Viquez                                           */
/*    ORG:  Acbotics Research, LLC                                 */
/*    FILE: UdpData.cpp                                            */
/*    DATE: Apr 26th 2024                                          */
/*                                                                 */
/*    For help, contact us at: support@acbotics.com                */
/*******************************************************************/

#include <byteswap.h>
#include <cstring>
#include <glog/logging.h>
#include <iomanip>

// includes from within project
#include "UdpData.h"

DEFINE_bool(debug_udp_data, false, "Enable expanded debug for UdpData");

UdpData::UdpData() {}

UdpData::Header::Header(std::vector<int8_t> &buff) { this->decode(buff); }

void UdpData::Header::decode(std::vector<int8_t> &buff) {
  auto buff_raw = buff.data();

  size_t offset;

  int64_t start_time_10nsec;
  int16_t num_bytes;

  offset = 0;
  std::memcpy(&start_time_10nsec, buff_raw + offset, sizeof(start_time_10nsec));
  offset += sizeof(start_time_10nsec);

  this->id[0] = buff[offset];
  this->id[1] = buff[offset + 1];

  offset += 2;

  std::memcpy(&num_bytes, buff_raw + offset, sizeof(num_bytes));
  offset += sizeof(num_bytes);

  this->start_time_nsec = start_time_10nsec * 10;
  this->num_bytes = num_bytes;
}

UdpData::UdpData(std::vector<int8_t> &buff) {
  std::string buff_start(buff.begin(), buff.end());
  buff_start = buff_start.substr(0, 6);

  if (buff.size() >= sizeof(Header)) {
    this->header = Header(buff);

  } else {
    log_invalid_buffer(buff_start);
  }
}

bool UdpData::unpack_data(std::vector<int8_t> &buff) {
  // not implemented since true generic payload is unknown;
  // return false to signal this is the case
  return false;
}

void UdpData::csv_header(std::ostream &oss) {
  oss << "host_epoch_sec,data_epoch_nsec,header_ID,header_numBytes" << std::endl;
}

void UdpData::csv_serialize(std::ostream &oss) {
  oss << std::time(nullptr) << "," << std::fixed << this->header.start_time_nsec << ","
      << this->header.id[0] << this->header.id[1] << "," << this->header.num_bytes << std::endl;
}

void UdpData::log_invalid_buffer(std::string &buff_start) {
  std::ostringstream os;
  for (auto cc : buff_start) {
    os << std::setfill('0') << std::setw(2) << std::hex << (int)cc;
  }
  LOG(WARNING) << "Ignoring invalid buffer : " << buff_start << " (" << os.str() << ")";
}

std::ostream &operator<<(std::ostream &os, const UdpData::Header &st) {
  std::ostringstream oss;
  oss << st;
  os << oss.str();
  return os;
}
std::ostringstream &operator<<(std::ostringstream &os, const UdpData::Header &st) {
  int width = 20;

  time_t start_time_sec = st.start_time_nsec / 1e9;

  struct tm *start_time_tm = std::gmtime(&start_time_sec);
  auto start_time_str = std::put_time(start_time_tm, "%Y-%m-%d %H:%M:%S %Z");

  os << "Generic UDP Data Header:" << std::endl
     << std::left << std::setw(width) << std::setfill('.') << "START_TIME" << ": "
     << st.start_time_nsec << " nsec" << std::endl
     << std::left << std::setw(width) << std::setfill('.') << "" << ": " << start_time_str
     << std::endl
     << std::left << std::setw(width) << std::setfill('.') << "ID" << ": " << st.id[0] << st.id[1]
     << std::endl
     << std::left << std::setw(width) << std::setfill('.') << "NUM_BYTES" << ": " << st.num_bytes
     << std::endl;

  return os;
}

std::ostream &operator<<(std::ostream &os, const UdpData &st) {
  std::ostringstream oss;
  oss << st;
  os << oss.str();
  return os;
}
std::ostringstream &operator<<(std::ostringstream &os, const UdpData &st) {
  os << "AcSense UDP protocol : Generic Data" << std::endl << st.header << std::endl;
  os << std::endl;

  return os;
}
