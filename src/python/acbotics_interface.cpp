/*******************************************************************/
/*    NAME: Oscar Viquez                                           */
/*    ORG:  Acbotics Research, LLC                                 */
/*    FILE: acbotics_bindings.cpp                                  */
/*    DATE: Apr 4th 2024                                           */
/*                                                                 */
/*    For help, contact us at: support@acbotics.com                */
/*******************************************************************/

#include <pybind11/pybind11.h>

namespace py = pybind11;

void _preliminaries(py::module_ &m);
void _acsense_types(py::module_ &m);
void _udp_protocol(py::module_ &m);
void _ipc_protocol(py::module_ &m);
void _utils(py::module_ &m);

PYBIND11_MODULE(acbotics_interface, m) {

  // Preliminaries : Logging & Debug
  // ===============================
  _preliminaries(m);

  // Data Container / Protocol Types
  // ===============================
  _acsense_types(m);
  _udp_protocol(m);
  _ipc_protocol(m);

  // Implementation Types
  // ====================
  _utils(m);

  m.attr("__version__") = BUILD_TAG;
}
