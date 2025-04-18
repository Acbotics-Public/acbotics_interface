/*******************************************************************/
/*    NAME: Oscar Viquez                                           */
/*    ORG:  Acbotics Research, LLC                                 */
/*    FILE: IpcFFT.h                                               */
/*    DATE: Feb 17th 2025                                          */
/*                                                                 */
/*    For help, contact us at: support@acbotics.com                */
/*******************************************************************/

#ifndef ipc_fft_HEADER
#define ipc_fft_HEADER

#include <Eigen/Dense>

#include "IpcData.h"

struct IpcFFT : virtual public IpcData {
  Eigen::MatrixXcd fft;

  IpcFFT();
  IpcFFT(Eigen::MatrixXcd &fft);
  IpcFFT(size_t rows, size_t cols);
};

std::ostream &operator<<(std::ostream &os, const IpcFFT &st);
std::ostringstream &operator<<(std::ostringstream &os, const IpcFFT &st);

#endif
