/*******************************************************************/
/*    NAME: Oscar Viquez                                           */
/*    ORG:  Acbotics Research, LLC                                 */
/*    FILE: UdpRtcData.h                                           */
/*    DATE: Nov 22th 2024                                          */
/*                                                                 */
/*    For help, contact us at: support@acbotics.com                */
/*******************************************************************/

#ifndef udp_rtc_data_HEADER
#define udp_rtc_data_HEADER

#include "UdpData.h"

struct UdpRtcData : UdpData {

  time_t rtc_time = 0;

  UdpRtcData() {};
  UdpRtcData(std::vector<int8_t> &buff);
  bool unpack_data(std::vector<int8_t> &buff);
  void csv_header(std::ostream &oss);
  void csv_serialize(std::ostream &oss);
};

std::ostream &operator<<(std::ostream &os, const UdpRtcData &st);
std::ostringstream &operator<<(std::ostringstream &os, const UdpRtcData &st);

#endif
