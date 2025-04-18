/*******************************************************************/
/*    NAME: Oscar Viquez                                           */
/*    ORG:  Acbotics Research, LLC                                 */
/*    FILE: run_tests.cpp                                          */
/*    DATE: Apr 4th 2024                                           */
/*                                                                 */
/*    For help, contact us at: support@acbotics.com                */
/*******************************************************************/

#include <gflags/gflags.h>
#include <glog/logging.h>

#include "tests.h"
// #include "utils/Logger.h"
#include "utils/UdpSocketIn.h"

DEFINE_bool(use_mcast, false, "Enable UDP Multicast");
DEFINE_string(mcast_group, "224.1.1.1", "UDP Multicast Group");
DEFINE_string(iface_ip, "127.0.0.1", "Local network interface IP");
DEFINE_int32(port, 9760, "Target port for UDP traffic");

DEFINE_string(test_data_dir, "/tmp/", "Path to test data files");

int main(int argc, char *argv[]) {
  // Initialize Google’s logging library.
  google::InitGoogleLogging(argv[0]);
  FLAGS_alsologtostderr = 1;

  gflags::SetUsageMessage("Usage:");
  gflags::ParseCommandLineFlags(&argc, &argv, true);

  LOG(INFO) << "Log level set to " << FLAGS_v;

  UdpSocketIn socket_args_def;
  LOG(INFO) << "Printing default socket info" << std::endl << socket_args_def;

  UdpSocketIn socket_args;
  socket_args.use_mcast = FLAGS_use_mcast;
  socket_args.iface_ip = FLAGS_iface_ip;
  socket_args.port = FLAGS_port;
  socket_args.mcast_group = FLAGS_mcast_group;

  LOG(INFO) << "Printing socket info" << std::endl << socket_args;

  LOG(INFO) << "Running basic tests";
  run_aco_tests(FLAGS_test_data_dir, "sample_raw_acoustic_packet.dat");
  run_bf_raw_tests(FLAGS_test_data_dir, "sample_beamformer_raw_packet.dat");
  run_bf_2d_tests(FLAGS_test_data_dir, "sample_beamformer_2d_packet.dat");
  run_pts_tests();

  return 0;
}
