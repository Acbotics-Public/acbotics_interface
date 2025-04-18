/*******************************************************************/
/*    NAME: Oscar Viquez                                           */
/*    ORG:  Acbotics Research, LLC                                 */
/*    FILE: UdpImuData.h                                           */
/*    DATE: Apr 26th 2024                                          */
/*                                                                 */
/*    For help, contact us at: support@acbotics.com                */
/*******************************************************************/

#ifndef udp_imu_data_HEADER
#define udp_imu_data_HEADER

#include "UdpData.h"

struct UdpImuData : UdpData {

  //   float pressure_mbar;
  //   float temperature_c;
  float pitch_ned_deg = 0;
  float roll_ned_deg = 0;
  int16_t accel_x = 0;
  int16_t accel_y = 0;
  int16_t accel_z = 0;
  int16_t gyro_x = 0;
  int16_t gyro_y = 0;
  int16_t gyro_z = 0;

  UdpImuData() {};
  UdpImuData(std::vector<int8_t> &buff);
  bool unpack_data(std::vector<int8_t> &buff);
  void csv_header(std::ostream &oss);
  void csv_serialize(std::ostream &oss);
};

std::ostream &operator<<(std::ostream &os, const UdpImuData &st);
std::ostringstream &operator<<(std::ostringstream &os, const UdpImuData &st);

#endif
