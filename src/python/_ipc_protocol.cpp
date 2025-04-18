/*******************************************************************/
/*    NAME: Oscar Viquez                                           */
/*    ORG:  Acbotics Research, LLC                                 */
/*    FILE: _ipc_protocol.cpp                                      */
/*    DATE: Feb 25th 2025                                          */
/*                                                                 */
/*    For help, contact us at: support@acbotics.com                */
/*******************************************************************/

#include <sstream>

#include <pybind11/eigen.h>
#include <pybind11/numpy.h>
#include <pybind11/pybind11.h>
#include <pybind11/stl.h>

#include "ipc_protocols/IpcData.h"

#include "ipc_protocols/IpcBnoState.h"
#include "ipc_protocols/IpcDetector.h"
#include "ipc_protocols/IpcFFT.h"

namespace py = pybind11;

void _ipc_protocol(py::module_ &m) {

  py::class_<IpcData::Header>(m, "IpcData_Header")
      .def("__repr__",
           [](const IpcData::Header &st) {
             std::ostringstream oss;
             oss << st;
             return oss.str();
           })
      .def_readonly("start_time_nsec", &IpcData::Header::start_time_nsec)
      .def_readonly("packet_num", &IpcData::Header::packet_num);

  py::class_<IpcData>(m, "IpcData")
      .def("__repr__",
           [](const IpcData &st) {
             std::ostringstream oss;
             oss << st;
             return oss.str();
           })
      .def_readonly("header", &IpcData::header);

  py::class_<IpcFFT, IpcData>(m, "IpcFFT")
      .def("__repr__",
           [](const IpcFFT &st) {
             std::ostringstream oss;
             oss << st;
             return oss.str();
           })
      .def_readonly("fft", &IpcFFT::fft);

  py::class_<IpcDetector, IpcData>(m, "IpcDetector")
      .def("__repr__",
           [](const IpcDetector &st) {
             std::ostringstream oss;
             oss << st;
             return oss.str();
           })
      .def_readonly("detections", &IpcDetector::detections);

  py::class_<IpcBnoState, IpcData>(m, "IpcBnoState")
      .def("__repr__",
           [](const IpcBnoState &st) {
             std::ostringstream oss;
             oss << st;
             return oss.str();
           })
      .def_readonly("accel_x", &IpcBnoState::accel_x)
      .def_readonly("accel_y", &IpcBnoState::accel_y)
      .def_readonly("accel_z", &IpcBnoState::accel_z)
      .def_readonly("gyro_x", &IpcBnoState::gyro_x)
      .def_readonly("gyro_y", &IpcBnoState::gyro_y)
      .def_readonly("gyro_z", &IpcBnoState::gyro_z)
      .def_readonly("mag_x", &IpcBnoState::mag_x)
      .def_readonly("mag_y", &IpcBnoState::mag_y)
      .def_readonly("mag_z", &IpcBnoState::mag_z)
      .def_readonly("accuracy", &IpcBnoState::accuracy)
      .def_readonly("quat_i", &IpcBnoState::quat_i)
      .def_readonly("quat_j", &IpcBnoState::quat_j)
      .def_readonly("quat_k", &IpcBnoState::quat_k)
      .def_readonly("quat_r", &IpcBnoState::quat_r);
}
