/*******************************************************************/
/*    NAME: Oscar Viquez                                           */
/*    ORG:  Acbotics Research, LLC                                 */
/*    FILE: UdpBeamformRaw.cpp                                     */
/*    DATE: Apr 4th 2024                                           */
/*                                                                 */
/*    For help, contact us at: support@acbotics.com                */
/*******************************************************************/

#include <byteswap.h>
#include <cstring>
#include <glog/logging.h>
#include <iomanip>

// includes from within project
#include "UdpBeamformRaw.h"

UdpBeamformRaw::UdpBeamformRaw() {}

UdpBeamformRaw::Header::Header(std::vector<int8_t> &buff) : UdpBeamformRaw::Header::Header() {
  this->decode(buff);
}

void UdpBeamformRaw::Header::decode(std::vector<int8_t> &buff) {
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

  int32_t num_subpackets;
  int32_t packet_num;

  this->id[0] = buff[0];
  this->id[1] = buff[1];
  this->sid[0] = buff[2];
  this->sid[1] = buff[3];

  offset = 4;

  if (this->sid[1] == 'R') {
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

    std::memcpy(&num_subpackets, buff_raw + offset, sizeof(num_subpackets));
    offset += sizeof(num_subpackets);

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

      num_subpackets = bswap_32(num_subpackets);
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

    this->num_subpackets = num_subpackets;
    this->packet_num = packet_num;
  } else if (this->sid[1] == 'C') {

    std::memcpy(&num_subpackets, buff_raw + offset, sizeof(num_subpackets));
    offset += sizeof(num_subpackets);

    std::memcpy(&packet_num, buff_raw + offset, sizeof(packet_num));
    offset += sizeof(packet_num);

    if (this->endian == '<') {
      // this takes the endian from the default constructor since it's not provided in ACBC header
      num_subpackets = bswap_32(num_subpackets);
      packet_num = bswap_32(packet_num);
    }

    this->num_subpackets = num_subpackets;
    this->packet_num = packet_num;
  }
}

UdpBeamformRaw::UdpBeamformRaw(std::vector<int8_t> &buff) {
  std::string buff_start(buff.begin(), buff.end());
  buff_start = buff_start.substr(0, 6);

  if (buff[0] == 'A' && buff[1] == 'C' && buff[2] == 'B' && buff[3] == 'R' &&
      buff.size() >= sizeof(Header)) {
    // This is a primary packet; allocate memory to handle cont packets
    this->header = Header(buff);
    this->binary_payload.resize(this->header.num_subpackets);
    // preserve the primary header
    this->binary_payload[0] = std::vector<int8_t>(buff.begin(), buff.end());
    this->binary_payload_indices.insert(0);
    if (this->header.num_subpackets == 1) {
      this->unpack_data();
    }

  } else if (buff[0] == 'A' && buff[1] == 'C' && buff[2] == 'B' &&
             (buff[3] == 'C' && buff.size() >= 4 + sizeof(int32_t) * 2)) {
    // this is a continuation packet; handle known subfields to match with a primary
    this->header = Header(buff);
  } else {
    log_invalid_buffer(buff_start);
  }
}

bool UdpBeamformRaw::add_cont_packet(std::vector<int8_t> &buff, int32_t index) {
  if (this->binary_payload_indices.find(index) == this->binary_payload_indices.end()) {
    this->binary_payload_indices.insert(index);
    this->binary_payload[index] = std::vector<int8_t>(buff.begin(), buff.end());
    return unpack_data();
  }
  return false;
}

bool UdpBeamformRaw::unpack_data() {

  if (this->binary_payload_indices.size() == this->header.num_subpackets) {

    int contd_header_size = 4 + sizeof(int32_t) * 2;
    size_t _payload_size = 0;
    std::vector<int8_t> full_payload;
    size_t offset;
    double val_d;
    int8_t val_8;

    // elements in binary payload should be self-sorted per add_cont_packet();
    // consolidate into a single unified buffer for parsing:

    _payload_size += this->binary_payload[0].size();
    VLOG(4) << "Number of packets: " << this->binary_payload.size();
    for (int ii = 1; ii < this->binary_payload.size(); ii++) {
      VLOG(5) << "Appending packet: " << ii;
      _payload_size += this->binary_payload[ii].size() - contd_header_size;
    }

    full_payload.reserve(_payload_size);

    std::copy(this->binary_payload[0].begin(), this->binary_payload[0].end(),
              std::back_inserter(full_payload));

    for (int ii = 1; ii < this->binary_payload.size(); ii++) {

      Header test(this->binary_payload[ii]);
      VLOG(5) << "Loading sub-packet index :" << test.num_subpackets;

      // full_payload.insert(full_payload.end(),
      //                     std::make_move_iterator(this->binary_payload[ii].begin()),
      //                     std::make_move_iterator(this->binary_payload[ii].end()));
      std::copy(this->binary_payload[ii].begin() + contd_header_size,
                this->binary_payload[ii].end(), std::back_inserter(full_payload));
    }

    if (full_payload.size() != _payload_size) {

      LOG(ERROR) << "Something went wrong with the consolidation of the full payload!";
      LOG(INFO) << full_payload.size() << " vs " << _payload_size;
      return false;
    }

    // with the buffer now successfully unified,
    // pre-allocate memory blocks for unpacking
    this->data.array_x.reserve(header.num_elements);
    this->data.array_y.reserve(header.num_elements);
    this->data.array_z.reserve(header.num_elements);

    this->data.frequencies.reserve(header.num_frequencies);

    this->data.element_mask.reserve(header.num_elements);
    this->data.element_weights.reserve(header.num_elements);

    this->data.bearings_rad.reserve(header.num_bearings);
    this->data.elevations_rad.reserve(header.num_elevations);

    // set the offset to skip over the primary packet's header
    offset = sizeof(Header);

    for (int ii = 0; ii < header.num_elements; ii++) {
      std::memcpy(&val_d, full_payload.data() + offset, sizeof(val_d));
      offset += sizeof(val_d);
      this->data.array_x.push_back(val_d);
    }
    for (int ii = 0; ii < header.num_elements; ii++) {
      std::memcpy(&val_d, full_payload.data() + offset, sizeof(val_d));
      offset += sizeof(val_d);
      this->data.array_y.push_back(val_d);
    }
    for (int ii = 0; ii < header.num_elements; ii++) {
      std::memcpy(&val_d, full_payload.data() + offset, sizeof(val_d));
      offset += sizeof(val_d);
      this->data.array_z.push_back(val_d);
    }

    for (int ii = 0; ii < header.num_frequencies; ii++) {
      std::memcpy(&val_d, full_payload.data() + offset, sizeof(val_d));
      offset += sizeof(val_d);
      this->data.frequencies.push_back(val_d);
    }

    for (int ii = 0; ii < header.num_elements; ii++) {
      std::memcpy(&val_8, full_payload.data() + offset, sizeof(val_8));
      offset += sizeof(val_8);
      this->data.element_mask.push_back(val_8 == 1);
    }

    for (int ii = 0; ii < header.num_elements; ii++) {
      std::memcpy(&val_d, full_payload.data() + offset, sizeof(val_d));
      offset += sizeof(val_d);
      this->data.element_weights.push_back(val_d);
    }

    for (int ii = 0; ii < header.num_bearings; ii++) {
      std::memcpy(&val_d, full_payload.data() + offset, sizeof(val_d));
      offset += sizeof(val_d);
      this->data.bearings_rad.push_back(val_d);
    }

    for (int ii = 0; ii < header.num_elevations; ii++) {
      std::memcpy(&val_d, full_payload.data() + offset, sizeof(val_d));
      offset += sizeof(val_d);
      this->data.elevations_rad.push_back(val_d);
    }

    std::vector<double> slice_ff;
    std::vector<std::vector<double>> slice_ee;

    for (auto bb : this->data.bearings_rad) {
      slice_ee.clear();

      for (auto ee : this->data.elevations_rad) {
        slice_ff.clear();
        for (auto ff : this->data.frequencies) {
          std::memcpy(&val_d, full_payload.data() + offset, sizeof(val_d));
          offset += sizeof(val_d);
          slice_ff.push_back(val_d);
        }
        slice_ee.push_back(slice_ff);
      }

      this->data.beampattern.push_back(slice_ee);
    }

    // clear these when data has been successfully parsed,
    // ie before passing this packet into a processing queue
    this->binary_payload.clear();
    this->binary_payload_indices.clear();

    return true;
  }
  return false;
}

std::vector<int8_t> UdpBeamformRaw::Header::encode() {
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

    _header.num_subpackets = bswap_32(_header.num_subpackets);
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

std::vector<int8_t> UdpBeamformRaw::encode() {
  auto d = this->data;
  std::vector<int8_t> buff;
  std::vector<int8_t> element_mask;

  for (int ii = 0; ii < d.element_mask.size(); ii++) {
    element_mask.push_back(d.element_mask.at(ii) ? 1 : 0);
  }

  size_t sz_header = sizeof(Header);

  size_t num_ch = d.array_x.size();
  size_t sz_coords_val = sizeof(d.array_x.at(0));
  size_t sz_coords = num_ch * sz_coords_val;

  size_t sz_frequencies = d.frequencies.size() * sizeof(d.frequencies.at(0));

  size_t sz_element_mask = element_mask.size() * sizeof(element_mask.at(0));
  size_t sz_element_weights = d.element_weights.size() * sizeof(d.element_weights.at(0));

  size_t sz_bearings_rad = d.bearings_rad.size() * sizeof(d.bearings_rad.at(0));
  size_t sz_elevations_rad = d.elevations_rad.size() * sizeof(d.elevations_rad.at(0));

  size_t sz_beampattern_i = d.beampattern.size();
  size_t sz_beampattern_j = d.beampattern.at(0).size();
  size_t sz_beampattern_k = d.beampattern.at(0).at(0).size();
  size_t sz_beampattern_val = sizeof(d.beampattern.at(0).at(0).at(0));

  size_t sz_beampattern =
      sz_beampattern_i * sz_beampattern_j * sz_beampattern_k * sz_beampattern_val;

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

  for (int ii = 0; ii < sz_beampattern_i; ii++) {
    for (int jj = 0; jj < sz_beampattern_j; jj++) {
      std::memcpy(buff.data() + offset, d.beampattern.at(ii).at(jj).data(),
                  sz_beampattern_k * sz_beampattern_val);
      offset += sz_beampattern_k * sz_beampattern_val;
    }
  }

  return buff;
}

std::ostream &operator<<(std::ostream &os, const UdpBeamformRaw::Header &st) {
  std::ostringstream oss;
  oss << st;
  os << oss.str();
  return os;
}
std::ostringstream &operator<<(std::ostringstream &os, const UdpBeamformRaw::Header &st) {
  int width = 20;

  time_t start_time_sec = st.start_time_nsec / 1e9;
  struct tm *start_time_tm = std::gmtime(&start_time_sec);
  auto start_time_str = std::put_time(start_time_tm, "%Y-%m-%d %H:%M:%S %Z");

  if (st.sid[1] == 'C') {
    os << "Beamformer Raw Data Header:" << std::endl
       << std::left << std::setw(width) << std::setfill('.') << "ID"
       << ": " << st.id[0] << st.id[1] << std::endl
       << std::left << std::setw(width) << std::setfill('.') << "SID"
       << ": " << st.sid[0] << st.sid[1] << std::endl
       << std::left << std::setw(width) << std::setfill('.') << "NUM_SUBPACKETS"
       << ": " << st.num_subpackets << " (index)" << std::endl
       << std::left << std::setw(width) << std::setfill('.') << "PACKET_NUM"
       << ": " << st.packet_num << std::endl;
    return os;
  }

  os << "Beamformer Raw Data Header:" << std::endl
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
     << std::left << std::setw(width) << std::setfill('.') << "NUM_SUBPACKETS"
     << ": " << st.num_subpackets << std::endl
     << std::left << std::setw(width) << std::setfill('.') << "PACKET_NUM"
     << ": " << st.packet_num << std::endl;

  return os;
}

std::ostream &operator<<(std::ostream &os, const UdpBeamformRaw &st) {
  std::ostringstream oss;
  oss << st;
  os << oss.str();
  return os;
}
std::ostringstream &operator<<(std::ostringstream &os, const UdpBeamformRaw &st) {

  int width = 25;

  os << "AcSense UDP protocol : Beamformer Raw Data" << std::endl
     << st.header << std::endl
     << std::endl;

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

  int beam_d1;
  int beam_d2;
  int beam_d3;
  if (st.data.beampattern.size() > 0) {
    beam_d1 = st.data.beampattern.size();

    if (st.data.beampattern[0].size() > 0) {
      beam_d2 = st.data.beampattern[0].size();

      if (st.data.beampattern[0][0].size() > 0) {
        beam_d3 = st.data.beampattern[0][0].size();
      }
    }
  }
  os << std::left << std::setw(width) << std::setfill('.') << "BEAMPATTERN"
     << ": (" << beam_d1 << " x " << beam_d2 << " x " << beam_d3 << ")" << std::endl
     << std::endl
     << std::endl;

  return os;
}

std::ostream &operator<<(std::ostream &os, const UdpBeamformRaw::Payload &st) {
  std::ostringstream oss;
  oss << st;
  os << oss.str();
  return os;
}
std::ostringstream &operator<<(std::ostringstream &os, const UdpBeamformRaw::Payload &st) {
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

  // os << "Raw data : " << std::endl;
  // for (int bb = 0; bb < st.header.num_bearings; bb++) {
  //   for (int ee = 0; ee < st.header.num_elevations; ee++) {
  //     for (int ff = 0; ff < st.header.num_frequencies; ff++) {
  //       os << st.data[bb][ee][ff] << ", ";
  //     }
  //     os << std::endl;
  //   }
  //   os << std::endl;
  // }

  int beam_d1;
  int beam_d2;
  int beam_d3;
  if (st.beampattern.size() > 0) {
    beam_d1 = st.beampattern.size();

    if (st.beampattern[0].size() > 0) {
      beam_d2 = st.beampattern[0].size();

      if (st.beampattern[0][0].size() > 0) {
        beam_d3 = st.beampattern[0][0].size();
      }
    }
  }
  os << std::left << std::setw(width) << std::setfill('.') << "BEAMPATTERN"
     << ": (" << beam_d1 << " x " << beam_d2 << " x " << beam_d3 << ")" << std::endl
     << std::endl
     << std::endl;

  return os;
}
