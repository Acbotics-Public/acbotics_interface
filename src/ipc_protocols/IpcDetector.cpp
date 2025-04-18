/*******************************************************************/
/*    NAME: Oscar Viquez                                           */
/*    ORG:  Acbotics Research, LLC                                 */
/*    FILE: IpcDetector.cpp                                        */
/*    DATE: Feb 17th 2025                                          */
/*                                                                 */
/*    For help, contact us at: support@acbotics.com                */
/*******************************************************************/

#include <byteswap.h>
#include <cstring>
#include <glog/logging.h>
#include <iomanip>

// includes from within project
#include "IpcDetector.h"

IpcDetector::IpcDetector() {}

std::ostream &operator<<(std::ostream &os, const IpcDetector &st) {
  std::ostringstream oss;
  oss << st;
  os << oss.str();
  return os;
}
std::ostringstream &operator<<(std::ostringstream &os, const IpcDetector &st) {

  int width = 25;

  os << "AcSense IPC protocol : Energy Detector Data" << std::endl << st.header << std::endl;
  os << std::endl;

  // Don't use the full Payload operator<< -- use summary for a general packet view
  os << "Detector Raw Data Payload (Summary):" << std::endl;

  os << std::left << std::setw(width) << std::setfill('.') << "DETECTIONS"
     << ": " << st.detections << std::endl
     << std::endl
     << std::endl;

  return os;
}
