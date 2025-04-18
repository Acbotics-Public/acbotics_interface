/*******************************************************************/
/*    NAME: Oscar Viquez                                           */
/*    ORG:  Acbotics Research, LLC                                 */
/*    FILE: UdpImuData.h                                           */
/*    DATE: Apr 26th 2024                                          */
/*                                                                 */
/*    For help, contact us at: support@acbotics.com                */
/*******************************************************************/

#ifndef udp_bno_data_HEADER
#define udp_bno_data_HEADER

#include "UdpData.h"
#include <unordered_map>

enum class BNO_TYPE { UNKNOWN, ACCEL, GYRO, MAG };
inline std::unordered_map<char, BNO_TYPE> BNO_TYPE_CHAR{
    {'A', BNO_TYPE::ACCEL}, {'G', BNO_TYPE::GYRO}, {'M', BNO_TYPE::MAG}};

struct UdpBnoData : UdpData {

  BNO_TYPE sense_type = BNO_TYPE::UNKNOWN;
  char sense_char = 'U';
  uint8_t status = 0;
  float sense_x = 0;
  float sense_y = 0;
  float sense_z = 0;

  UdpBnoData() {};
  UdpBnoData(std::vector<int8_t> &buff);
  bool unpack_data(std::vector<int8_t> &buff);
  void csv_header(std::ostream &oss);
  void csv_serialize(std::ostream &oss);
};

std::ostream &operator<<(std::ostream &os, const UdpBnoData &st);
std::ostringstream &operator<<(std::ostringstream &os, const UdpBnoData &st);

#endif
