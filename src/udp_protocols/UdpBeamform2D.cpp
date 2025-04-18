/*******************************************************************/
/*    NAME: Oscar Viquez                                           */
/*    ORG:  Acbotics Research, LLC                                 */
/*    FILE: UdpBeamform2D.cpp                                      */
/*    DATE: Apr 4th 2024                                           */
/*                                                                 */
/*    For help, contact us at: support@acbotics.com                */
/*******************************************************************/

#include <byteswap.h>
#include <cstring>
#include <glog/logging.h>
#include <iomanip>

// includes from within project
#include "UdpBeamform2D.h"

UdpBeamform2D::UdpBeamform2D() {}

UdpBeamform2D::Header::Header(std::vector<int8_t> &buff) { this->decode(buff); }

void UdpBeamform2D::Header::decode(std::vector<int8_t> &buff) {
  auto buff_raw = buff.data();

  size_t offset;

  int32_t num_elements;
  int32_t num_frequencies;
  int32_t num_bearings;
  int32_t num_elevations;

  int32_t sample_rate;
  int64_t window_length_sec_buff;
  double window_length_sec;
  int64_t start_time_nsec;

  int64_t xform_pitch_deg_buff;
  double xform_pitch_deg;
  int64_t xform_roll_deg_buff;
  double xform_roll_deg;
  int64_t xform_yaw_deg_buff;
  double xform_yaw_deg;

  int32_t packet_num;

  this->id[0] = buff[0];
  this->id[1] = buff[1];
  this->sid[0] = buff[2];
  this->sid[1] = buff[3];

  this->ver_maj = (int)buff[4];
  this->ver_min = (int)buff[5];
  this->endian = buff[6];

  offset = 7;

  std::memcpy(&num_elements, buff_raw + offset, sizeof(num_elements));
  offset += sizeof(num_elements);

  std::memcpy(&num_frequencies, buff_raw + offset, sizeof(num_frequencies));
  offset += sizeof(num_frequencies);

  std::memcpy(&num_bearings, buff_raw + offset, sizeof(num_bearings));
  offset += sizeof(num_bearings);

  std::memcpy(&num_elevations, buff_raw + offset, sizeof(num_elevations));
  offset += sizeof(num_elevations);

  this->index_size_bytes = (int)buff[offset];
  this->data_size_bytes = (int)buff[offset + 1];
  offset += 2;

  std::memcpy(&sample_rate, buff_raw + offset, sizeof(sample_rate));
  offset += sizeof(sample_rate);

  std::memcpy(&window_length_sec_buff, buff_raw + offset, sizeof(window_length_sec_buff));
  offset += sizeof(window_length_sec_buff);

  std::memcpy(&start_time_nsec, buff_raw + offset, sizeof(start_time_nsec));
  offset += sizeof(start_time_nsec);

  std::memcpy(&xform_pitch_deg_buff, buff_raw + offset, sizeof(xform_pitch_deg_buff));
  offset += sizeof(xform_pitch_deg_buff);

  std::memcpy(&xform_roll_deg_buff, buff_raw + offset, sizeof(xform_roll_deg_buff));
  offset += sizeof(xform_roll_deg_buff);

  std::memcpy(&xform_yaw_deg_buff, buff_raw + offset, sizeof(xform_yaw_deg_buff));
  offset += sizeof(xform_yaw_deg_buff);

  this->mode = buff[offset];
  this->weighting_type = buff[offset + 1];
  offset += 2;

  std::memcpy(&packet_num, buff_raw + offset, sizeof(packet_num));
  offset += sizeof(packet_num);

  if (this->endian == '<') {
    num_elements = bswap_32(num_elements);
    num_frequencies = bswap_32(num_frequencies);
    num_bearings = bswap_32(num_bearings);
    num_elevations = bswap_32(num_elevations);

    sample_rate = bswap_32(sample_rate);
    window_length_sec_buff = bswap_64(window_length_sec_buff);
    start_time_nsec = bswap_64(start_time_nsec);

    xform_pitch_deg_buff = bswap_32(xform_pitch_deg_buff);
    xform_roll_deg_buff = bswap_32(xform_roll_deg_buff);
    xform_yaw_deg_buff = bswap_32(xform_yaw_deg_buff);

    packet_num = bswap_32(packet_num);
  }

  std::memcpy(&window_length_sec, &window_length_sec_buff, sizeof(window_length_sec));

  std::memcpy(&xform_pitch_deg, &xform_pitch_deg_buff, sizeof(xform_pitch_deg));
  std::memcpy(&xform_roll_deg, &xform_roll_deg_buff, sizeof(xform_roll_deg));
  std::memcpy(&xform_yaw_deg, &xform_yaw_deg_buff, sizeof(xform_yaw_deg));

  this->num_elements = num_elements;
  this->num_frequencies = num_frequencies;
  this->num_bearings = num_bearings;
  this->num_elevations = num_elevations;

  this->sample_rate = sample_rate;
  this->window_length_sec = window_length_sec;
  this->start_time_nsec = start_time_nsec;

  this->xform_pitch_deg = xform_pitch_deg;
  this->xform_roll_deg = xform_roll_deg;
  this->xform_yaw_deg = xform_yaw_deg;

  this->packet_num = packet_num;
}

UdpBeamform2D::UdpBeamform2D(std::vector<int8_t> &buff) {
  std::string buff_start(buff.begin(), buff.end());
  buff_start = buff_start.substr(0, 6);

  if (buff[0] == 'A' && buff[1] == 'C' && buff[2] == 'B' && buff[3] == '2' &&
      buff.size() >= sizeof(Header)) {
    this->header = Header(buff);
    this->unpack_data(buff);
  } else {
    log_invalid_buffer(buff_start);
  }
}

bool UdpBeamform2D::unpack_data(std::vector<int8_t> &buff) {

  // set the offset to skip over the primary packet's header
  size_t offset = sizeof(Header);

  // Load the array coordinates:
  this->data.array_x = Eigen::Map<Eigen::VectorXd>(reinterpret_cast<double *>(buff.data() + offset),
                                                   this->header.num_elements);
  offset += sizeof(double) * this->header.num_elements;

  this->data.array_y = Eigen::Map<Eigen::VectorXd>(reinterpret_cast<double *>(buff.data() + offset),
                                                   this->header.num_elements);
  offset += sizeof(double) * this->header.num_elements;

  this->data.array_z = Eigen::Map<Eigen::VectorXd>(reinterpret_cast<double *>(buff.data() + offset),
                                                   this->header.num_elements);
  offset += sizeof(double) * this->header.num_elements;

  // Load the frequency bins:
  this->data.frequencies = Eigen::Map<Eigen::VectorXd>(
      reinterpret_cast<double *>(buff.data() + offset), this->header.num_frequencies);
  offset += sizeof(double) * this->header.num_frequencies;

  // Load the element mask + weights
  this->data.element_mask =
      (Eigen::Map<Eigen::VectorX<int8_t>>(reinterpret_cast<int8_t *>(buff.data() + offset),
                                          this->header.num_elements)
           .array() == 1);
  offset += sizeof(int8_t) * this->header.num_elements;

  this->data.element_weights = Eigen::Map<Eigen::VectorXd>(
      reinterpret_cast<double *>(buff.data() + offset), this->header.num_elements);
  offset += sizeof(double) * this->header.num_elements;

  // Load the look angles: bearing & elevation
  this->data.bearings_rad = Eigen::Map<Eigen::VectorXd>(
      reinterpret_cast<double *>(buff.data() + offset), this->header.num_bearings);
  offset += sizeof(double) * this->header.num_bearings;

  this->data.elevations_rad = Eigen::Map<Eigen::VectorXd>(
      reinterpret_cast<double *>(buff.data() + offset), this->header.num_elevations);
  offset += sizeof(double) * this->header.num_elevations;

  // Load the beampattern itself
  this->data.beampattern =
      Eigen::Map<Eigen::MatrixXd>(reinterpret_cast<double *>(buff.data() + offset),
                                  this->header.num_elevations, this->header.num_bearings)
          .transpose();

  return true;
}

std::vector<int8_t> UdpBeamform2D::Header::encode() {
  Header _header = *this;
  std::vector<int8_t> buff;

  if (this->endian == '<') {

    int64_t window_length_sec_buff;
    int64_t xform_pitch_deg_buff;
    int64_t xform_roll_deg_buff;
    int64_t xform_yaw_deg_buff;

    std::memcpy(&window_length_sec_buff, &_header.window_length_sec,
                sizeof(window_length_sec_buff));
    std::memcpy(&xform_pitch_deg_buff, &_header.xform_pitch_deg, sizeof(xform_pitch_deg_buff));
    std::memcpy(&xform_roll_deg_buff, &_header.xform_roll_deg, sizeof(xform_roll_deg_buff));
    std::memcpy(&xform_yaw_deg_buff, &_header.xform_yaw_deg, sizeof(xform_yaw_deg_buff));

    _header.num_elements = bswap_32(_header.num_elements);
    _header.num_frequencies = bswap_32(_header.num_frequencies);
    _header.num_bearings = bswap_32(_header.num_bearings);
    _header.num_elevations = bswap_32(_header.num_elevations);

    _header.sample_rate = bswap_32(_header.sample_rate);
    window_length_sec_buff = bswap_64(window_length_sec_buff);
    _header.start_time_nsec = bswap_64(_header.start_time_nsec);

    xform_pitch_deg_buff = bswap_32(xform_pitch_deg_buff);
    xform_roll_deg_buff = bswap_32(xform_roll_deg_buff);
    xform_yaw_deg_buff = bswap_32(xform_yaw_deg_buff);

    _header.packet_num = bswap_32(_header.packet_num);

    std::memcpy(&_header.window_length_sec, &window_length_sec_buff,
                sizeof(window_length_sec_buff));
    std::memcpy(&_header.xform_pitch_deg, &xform_pitch_deg_buff, sizeof(xform_pitch_deg_buff));
    std::memcpy(&_header.xform_roll_deg, &xform_roll_deg_buff, sizeof(xform_roll_deg_buff));
    std::memcpy(&_header.xform_yaw_deg, &xform_yaw_deg_buff, sizeof(xform_yaw_deg_buff));
  }
  buff.resize(sizeof(Header));
  std::memcpy(buff.data(), &_header, sizeof(Header));
  return buff;
}

std::vector<int8_t> UdpBeamform2D::encode() {
  auto d = this->data;
  std::vector<int8_t> buff;
  Eigen::VectorX<int8_t> element_mask = d.element_mask.cast<int8_t>();

  size_t sz_header = sizeof(Header);

  size_t num_ch = d.array_x.size();
  size_t sz_coords_val = sizeof(d.array_x(0));
  size_t sz_coords = num_ch * sz_coords_val;

  size_t sz_frequencies = d.frequencies.size() * sizeof(d.frequencies(0));

  size_t sz_element_mask = element_mask.size() * sizeof(element_mask(0));
  size_t sz_element_weights = d.element_weights.size() * sizeof(d.element_weights(0));

  size_t sz_bearings_rad = d.bearings_rad.size() * sizeof(d.bearings_rad(0));
  size_t sz_elevations_rad = d.elevations_rad.size() * sizeof(d.elevations_rad(0));

  size_t sz_beampattern_items = d.beampattern.size();
  size_t sz_beampattern_val = sizeof(d.beampattern(0, 0));
  size_t sz_beampattern = sz_beampattern_items * sz_beampattern_val;

  size_t sz_all = sz_header + sz_coords * 3 + sz_frequencies + sz_element_mask +
                  sz_element_weights + sz_bearings_rad + sz_elevations_rad + sz_beampattern;

  buff = this->header.encode();
  buff.resize(sz_all);

  size_t offset = sz_header;

  std::memcpy(buff.data() + offset, d.array_x.data(), sz_coords);
  offset += sz_coords;

  std::memcpy(buff.data() + offset, d.array_y.data(), sz_coords);
  offset += sz_coords;

  std::memcpy(buff.data() + offset, d.array_z.data(), sz_coords);
  offset += sz_coords;

  std::memcpy(buff.data() + offset, d.frequencies.data(), sz_frequencies);
  offset += sz_frequencies;

  std::memcpy(buff.data() + offset, element_mask.data(), sz_element_mask);
  offset += sz_element_mask;

  std::memcpy(buff.data() + offset, d.element_weights.data(), sz_element_weights);
  offset += sz_element_weights;

  std::memcpy(buff.data() + offset, d.bearings_rad.data(), sz_bearings_rad);
  offset += sz_bearings_rad;

  std::memcpy(buff.data() + offset, d.elevations_rad.data(), sz_elevations_rad);
  offset += sz_elevations_rad;

  Eigen::MatrixXd mm = d.beampattern.transpose();
  std::memcpy(buff.data() + offset, mm.data(), sz_beampattern);

  return buff;
}

Eigen::MatrixXd UdpBeamform2D::get_1D() { return this->data.beampattern.rowwise().mean(); }

std::ostream &operator<<(std::ostream &os, const UdpBeamform2D::Header &st) {
  std::ostringstream oss;
  oss << st;
  os << oss.str();
  return os;
}
std::ostringstream &operator<<(std::ostringstream &os, const UdpBeamform2D::Header &st) {
  int width = 20;

  time_t start_time_sec = st.start_time_nsec / 1e9;
  struct tm *start_time_tm = std::gmtime(&start_time_sec);
  auto start_time_str = std::put_time(start_time_tm, "%Y-%m-%d %H:%M:%S %Z");

  os << "Beamformer 2D Data Header:" << std::endl
     << std::left << std::setw(width) << std::setfill('.') << "ID"
     << ": " << st.id[0] << st.id[1] << std::endl
     << std::left << std::setw(width) << std::setfill('.') << "SID"
     << ": " << st.sid[0] << st.sid[1] << std::endl
     << std::left << std::setw(width) << std::setfill('.') << "VERSION"
     << ": " << (int)st.ver_maj << "." << (int)st.ver_min << std::endl
     << std::left << std::setw(width) << std::setfill('.') << "ENDIAN"
     << ": " << st.endian << std::endl
     << std::left << std::setw(width) << std::setfill('.') << "NUM_ELEMENTS"
     << ": " << st.num_elements << " ch" << std::endl
     << std::left << std::setw(width) << std::setfill('.') << "NUM_FREQUENCIES"
     << ": " << st.num_frequencies << std::endl
     << std::left << std::setw(width) << std::setfill('.') << "NUM_BEARINGS"
     << ": " << st.num_bearings << std::endl
     << std::left << std::setw(width) << std::setfill('.') << "NUM_ELEVATIONS"
     << ": " << st.num_elevations << std::endl
     << std::left << std::setw(width) << std::setfill('.') << "INDEX_SIZE"
     << ": " << (int)st.index_size_bytes << " bytes" << std::endl
     << std::left << std::setw(width) << std::setfill('.') << "DATA_SIZE"
     << ": " << (int)st.data_size_bytes << " bytes" << std::endl
     << std::left << std::setw(width) << std::setfill('.') << "SAMPLE_RATE"
     << ": " << st.sample_rate << " Hz" << std::endl
     << std::left << std::setw(width) << std::setfill('.') << "WINDOW_LENGTH_S"
     << ": " << st.window_length_sec << std::endl
     << std::left << std::setw(width) << std::setfill('.') << "START_TIME"
     << ": " << st.start_time_nsec << " nsec" << std::endl
     << std::left << std::setw(width) << std::setfill('.') << ""
     << ": " << start_time_str << std::endl
     << std::left << std::setw(width) << std::setfill('.') << "XFORM_PITCH"
     << ": " << st.xform_pitch_deg << " deg" << std::endl
     << std::left << std::setw(width) << std::setfill('.') << "XFORM_ROLL"
     << ": " << st.xform_roll_deg << " deg" << std::endl
     << std::left << std::setw(width) << std::setfill('.') << "XFORM_YAW"
     << ": " << st.xform_yaw_deg << " deg" << std::endl
     << std::left << std::setw(width) << std::setfill('.') << "MODE"
     << ": " << st.mode << std::endl
     << std::left << std::setw(width) << std::setfill('.') << "WEIGHTING_TYPE"
     << ": " << st.weighting_type << std::endl
     << std::left << std::setw(width) << std::setfill('.') << "PACKET_NUM"
     << ": " << st.packet_num << std::endl;

  return os;
}

std::ostream &operator<<(std::ostream &os, const UdpBeamform2D &st) {
  std::ostringstream oss;
  oss << st;
  os << oss.str();
  return os;
}
std::ostringstream &operator<<(std::ostringstream &os, const UdpBeamform2D &st) {

  int width = 25;

  os << "AcSense UDP protocol : Beamformer 2D Data" << std::endl << st.header << std::endl;
  os << std::endl;

  // Don't use the full Payload operator<< -- use summary for a general packet view
  os << "Beamformer Raw Data Payload (Summary):" << std::endl;
  os << std::left << std::setw(width) << std::setfill('.') << "ARRAY_X"
     << ": (" << st.data.array_x.size() << ")" << std::endl;
  os << std::left << std::setw(width) << std::setfill('.') << "ARRAY_Y"
     << ": (" << st.data.array_y.size() << ")" << std::endl;
  os << std::left << std::setw(width) << std::setfill('.') << "ARRAY_Z"
     << ": (" << st.data.array_z.size() << ")" << std::endl;
  os << std::left << std::setw(width) << std::setfill('.') << "FREQUENCIES"
     << ": (" << st.data.frequencies.size() << ")" << std::endl;
  os << std::left << std::setw(width) << std::setfill('.') << "ELEMENT_MASK"
     << ": (" << st.data.element_mask.size() << ")" << std::endl;
  os << std::left << std::setw(width) << std::setfill('.') << "ELEMENT_WEIGHTS"
     << ": (" << st.data.element_weights.size() << ")" << std::endl;
  os << std::left << std::setw(width) << std::setfill('.') << "BEARINGS"
     << ": (" << st.data.bearings_rad.size() << ")" << std::endl;
  os << std::left << std::setw(width) << std::setfill('.') << "ELEVATIONS"
     << ": (" << st.data.elevations_rad.size() << ")" << std::endl;

  int beam_d1 = st.data.beampattern.rows();
  int beam_d2 = st.data.beampattern.cols();

  os << std::left << std::setw(width) << std::setfill('.') << "BEAMPATTERN"
     << ": (" << beam_d1 << " x " << beam_d2 << ")" << std::endl
     << std::endl
     << std::endl;

  return os;
}

std::ostream &operator<<(std::ostream &os, const UdpBeamform2D::Payload &st) {
  std::ostringstream oss;
  oss << st;
  os << oss.str();
  return os;
}
std::ostringstream &operator<<(std::ostringstream &os, const UdpBeamform2D::Payload &st) {
  int width = 25;

  os << "AcSense UDP protocol : Beamformer Raw Data Payload" << std::endl;

  os << std::left << std::setw(width) << std::setfill('.') << "ARRAY_X"
     << ": (" << st.array_x.size() << ")" << std::endl;
  for (int ii = 0; ii < st.array_x.size(); ii++) {
    os << std::right << std::setw(width + 2) << std::setfill(' ') << "  " << st.array_x[ii]
       << std::endl;
  }
  os << std::endl;

  os << std::left << std::setw(width) << std::setfill('.') << "ARRAY_Y"
     << ": (" << st.array_y.size() << ")" << std::endl;
  for (int ii = 0; ii < st.array_y.size(); ii++) {
    os << std::right << std::setw(width + 2) << std::setfill(' ') << "  " << st.array_y[ii]
       << std::endl;
  }
  os << std::endl;

  os << std::left << std::setw(width) << std::setfill('.') << "ARRAY_Z"
     << ": (" << st.array_z.size() << ")" << std::endl;
  for (int ii = 0; ii < st.array_z.size(); ii++) {
    os << std::right << std::setw(width + 2) << std::setfill(' ') << "  " << st.array_z[ii]
       << std::endl;
  }
  os << std::endl;

  os << std::left << std::setw(width) << std::setfill('.') << "FREQUENCIES"
     << ": (" << st.frequencies.size() << ")" << std::endl;
  for (int ii = 0; ii < st.frequencies.size(); ii++) {
    os << std::right << std::setw(width + 2) << std::setfill(' ') << "  " << std::setprecision(6)
       << st.frequencies[ii] << std::endl;
  }
  os << std::endl;

  os << std::left << std::setw(width) << std::setfill('.') << "ELEMENT_MASK"
     << ": (" << st.element_mask.size() << ")" << std::endl;
  for (int ii = 0; ii < st.element_mask.size(); ii++) {
    os << std::right << std::setw(width + 2) << std::setfill(' ') << "  " << st.element_mask[ii]
       << std::endl;
  }
  os << std::endl;

  os << std::left << std::setw(width) << std::setfill('.') << "ELEMENT_WEIGHTS"
     << ": (" << st.element_weights.size() << ")" << std::endl;
  for (int ii = 0; ii < st.element_weights.size(); ii++) {
    os << std::right << std::setw(width + 2) << std::setfill(' ') << "  " << st.element_weights[ii]
       << std::endl;
  }
  os << std::endl;

  os << std::left << std::setw(width) << std::setfill('.') << "BEARINGS"
     << ": (" << st.bearings_rad.size() << ")" << std::endl;
  for (int ii = 0; ii < st.bearings_rad.size(); ii++) {
    os << std::right << std::setw(width + 2) << std::setfill(' ') << "  " << std::setprecision(6)
       << st.bearings_rad[ii] << std::endl;
  }
  os << std::endl;

  os << std::left << std::setw(width) << std::setfill('.') << "ELEVATIONS"
     << ": (" << st.elevations_rad.size() << ")" << std::endl;
  for (int ii = 0; ii < st.elevations_rad.size(); ii++) {
    os << std::right << std::setw(width + 2) << std::setfill(' ') << "  " << std::setprecision(6)
       << st.elevations_rad[ii] << std::endl;
  }
  os << std::endl;

  /*===============================*/
  /*===============================*/
  /*===============================*/

  int beam_d1 = st.beampattern.rows();
  int beam_d2 = st.beampattern.cols();

  os << std::left << std::setw(width) << std::setfill('.') << "BEAMPATTERN"
     << ": (" << beam_d1 << " x " << beam_d2 << ")" << std::endl
     << std::endl
     << std::endl;

  return os;
}
