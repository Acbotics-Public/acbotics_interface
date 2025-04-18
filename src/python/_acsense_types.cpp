/*******************************************************************/
/*    NAME: Oscar Viquez                                           */
/*    ORG:  Acbotics Research, LLC                                 */
/*    FILE: _acsense_types.cpp                                      */
/*    DATE: Feb 25th 2025                                          */
/*                                                                 */
/*    For help, contact us at: support@acbotics.com                */
/*******************************************************************/

// #include <pybind11/eigen.h>
// #include <pybind11/numpy.h>
#include <pybind11/pybind11.h>
// #include <pybind11/stl.h>

#include "utils/Types.h"

namespace py = pybind11;

void _acsense_types(py::module_ &m) {

  py::enum_<LOGGER>(m, "LOGGER")
      .value("UNKNOWN", LOGGER::UNKNOWN)
      .value("ACO_CSV", LOGGER::ACO_CSV)
      .value("ACO_FLAC", LOGGER::ACO_FLAC)
      .value("ACO_WAV", LOGGER::ACO_WAV)
      .value("GPS", LOGGER::GPS)
      .value("PTS", LOGGER::PTS)
      .value("IMU", LOGGER::IMU)
      .value("EPT", LOGGER::EPT)
      .value("RTC", LOGGER::RTC)
      .value("BNO", LOGGER::BNO)
      .value("BNR", LOGGER::BNR);

  py::enum_<QUEUE>(m, "QUEUE")
      .value("UNKNOWN", QUEUE::UNKNOWN)
      .value("ACO", QUEUE::ACO)
      .value("FFT", QUEUE::FFT)
      .value("CBF", QUEUE::CBF)
      .value("DETECT", QUEUE::DETECT)

      .value("GPS", QUEUE::GPS)
      .value("PTS", QUEUE::PTS)
      .value("IMU", QUEUE::IMU)
      .value("EPT", QUEUE::EPT)
      .value("RTC", QUEUE::RTC)
      .value("BNO", QUEUE::BNO)
      .value("BNR", QUEUE::BNR);
}
