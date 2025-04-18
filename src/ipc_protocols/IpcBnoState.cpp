/*******************************************************************/
/*    NAME: Oscar Viquez                                           */
/*    ORG:  Acbotics Research, LLC                                 */
/*    FILE: IpcBnoState.cpp                                        */
/*    DATE: Feb 19th 2025                                          */
/*                                                                 */
/*    For help, contact us at: support@acbotics.com                */
/*******************************************************************/

#include <byteswap.h>
#include <cstring>
#include <glog/logging.h>
#include <iomanip>

// includes from within project
#include "IpcBnoState.h"

IpcBnoState::IpcBnoState() {}

std::ostream &operator<<(std::ostream &os, const IpcBnoState &st) {
  std::ostringstream oss;
  oss << st;
  os << oss.str();
  return os;
}
std::ostringstream &operator<<(std::ostringstream &os, const IpcBnoState &st) {

  int width = 25;

  os << "AcSense IPC protocol : BNO State Data" << std::endl << st.header << std::endl;
  os << std::endl;

  // Don't use the full Payload operator<< -- use summary for a general packet view
  os << "BNO State Data Payload (Summary):" << std::endl;

  os << "BNO ACCEL Payload:" << std::endl
     << std::left << std::setw(width) << std::setfill('.') << "ACCEL_X"
     << ": " << st.accel_x << std::endl
     << std::left << std::setw(width) << std::setfill('.') << "ACCEL_Y"
     << ": " << st.accel_y << std::endl
     << std::left << std::setw(width) << std::setfill('.') << "ACCEL_Z"
     << ": " << st.accel_z << std::endl
     << std::endl;

  os << "BNO GYRO Payload:" << std::endl
     << std::left << std::setw(width) << std::setfill('.') << "GYRO_X"
     << ": " << st.gyro_x << std::endl
     << std::left << std::setw(width) << std::setfill('.') << "GYRO_Y"
     << ": " << st.gyro_y << std::endl
     << std::left << std::setw(width) << std::setfill('.') << "GYRO_Z"
     << ": " << st.gyro_z << std::endl
     << std::endl;

  os << "BNO MAG Payload:" << std::endl
     << std::left << std::setw(width) << std::setfill('.') << "MAG_X"
     << ": " << st.mag_x << std::endl
     << std::left << std::setw(width) << std::setfill('.') << "MAG_Y"
     << ": " << st.mag_y << std::endl
     << std::left << std::setw(width) << std::setfill('.') << "MAG_Z"
     << ": " << st.mag_z << std::endl
     << std::endl;

  os << std::endl;

  return os;
}
