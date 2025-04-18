/*******************************************************************/
/*    NAME: Oscar Viquez                                           */
/*    ORG:  Acbotics Research, LLC                                 */
/*    FILE: EnergyDetector.h                                       */
/*    DATE: Feb 14th 2025                                          */
/*                                                                 */
/*    For help, contact us at: support@acbotics.com                */
/*******************************************************************/

#ifndef detector_HEADER
#define detector_HEADER

#include <pthread.h>

// includes from within project
#include "utils/FreqDomainBase.h"

DECLARE_bool(debug_energy_detector);

class EnergyDetector : virtual public FreqDomainBase {
public:
  std::vector<ptr_tsQ<IpcDetector>> v_q_detect;

  EnergyDetector() : FreqDomainBase() {
    this->thread_name = "detector_thr";
    this->phone_sensitivity_V_uPa = -167;
  };

  void run();
  void register_client(QueueClient &client);

protected:
  double phone_sensitivity_V_uPa;

  static void *_run_detector_thread(void *arg);
};

#endif
