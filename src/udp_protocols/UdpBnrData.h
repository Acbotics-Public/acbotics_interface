/*******************************************************************/
/*    NAME: Oscar Viquez                                           */
/*    ORG:  Acbotics Research, LLC                                 */
/*    FILE: UdpImuData.h                                           */
/*    DATE: Apr 26th 2024                                          */
/*                                                                 */
/*    For help, contact us at: support@acbotics.com                */
/*******************************************************************/

#ifndef udp_bnr_data_HEADER
#define udp_bnr_data_HEADER

#include "UdpData.h"

struct UdpBnrData : UdpData {

  uint8_t status =0;
  float quat_i = 0;
  float quat_j = 0;
  float quat_k = 0;
  float quat_r = 0;
  float accuracy = 0; 

  UdpBnrData() {};
  UdpBnrData(std::vector<int8_t> &buff);
  bool unpack_data(std::vector<int8_t> &buff);
  void csv_header(std::ostream &oss);
  void csv_serialize(std::ostream &oss);
};

std::ostream &operator<<(std::ostream &os, const UdpBnrData &st);
std::ostringstream &operator<<(std::ostringstream &os, const UdpBnrData &st);

#endif
