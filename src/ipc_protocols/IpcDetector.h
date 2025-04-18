/*******************************************************************/
/*    NAME: Oscar Viquez                                           */
/*    ORG:  Acbotics Research, LLC                                 */
/*    FILE: IpcDetector.h                                          */
/*    DATE: Feb 17th 2025                                          */
/*                                                                 */
/*    For help, contact us at: support@acbotics.com                */
/*******************************************************************/

#ifndef ipc_detector_HEADER
#define ipc_detector_HEADER

#include "IpcData.h"

struct IpcDetector : virtual public IpcData {
  int detections;

  IpcDetector();
};

std::ostream &operator<<(std::ostream &os, const IpcDetector &st);
std::ostringstream &operator<<(std::ostringstream &os, const IpcDetector &st);

#endif
