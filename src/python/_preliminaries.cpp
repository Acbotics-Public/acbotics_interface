/*******************************************************************/
/*    NAME: Oscar Viquez                                           */
/*    ORG:  Acbotics Research, LLC                                 */
/*    FILE: _acsense_types.cpp                                      */
/*    DATE: Feb 25th 2025                                          */
/*                                                                 */
/*    For help, contact us at: support@acbotics.com                */
/*******************************************************************/

#include <glog/logging.h>
#include <pybind11/pybind11.h>

#include "utils/InterfaceHelper.h"

namespace py = pybind11;

void _preliminaries(py::module_ &m) {

#ifndef SKIP_GLOG_INIT_CHECK
  if (!google::IsGoogleLoggingInitialized()) {
    google::InitGoogleLogging("acbotics_interface");
    FLAGS_alsologtostderr = 1;
  }
#else
  google::InitGoogleLogging("acbotics_interface");
#endif

  m.doc() = "Acbotics AcSense Interface";

  m.def("init_logger", []() {
#ifndef SKIP_GLOG_INIT_CHECK
    if (!google::IsGoogleLoggingInitialized()) {
      google::InitGoogleLogging("acbotics_interface");
    }
#endif
    FLAGS_alsologtostderr = 1;
  });

  m.def("set_verbose", [](int v_level) { FLAGS_v = v_level; });

  m.def("debug_interface_helper", []() { FLAGS_debug_interface_helper = true; });
  m.def("debug_socket_in", []() { FLAGS_debug_socket_in = true; });
  m.def("debug_udp_data", []() { FLAGS_debug_udp_data = true; });
  m.def("debug_fft", []() { FLAGS_debug_fft = true; });
}
