/*******************************************************************/
/*    NAME: Oscar Viquez                                           */
/*    ORG:  Acbotics Research, LLC                                 */
/*    FILE: tests.cpp                                              */
/*    DATE: Apr 4th 2024                                           */
/*                                                                 */
/*    For help, contact us at: support@acbotics.com                */
/*******************************************************************/

#include <algorithm>
#include <arpa/inet.h>
#include <cstring>
#include <gflags/gflags.h>
#include <glog/logging.h>
#include <sys/socket.h>
#include <vector>

#include "tests.h"
#include "udp_protocols/UdpAcousticData.h"
#include "udp_protocols/UdpBeamform2D.h"
#include "udp_protocols/UdpBeamformRaw.h"
#include "udp_protocols/UdpData.h"

#include <Eigen/Dense>

void run_aco_tests(std::string test_file_dir, std::string test_file_name) {
  // Test UdpAcousticData
  LOG(INFO) << "Checking Acoustic Data protocol";

  // Empty test Aco
  LOG(INFO) << "Running empty test...";
  UdpAcousticData test;
  LOG(INFO) << "Result for empty test: " << std::endl << test;

  // Dummy test Aco
  LOG(INFO) << "Running invalid packet test...";
  std::string sample = "12er,gi";
  std::vector<int8_t> buff(sample.begin(), sample.end());
  UdpAcousticData test2(buff);

  LOG(INFO) << "Result for invalid packet test  " << std::endl << test2;

  // Real frame test Aco
  // > Load sample binary data from file
  LOG(INFO) << "Running valid data sample (DAT) test...";
  FILE *datfil;
  datfil = fopen((test_file_dir + test_file_name).c_str(), "rb");
  fseek(datfil, 0, SEEK_END);
  size_t n_chars = ftell(datfil);
  fseek(datfil, 0, SEEK_SET);

  LOG(INFO) << "Sample file is " << n_chars << " long";

  buff.clear();

  int8_t val;
  bool is_eof = false;
  for (size_t ii = 0; ii < n_chars; ii++) {
    val = fgetc(datfil);
    buff.push_back(val);
  }

  fclose(datfil);

  UdpAcousticData test3(buff);

  LOG(INFO) << "Result for valid data sample (DAT) test " << std::endl << test3;

  /* TEST EIGEN*/

  int cols;
  std::string trm;
  std::ostringstream os;

  Eigen::MatrixX<int16_t> mat(test3.header.num_channels,
                              test3.header.num_values / test3.header.num_channels);
  mat = Eigen::Map<Eigen::MatrixX<int16_t>>(
      reinterpret_cast<int16_t *>(buff.data() + sizeof(UdpAcousticData::Header)),
      test3.header.num_channels, test3.header.num_values / test3.header.num_channels);

  cols = std::min((int)mat.cols(), 10);
  trm = "";
  if (cols < mat.cols())
    trm = " ...";

  os << "Eigen::MatrixX<> form:" << std::endl;

  for (int ii = 0; ii < mat.rows(); ii++)
    os << mat.row(ii).block(0, 0, 1, cols) << trm << std::endl;
  LOG(INFO) << os.str();

  LOG(INFO) << "Row: " << std::endl << mat.row(0).block(0, 0, 1, cols) << trm;
  LOG(INFO) << "Col: " << std::endl << mat.col(0);

  size_t icol = mat.cols();

  size_t ncols = test3.header.num_values / test3.header.num_channels;
  mat.conservativeResize(Eigen::NoChange, mat.cols() + ncols);

  // mat.block(0, icol, test3.header.num_channels, ncols) = Eigen::Map<Eigen::MatrixX<int16_t>>(
  //     reinterpret_cast<int16_t *>(buff.data() + sizeof(UdpAcousticData::Header)),
  //     test3.header.num_channels, test3.header.num_values / test3.header.num_channels);

  cols = std::min((int)mat.cols(), 10);
  trm = "";
  if (cols < mat.cols())
    trm = " ...";

  os << "Eigen::MatrixX<> form:" << std::endl;

  for (int ii = 0; ii < mat.rows(); ii++)
    os << mat.row(ii).block(0, 0, 1, cols) << trm << std::endl;
  LOG(INFO) << os.str();

  LOG(INFO) << "Row: " << std::endl << mat.row(0).block(0, 0, 1, cols) << trm;
  LOG(INFO) << "Col: " << std::endl << mat.col(0);
}

void run_bf_raw_tests(std::string test_file_dir, std::string test_file_name) {
  // Test UdpBeamform*
  LOG(INFO) << "Checking Beamformer Data protocol : Raw (3D)";

  LOG(INFO) << "Running empty test (BeamformRaw)...";
  UdpBeamformRaw testbfraw;
  LOG(INFO) << "Result for empty test (BeamformRaw) " << std::endl << testbfraw;

  // LOG(INFO) << "Empty test " << std::endl
  //           << sizeof(testbf2d.header.id) << " " << testbf2d.header.id;
  // LOG(INFO) << "Empty test " << std::endl
  //           << sizeof(testbf2d.header.sid);

  // Real frame test Aco
  // > Load sample binary data from file
  LOG(INFO) << "Running valid data sample (DAT) test...";

  FILE *datfil;
  datfil = fopen((test_file_dir + test_file_name).c_str(), "rb");
  fseek(datfil, 0, SEEK_END);
  size_t n_chars = ftell(datfil);
  fseek(datfil, 0, SEEK_SET);

  LOG(INFO) << "Sample file is " << n_chars << " long";

  std::vector<int8_t> buff;

  int8_t val;
  bool is_eof = false;
  for (size_t ii = 0; ii < n_chars; ii++) {
    val = fgetc(datfil);
    buff.push_back(val);
  }

  fclose(datfil);

  UdpBeamformRaw testbfraw2(buff);

  LOG(INFO) << "Result for valid data sample (DAT) test " << std::endl << testbfraw2;
  LOG(INFO) << "Result for valid data sample (DAT) test : payload " << std::endl << testbfraw2.data;

  LOG(INFO) << "Result for encoder, input size : " << buff.size() * sizeof(buff.at(0));
  std::vector<int8_t> buff2;
  buff2 = testbfraw2.encode();
  LOG(INFO) << "Buffer comparison " << std::boolalpha << (buff == buff2);
  LOG(INFO) << "Buffer #1 length " << buff.size();
  LOG(INFO) << "Buffer #2 length " << buff2.size();
  size_t counter = 0;
  for (int ii = 0; ii < buff.size() && ii < buff2.size() && counter < 100; ii++) {
    // for (int ii = 0; ii < buff.size() && ii < buff2.size() && ii < 100; ii++) {
    if (buff.at(ii) != buff2.at(ii)) {
      LOG(INFO) << "  " << ii << " : " << (int)buff.at(ii) << " | " << (int)buff2.at(ii)
                << std::endl;
      counter++;
    }
  }

  LOG(INFO) << "End of encoder test" << std::endl << std::endl;
}

void run_bf_2d_tests(std::string test_file_dir, std::string test_file_name) {
  // Test UdpBeamform*
  LOG(INFO) << "Checking Beamformer Data protocol : Raw (3D)";

  LOG(INFO) << "Running empty test (Beamform2d)...";
  UdpBeamform2D testbf2d;
  LOG(INFO) << "Result for empty test (Beamform2d) " << std::endl << testbf2d;

  LOG(INFO) << "Running valid data sample (DAT) test...";

  FILE *datfil;
  datfil = fopen((test_file_dir + test_file_name).c_str(), "rb");
  fseek(datfil, 0, SEEK_END);
  size_t n_chars = ftell(datfil);
  fseek(datfil, 0, SEEK_SET);

  LOG(INFO) << "Sample file is " << n_chars << " long";

  std::vector<int8_t> buff;

  int8_t val;
  bool is_eof = false;
  for (size_t ii = 0; ii < n_chars; ii++) {
    val = fgetc(datfil);
    buff.push_back(val);
  }

  fclose(datfil);

  UdpBeamform2D testbf2d2(buff);

  LOG(INFO) << "Result for valid data sample (DAT) test " << std::endl << testbf2d2;
  LOG(INFO) << "Result for valid data sample (DAT) test : payload " << std::endl << testbf2d2.data;

  LOG(INFO) << "Result for encoder, input size : " << buff.size() * sizeof(buff.at(0));
  std::vector<int8_t> buff2;
  buff2 = testbf2d2.encode();
  LOG(INFO) << "Buffer comparison result : " << std::boolalpha << (buff == buff2);
  LOG(INFO) << "Buffer #1 length " << buff.size();
  LOG(INFO) << "Buffer #2 length " << buff2.size();
  size_t counter = 0;
  for (int ii = 0; ii < buff.size() && ii < buff2.size() && counter < 100; ii++) {
    // for (int ii = 0; ii < buff.size() && ii < buff2.size() && ii < 100; ii++) {
    if (buff.at(ii) != buff2.at(ii)) {
      LOG(INFO) << "  " << ii << " : " << (int)buff.at(ii) << " | " << (int)buff2.at(ii)
                << std::endl;
      counter++;
    }
  }

  LOG(INFO) << "End of encoder test" << std::endl << std::endl;
}

void run_pts_tests() {
  // Test UdpAcousticData
  LOG(INFO) << "Checking PTS Data protocol";

  // Empty test Aco
  LOG(INFO) << "Running empty test...";
  UdpData test;
  LOG(INFO) << "Result for empty test: " << std::endl << test;

  // Dummy test Aco
  LOG(INFO) << "Running invalid packet test...";
  std::string sample = "12er,gidm;,cdsda";
  std::vector<int8_t> buff(sample.begin(), sample.end());
  UdpData test2(buff);

  LOG(INFO) << "Result for invalid packet test  " << std::endl
            << "  >> Expect odd results since nothing is known about this packet, besides min "
               "required length"
            << std::endl
            << test2;
}
