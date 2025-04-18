/*******************************************************************/
/*    NAME: Oscar Viquez                                           */
/*    ORG:  Acbotics Research, LLC                                 */
/*    FILE: _acsense_types.cpp                                      */
/*    DATE: Feb 25th 2025                                          */
/*                                                                 */
/*    For help, contact us at: support@acbotics.com                */
/*******************************************************************/

#include <gflags/gflags.h>
#include <glog/logging.h>

#include <pybind11/eigen.h>
#include <pybind11/numpy.h>
#include <pybind11/pybind11.h>
#include <pybind11/stl.h>

#include <Eigen/Dense>
#include <variant>

// includes from within project
#include "utils/InterfaceHelper.h"
#include "utils/UdpSocketIn.h"

#include "ipc_protocols/IpcBnoState.h"

namespace py = pybind11;

void _utils(py::module_ &m) {

  py::class_<QueueClient>(m, "QueueClient")
      .def(py::init<>())
      .def("run", &QueueClient::run)
      .def("stop", &QueueClient::stop)
      .def("pop_aco", [](QueueClient &sst) { return *sst.q_aco->pop(); })
      .def("pop_fft", [](QueueClient &sst) { return *sst.q_fft->pop(); })
      .def("pop_beam2d", [](QueueClient &sst) { return *sst.q_beam2d->pop(); })
      .def("pop_detector", [](QueueClient &sst) { return *sst.q_detect->pop(); })
      // .def("pop_detector_all", [](QueueClient &sst) { return sst.q_detect->pop_all(); })
      .def("size", [](QueueClient &sst, QUEUE q) {
        switch (q) {
        case QUEUE::ACO:
          return sst.q_aco->size();
          break;
        case QUEUE::FFT:
          return sst.q_fft->size();
          break;
        case QUEUE::CBF:
          return sst.q_beam2d->size();
          break;
        case QUEUE::DETECT:
          return sst.q_detect->size();
          break;

        default:
          return (size_t)0;
          break;
        }
      });

  py::class_<UdpSocketIn>(m, "UdpSocketIn")
      .def(py::init<>())
      // .def(py::init<bool, std::string, std::string>())
      .def(py::init<bool, std::string, int32_t, std::string>(), py::arg("use_mcast"),
           py::arg("iface_ip"), py::arg("port"), py::arg("mcast_group"))
      .def("__repr__",
           [](const UdpSocketIn &st) {
             std::ostringstream oss;
             oss << st;
             return oss.str();
           })
      .def_readwrite("use_mcast", &UdpSocketIn::use_mcast)
      .def_readwrite("port", &UdpSocketIn::port)
      .def_readwrite("iface_ip", &UdpSocketIn::iface_ip)
      .def_readwrite("mcast_group", &UdpSocketIn::mcast_group)
      .def("run_socket_main_thread",
           static_cast<void (UdpSocketIn::*)()>(&UdpSocketIn::run_socket_main_thread),
           "Run intake socket in main thread")
      .def("run_socket_thread", &UdpSocketIn::run_socket_thread,
           "Run intake socket in separate thread")
      // pybind11 requires explicit typing;
      // cannot just use the parent class as in pure C++
      // instead, the child classes must be declared as acceptable inputs
      // and the order of overload resolution should be accounted for
      .def(
          "register_client", [](UdpSocketIn &sst, FFT &cst) { sst.register_client(cst); },
          py::arg("client"), "Register client")
      .def(
          "register_client",
          [](UdpSocketIn &sst, InterfaceHelper &cst) { sst.register_client(cst); },
          py::arg("client"), "Register client")
      // Per overload resolution order, we register the generic QueueClient form as the last choice
      .def("register_client", &UdpSocketIn::register_client, py::arg("client"), "Register client");

  py::class_<FreqDomainBase, QueueClient>(m, "FreqDomainBase")
      .def(py::init<>())
      .def("set_sample_rate", &FreqDomainBase::set_sample_rate, py::arg("sample_rate"))
      .def("set_NFFT", &FreqDomainBase::set_NFFT, py::arg("NFFT"))
      .def("set_noverlap", &FreqDomainBase::set_noverlap, py::arg("noverlap"))

      .def("get_sample_rate", &FreqDomainBase::get_sample_rate)
      .def("get_NFFT", &FreqDomainBase::get_NFFT)
      .def("get_noverlap", &FreqDomainBase::get_noverlap)
      .def("get_nstep", &FreqDomainBase::get_nstep)

      .def("add_frequency_band_min_max", &FreqDomainBase::add_frequency_band_min_max)
      .def("add_frequency_band_center", &FreqDomainBase::add_frequency_band_center)
      .def("add_frequency_bin_closest", &FreqDomainBase::add_frequency_bin_closest)
      .def("clear_frequency_bands", &FreqDomainBase::clear_frequency_bands)
      .def("reset_frequency_band_min_max", &FreqDomainBase::reset_frequency_band_min_max)
      .def("reset_frequency_band_center", &FreqDomainBase::reset_frequency_band_center)

      .def(
          "add_acoustic_data",
          [](FreqDomainBase &sst, int64_t &start_time_nsec, Eigen::MatrixX<int16_t> &data) {
            std::shared_ptr<UdpAcousticData> aco_data = std::make_shared<UdpAcousticData>();
            aco_data->header.sample_rate = sst.get_sample_rate();
            aco_data->header.num_channels = data.rows();
            aco_data->header.num_values = data.size();
            aco_data->header.start_time_nsec = start_time_nsec;

            aco_data->data = data;

            sst.q_aco->push(aco_data);
          },
          py::arg("start_time_nsec"), py::arg("time_series"),
          "Add time-series data to acoustic queue");

  py::class_<FFT, FreqDomainBase, QueueClient>(m, "FFT")
      .def(py::init<>())
      .def(
          "register_client", [](FFT &sst, EnergyDetector &cst) { sst.register_client(cst); },
          py::arg("client"), "Register client")
      .def("register_client", &FFT::register_client, py::arg("client"), "Register client");

  py::class_<EnergyDetector, FreqDomainBase, QueueClient>(m, "EnergyDetector")
      .def(py::init<>())
      .def("register_client", &EnergyDetector::register_client, py::arg("client"),
           "Register client");

  py::class_<InterfaceHelper, QueueClient>(m, "InterfaceHelper")
      .def(py::init<>())
      .def_readwrite("fft", &InterfaceHelper::_fft_helper)
      .def("add_socket", &InterfaceHelper::add_socket, py::arg("use_mcast"), py::arg("iface_ip"),
           py::arg("port"), py::arg("mcast_group"))
      .def("run_threads", static_cast<void (InterfaceHelper::*)()>(&InterfaceHelper::run_threads),
           "Run intake socket in separate thread")

      .def("get_latest_pts", &InterfaceHelper::get_latest_data<UdpPtsData, LOGGER::PTS>)
      .def("get_latest_ept", &InterfaceHelper::get_latest_data<UdpEptData, LOGGER::EPT>)
      .def("get_latest_imu", &InterfaceHelper::get_latest_data<UdpImuData, LOGGER::IMU>)
      .def("get_latest_rtc", &InterfaceHelper::get_latest_data<UdpRtcData, LOGGER::RTC>)
      .def("get_latest_bno", &InterfaceHelper::get_latest_data<UdpBnoData, LOGGER::BNO>)
      .def("get_latest_bnr", &InterfaceHelper::get_latest_data<UdpBnrData, LOGGER::BNR>)

      .def("get_latest_bno_state", [](InterfaceHelper &sst) { return sst.latest_bno_state; })

      .def("get_acoustic_data",
           [](InterfaceHelper &sst) -> std::variant<Eigen::MatrixX<int16_t>, int> {
             sst.latest_time_nsec = sst.q_aco->back()->header.start_time_nsec;
             if (sst.has_data_aco()) {
               sst.q_request_aco->push(true);
               return sst.q_aco_out->pop();
             }
             return -1;
           })
      .def("get_fft_data_curr_ch",
           [](InterfaceHelper &sst) -> std::variant<Eigen::MatrixXd, py::none> {
             if (sst.has_data_fft()) {
               sst.q_request_fft->push(true);
               return sst.q_fft_out->pop();
             }
             return py::none();
           })
      .def("get_latest_timestamp",
           [](InterfaceHelper &sst) {
             size_t q_count;
             q_count = sst.q_aco->size();
             if (q_count > 0) {
               sst.latest_time_nsec = sst.q_aco->back()->header.start_time_nsec;
             }
             return sst.latest_time_nsec / 1e9;
           })
      .def("get_beamformer_data",
           [](InterfaceHelper &sst) -> std::variant<Eigen::MatrixXd, py::none> {
             if (sst.has_data_cbf()) {
               sst.q_request_cbf->push(true);
               return sst.q_cbf_out->pop();
             }
             return py::none();
           })
      .def("get_detection_data",
           [](InterfaceHelper &sst) {
             sst.q_request_detections->push(true);
             return sst.q_detections_out->pop();
           })
      .def("aco_size", [](InterfaceHelper &sst) { return sst.q_aco->size(); })
      .def("fft_size", [](InterfaceHelper &sst) { return sst.q_fft->size(); })
      .def("aco_front", [](InterfaceHelper &sst) { return sst.q_aco->front(); })
      .def("get_aco_header", [](InterfaceHelper &sst) { return sst.latest_aco_header; })
      .def("get_NFFT", [](InterfaceHelper &sst) { return sst._fft_helper.get_NFFT(); })
      .def("get_noverlap", [](InterfaceHelper &sst) { return sst._fft_helper.get_noverlap(); })
      .def("get_nstep", [](InterfaceHelper &sst) { return sst._fft_helper.get_nstep(); })
      .def("check_sockets", &InterfaceHelper::check_sockets)

      .def("set_outdir", &InterfaceHelper::set_outdir, py::arg("output_dir"))
      .def("set_curr_ch", &InterfaceHelper::set_curr_ch, py::arg("new_ch"))
      .def("set_rollover", &InterfaceHelper::set_rollover, py::arg("logger"),
           py::arg("rollover_min"))

      .def("enable_aco_fork", &InterfaceHelper::enable_aco_fork)

      .def("enable_buffer_all_ch", &InterfaceHelper::enable_buffer_all_ch)
      .def("enable_buffer_aco", &InterfaceHelper::enable_buffer_aco)
      .def("enable_buffer_fft", &InterfaceHelper::enable_buffer_fft)
      .def("enable_buffer_beamformer", &InterfaceHelper::enable_buffer_beamformer)
      .def("enable_detector", &InterfaceHelper::enable_detector)

      .def("enable_logger", &InterfaceHelper::enable_logger, py::arg("logger"), py::arg("enable"))
      .def("start_logging", &InterfaceHelper::start_logging, py::arg("logger"))
      .def("stop_logging", &InterfaceHelper::stop_logging, py::arg("logger"))

      .def("set_adc_scale", &InterfaceHelper::set_adc_scale, py::arg("adc_scale"))
      .def("set_phone_sensitivity_V_uPa", &InterfaceHelper::set_phone_sensitivity_V_uPa,
           py::arg("sensitivity_V_uPa"))
      .def("set_NFFT", &InterfaceHelper::set_NFFT, py::arg("NFFT"))
      .def("set_noverlap", &InterfaceHelper::set_noverlap, py::arg("noverlap"))
      .def("set_sample_rate", &InterfaceHelper::set_sample_rate, py::arg("sample_rate"))

      .def("set_channel_filter", py::overload_cast<int>(&InterfaceHelper::set_channel_filter),
           py::arg("num_ch"))
      .def("set_channel_filter",
           py::overload_cast<std::vector<int>>(&InterfaceHelper::set_channel_filter),
           py::arg("ch_filter"))

      .def("set_frequency_boundaries", &InterfaceHelper::set_frequency_boundaries,
           py::arg("freq_low"), py::arg("freq_high"))

      .def("get_adc_scale", &InterfaceHelper::get_adc_scale)
      .def("get_phone_sensitivity_V_uPa", &InterfaceHelper::get_phone_sensitivity_V_uPa);
}
