/*******************************************************************/
/*    NAME: Oscar Viquez                                           */
/*    ORG:  Acbotics Research, LLC                                 */
/*    FILE: UdpRtcData.cpp                                         */
/*    DATE: Nov 22nd 2024                                          */
/*                                                                 */
/*    For help, contact us at: support@acbotics.com                */
/*******************************************************************/

#include <byteswap.h>
#include <cstring>
#include <glog/logging.h>
#include <iomanip>

// includes from within project
#include "UdpRtcData.h"

UdpRtcData::UdpRtcData(std::vector<int8_t> &buff) {
  std::string buff_start(buff.begin(), buff.end());
  buff_start = buff_start.substr(0, 6);

  if (buff[8] == 'R' && buff[9] == 'T' && buff.size() >= sizeof(Header)) {
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

bool UdpRtcData::unpack_data(std::vector<int8_t> &buff) {
  size_t offset = sizeof(Header);

  uint8_t second = (buff[0 + offset] & 0xF) + (((buff[0 + offset] & 0xF0) >> 4) * 10);
  uint8_t minute = (buff[1 + offset] & 0xF) + (((buff[1 + offset] & 0xF0) >> 4) * 10);
  uint8_t hour = (buff[2 + offset] & 0xF) + (((buff[2 + offset] & 0xF0) >> 4) * 10);
  uint8_t day = (buff[4 + offset] & 0xF) + (((buff[4 + offset] & 0x30) >> 4) * 10);
  uint8_t month = (buff[5 + offset] & 0xF) + (((buff[5 + offset] & 0x10) >> 4) * 10);
  uint8_t year = (buff[6 + offset] & 0xF) + (((buff[6 + offset] & 0xF0) >> 4) * 10);

  std::tm time;
  time.tm_sec = second;
  time.tm_min = minute;
  time.tm_hour = hour;
  time.tm_mon = month - 1; // tm uses jan=0
  time.tm_mday = day;
  time.tm_year = 100 + year;

  // <ctime> gives local time via std::time(nullptr) ;
  // get adjustment for time zone correction:
  std::time_t ts_now = std::time(nullptr);
  std::tm tm_now = *std::gmtime(&ts_now);
  std::time_t tzoffset = ts_now - std::mktime(&tm_now);

  this->rtc_time = mktime(&time) + tzoffset;

  return true;
}

void UdpRtcData::csv_header(std::ostream &oss) {
  oss << "host_epoch_sec,data_epoch_nsec,rtc_time" << std::endl;
}

void UdpRtcData::csv_serialize(std::ostream &oss) {
  oss << std::time(nullptr) << "," << std::fixed << this->header.start_time_nsec << ","
      << this->rtc_time << std::endl;
}

std::ostream &operator<<(std::ostream &os, const UdpRtcData &st) {
  std::ostringstream oss;
  oss << st;
  os << oss.str();
  return os;
}
std::ostringstream &operator<<(std::ostringstream &os, const UdpRtcData &st) {
  os << "AcSense UDP protocol : RTC Data" << std::endl << st.header << std::endl;
  os << std::endl;

  int width = 20;

  std::tm *timestamp_tm = std::gmtime(&st.rtc_time);
  auto timestamp_str = std::put_time(timestamp_tm, "%Y-%m-%d %H:%M:%S %Z");

  os << "RTC Payload:" << std::endl
     << std::left << std::setw(width) << std::setfill('.') << "TIMESTAMP"
     << ": " << st.rtc_time << std::endl
     << std::left << std::setw(width) << std::setfill('.') << "" << ": " << timestamp_str
     << std::endl
     << std::endl
     << std::endl;

  return os;
}
