/*******************************************************************/
/*    NAME: Oscar Viquez                                           */
/*    ORG:  Acbotics Research, LLC                                 */
/*    FILE: IpcBnoState.h                                          */
/*    DATE: Feb 19th 2025                                          */
/*                                                                 */
/*    For help, contact us at: support@acbotics.com                */
/*******************************************************************/

#ifndef ipc_bno_state_HEADER
#define ipc_bno_state_HEADER

#include "IpcData.h"

struct IpcBnoState : virtual public IpcData {

  float accel_x;
  float accel_y;
  float accel_z;

  float gyro_x;
  float gyro_y;
  float gyro_z;

  float mag_x;
  float mag_y;
  float mag_z;

  float quat_i = 0;
  float quat_j = 0;
  float quat_k = 0;
  float quat_r = 0;
  float accuracy = 0;

  IpcBnoState();
};

std::ostream &operator<<(std::ostream &os, const IpcBnoState &st);
std::ostringstream &operator<<(std::ostringstream &os, const IpcBnoState &st);

#endif
