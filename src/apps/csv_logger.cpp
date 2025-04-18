/*******************************************************************/
/*    NAME: Oscar Viquez                                           */
/*    ORG:  Acbotics Research, LLC                                 */
/*    FILE: csv_logger.cpp                                         */
/*    DATE: Apr 4th 2024                                           */
/*                                                                 */
/*    For help, contact us at: support@acbotics.com                */
/*******************************************************************/

#include <gflags/gflags.h>
#include <glog/logging.h>

#include "utils/InterfaceHelper.h"

DEFINE_bool(use_mcast, false, "Enable UDP Multicast");
DEFINE_string(mcast_group, "224.1.1.1", "UDP Multicast Group");
DEFINE_string(iface_ip, "127.0.0.1", "Local network interface IP");
DEFINE_int32(port, 9760, "Target port for UDP traffic");

DEFINE_string(outdir, "/tmp/", "Target directory for output log files");

int main(int argc, char *argv[]) {
  // Initialize Google’s logging library.
  google::InitGoogleLogging(argv[0]);
  FLAGS_alsologtostderr = 1;

  gflags::SetUsageMessage("Usage:");
  gflags::ParseCommandLineFlags(&argc, &argv, true);

  LOG(INFO) << "Log verbosity level set to " << FLAGS_v;

  InterfaceHelper ss_helper;

  ss_helper.add_socket(FLAGS_use_mcast, FLAGS_iface_ip, FLAGS_port, FLAGS_mcast_group);

  if (FLAGS_outdir.back() != '/')
    FLAGS_outdir += '/';
  ss_helper.set_outdir(FLAGS_outdir);

  ss_helper.enable_logger(LOGGER::ACO_CSV, true);

  LOG(INFO) << "Printing socketIn info" << std::endl << ss_helper.sockets.at(0);

  ss_helper.run_threads();

  UdpPtsData pts;
  bool _new_data = false;
  while (true) {
    sleep(1);
  }

  return 0;
}
