/*******************************************************************/
/*    NAME: Oscar Viquez                                           */
/*    ORG:  Acbotics Research, LLC                                 */
/*    FILE: IpcFFT.cpp                                             */
/*    DATE: Feb 17th 2025                                          */
/*                                                                 */
/*    For help, contact us at: support@acbotics.com                */
/*******************************************************************/

#include <byteswap.h>
#include <cstring>
#include <glog/logging.h>
#include <iomanip>

// includes from within project
#include "IpcFFT.h"

IpcFFT::IpcFFT() {}
IpcFFT::IpcFFT(Eigen::MatrixXcd &fft) { this->fft = fft; }
IpcFFT::IpcFFT(size_t rows, size_t cols) { this->fft = Eigen::MatrixXcd(rows, cols); }

std::ostream &operator<<(std::ostream &os, const IpcFFT &st) {
  std::ostringstream oss;
  oss << st;
  os << oss.str();
  return os;
}
std::ostringstream &operator<<(std::ostringstream &os, const IpcFFT &st) {

  int width = 25;

  os << "AcSense IPC protocol : FFT Data" << std::endl << st.header << std::endl;
  os << std::endl;

  // Don't use the full Payload operator<< -- use summary for a general packet view
  os << "FFT Raw Data Payload (Summary):" << std::endl;

  os << std::left << std::setw(width) << std::setfill('.') << "FFT"
     << ": (" << st.fft.rows() << " x " << st.fft.cols() << ")" << std::endl
     << std::endl
     << std::endl;

  return os;
}
