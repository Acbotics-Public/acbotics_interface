/*******************************************************************/
/*    NAME: Oscar Viquez                                           */
/*    ORG:  Acbotics Research, LLC                                 */
/*    FILE: UdpEptData.h                                           */
/*    DATE: Nov 22nd 2024                                          */
/*                                                                 */
/*    For help, contact us at: support@acbotics.com                */
/*******************************************************************/

#ifndef udp_ept_data_HEADER
#define udp_ept_data_HEADER

#include "UdpData.h"

struct UdpEptData : UdpData {

  float pressure_mbar = 0;
  float temperature_c = 0;
  int mbar_coeff = 100;

  UdpEptData() {};
  UdpEptData(std::vector<int8_t> &buff);
  bool unpack_data(std::vector<int8_t> &buff);
  void csv_header(std::ostream &oss);
  void csv_serialize(std::ostream &oss);
};

std::ostream &operator<<(std::ostream &os, const UdpEptData &st);
std::ostringstream &operator<<(std::ostringstream &os, const UdpEptData &st);

#endif
