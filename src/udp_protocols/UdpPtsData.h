/*******************************************************************/
/*    NAME: Oscar Viquez                                           */
/*    ORG:  Acbotics Research, LLC                                 */
/*    FILE: UdpPtsData.h                                           */
/*    DATE: Apr 26th 2024                                          */
/*                                                                 */
/*    For help, contact us at: support@acbotics.com                */
/*******************************************************************/

#ifndef udp_pts_data_HEADER
#define udp_pts_data_HEADER

#include "UdpData.h"

struct UdpPtsData : UdpData {

  float pressure_mbar = 0;
  float temperature_c = 0;

  UdpPtsData() {};
  UdpPtsData(std::vector<int8_t> &buff);
  bool unpack_data(std::vector<int8_t> &buff);
  void csv_header(std::ostream &oss);
  void csv_serialize(std::ostream &oss);
};

std::ostream &operator<<(std::ostream &os, const UdpPtsData &st);
std::ostringstream &operator<<(std::ostringstream &os, const UdpPtsData &st);

#endif
