/*******************************************************************/
/*    NAME: Oscar Viquez                                           */
/*    ORG:  Acbotics Research, LLC                                 */
/*    FILE: _udp_protocol.cpp                                      */
/*    DATE: Feb 25th 2025                                          */
/*                                                                 */
/*    For help, contact us at: support@acbotics.com                */
/*******************************************************************/

#include <sstream>

#include <pybind11/eigen.h>
#include <pybind11/numpy.h>
#include <pybind11/pybind11.h>
#include <pybind11/stl.h>

#include "udp_protocols/UdpData.h"

#include "udp_protocols/UdpAcousticData.h"
#include "udp_protocols/UdpBeamform2D.h"
#include "udp_protocols/UdpBeamformRaw.h"
#include "udp_protocols/UdpBnoData.h"
#include "udp_protocols/UdpBnrData.h"
#include "udp_protocols/UdpEptData.h"
#include "udp_protocols/UdpImuData.h"
#include "udp_protocols/UdpPtsData.h"
#include "udp_protocols/UdpRtcData.h"

namespace py = pybind11;

void _udp_protocol(py::module_ &m) {

  py::class_<UdpData>(m, "UdpData")
      .def("__repr__",
           [](const UdpData &hh) {
             std::ostringstream oss;
             oss << hh;
             return oss.str();
           })
      .def_readonly("header", &UdpData::header);

  py::class_<UdpData::Header>(m, "UdpData_Header")
      .def("__repr__",
           [](const UdpData::Header &hh) {
             std::ostringstream oss;
             oss << hh;
             return oss.str();
           })
      .def_readonly("start_time_nsec", &UdpData::Header::start_time_nsec)
      .def_readonly("id", &UdpData::Header::id)
      .def_readonly("num_bytes", &UdpData::Header::num_bytes);

  py::class_<UdpAcousticData>(m, "UdpAcousticData")
      .def("__repr__",
           [](const UdpAcousticData &hh) {
             std::ostringstream oss;
             oss << hh;
             return oss.str();
           })
      .def_readonly("header", &UdpAcousticData::header);

  py::class_<UdpAcousticData::Header>(m, "UdpAcousticData_Header")
      .def("__repr__",
           [](const UdpAcousticData::Header &hh) {
             std::ostringstream oss;
             oss << hh;
             return oss.str();
           })
      .def_readonly("num_channels", &UdpAcousticData::Header ::num_channels)
      .def_readonly("sample_rate", &UdpAcousticData::Header ::sample_rate)
      .def_readonly("tick_time_nsec", &UdpAcousticData::Header::tick_time_nsec)

      // FIXME : remove getters get_num_channels(), get_sample_rate()
      .def("get_num_channels", [](const UdpAcousticData::Header &hh) { return hh.num_channels; })
      .def("get_sample_rate", [](const UdpAcousticData::Header &hh) { return hh.sample_rate; })

      .def("get_version", [](const UdpAcousticData::Header &hh) {
        std::ostringstream os;
        os << (int)hh.ver_maj << "." << (int)hh.ver_min;
        return os.str();
      });

  py::class_<UdpPtsData>(m, "UdpPtsData")
      .def("__repr__",
           [](const UdpPtsData &st) {
             std::ostringstream oss;
             oss << st;
             return oss.str();
           })
      .def_readonly("header", &UdpPtsData::header)
      .def_readonly("pressure_mbar", &UdpPtsData::pressure_mbar)
      .def_readonly("temperature_c", &UdpPtsData::temperature_c);

  py::class_<UdpImuData>(m, "UdpImuData")
      .def("__repr__",
           [](const UdpImuData &st) {
             std::ostringstream oss;
             oss << st;
             return oss.str();
           })
      .def_readonly("header", &UdpImuData::header)
      .def_readonly("pitch_ned_deg", &UdpImuData::pitch_ned_deg)
      .def_readonly("roll_ned_deg", &UdpImuData::roll_ned_deg)
      .def_readonly("accel_x", &UdpImuData::accel_x)
      .def_readonly("accel_y", &UdpImuData::accel_y)
      .def_readonly("accel_z", &UdpImuData::accel_z)
      .def_readonly("gyro_x", &UdpImuData::gyro_x)
      .def_readonly("gyro_y", &UdpImuData::gyro_y)
      .def_readonly("gyro_z", &UdpImuData::gyro_z);

  py::class_<UdpBnoData>(m, "UdpBnoData")
      .def("__repr__",
           [](const UdpBnoData &st) {
             std::ostringstream oss;
             oss << st;
             return oss.str();
           })
      .def_readonly("header", &UdpBnoData::header)
      .def_readonly("sense_type", &UdpBnoData::sense_type)
      .def_readonly("status", &UdpBnoData::status)
      .def_readonly("sense_x", &UdpBnoData::sense_x)
      .def_readonly("sense_y", &UdpBnoData::sense_y)
      .def_readonly("sense_z", &UdpBnoData::sense_z);

  py::class_<UdpBnrData>(m, "UdpBnrData")
      .def("__repr__",
           [](const UdpBnrData &st) {
             std::ostringstream oss;
             oss << st;
             return oss.str();
           })
      .def_readonly("header", &UdpBnrData::header)
      .def_readonly("status", &UdpBnrData::status)
      .def_readonly("accuracy", &UdpBnrData::accuracy)
      .def_readonly("quat_i", &UdpBnrData::quat_i)
      .def_readonly("quat_j", &UdpBnrData::quat_j)
      .def_readonly("quat_k", &UdpBnrData::quat_k)
      .def_readonly("quat_r", &UdpBnrData::quat_r);

  py::class_<UdpEptData>(m, "UdpEptData")
      .def("__repr__",
           [](const UdpEptData &st) {
             std::ostringstream oss;
             oss << st;
             return oss.str();
           })
      .def_readonly("header", &UdpEptData::header)
      .def_readonly("pressure_mbar", &UdpEptData::pressure_mbar)
      .def_readonly("temperature_c", &UdpEptData::temperature_c);

  py::class_<UdpRtcData>(m, "UdpRtcData")
      .def("__repr__",
           [](const UdpRtcData &st) {
             std::ostringstream oss;
             oss << st;
             return oss.str();
           })
      .def_readonly("header", &UdpRtcData::header)
      .def_readonly("rtc_time", &UdpRtcData::rtc_time);
}
