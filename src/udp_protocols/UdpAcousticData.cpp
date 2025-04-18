/*******************************************************************/
/*    NAME: Oscar Viquez                                           */
/*    ORG:  Acbotics Research, LLC                                 */
/*    FILE: UdpAcousticData.cpp                                    */
/*    DATE: Apr 4th 2024                                           */
/*                                                                 */
/*    For help, contact us at: support@acbotics.com                */
/*******************************************************************/

#include <byteswap.h>
#include <cstring>
#include <glog/logging.h>
#include <iomanip>

// includes from within project
#include "UdpAcousticData.h"

UdpAcousticData::UdpAcousticData() { this->data = Eigen::MatrixX<int16_t>::Constant(0, 0, 0); }

UdpAcousticData::Header::Header(std::vector<int8_t> &buff) { this->decode(buff); }

void UdpAcousticData::Header::decode(std::vector<int8_t> &buff) {
  auto buff_raw = buff.data();

  size_t offset;
  int32_t num_values;
  int32_t sample_rate_legacy;
  float sample_rate;
  int64_t start_time_nsec;
  uint64_t tick_time_10nsec;
  int32_t adc_count;
  int64_t scale_buff;
  double scale;
  int32_t packet_num;

  this->id[0] = buff[0];
  this->id[1] = buff[1];
  this->ver_maj = (int)buff[2];
  this->ver_min = (int)buff[3];
  this->endian = buff[4];

  this->num_channels = (int)buff[5];
  this->data_size_bits = (int)buff[6];

  offset = 7;

  std::memcpy(&num_values, buff_raw + offset, sizeof(num_values));
  offset += sizeof(num_values);

  // Earlier firmware revisions transmitted sample_rate as int32
  std::memcpy(&sample_rate, buff_raw + offset, sizeof(sample_rate));
  std::memcpy(&sample_rate_legacy, buff_raw + offset, sizeof(sample_rate_legacy));
  offset += sizeof(sample_rate_legacy);

  std::memcpy(&start_time_nsec, buff_raw + offset, sizeof(start_time_nsec));
  offset += sizeof(start_time_nsec);

  bool has_tick_time = this->ver_maj >= 4;
  if (has_tick_time) {
    std::memcpy(&tick_time_10nsec, buff_raw + offset, sizeof(tick_time_10nsec));
    offset += sizeof(tick_time_10nsec);
  } else {
    tick_time_10nsec = 0;
  }
  std::memcpy(&adc_count, buff_raw + offset, sizeof(adc_count));
  offset += sizeof(adc_count);

  std::memcpy(&scale_buff, buff_raw + offset, sizeof(scale_buff));
  offset += sizeof(scale_buff);

  std::memcpy(&packet_num, buff_raw + offset, sizeof(packet_num));
  offset += sizeof(packet_num);

  if (this->endian == '<') {
    num_values = bswap_32(num_values);
    sample_rate = bswap_32(sample_rate);
    sample_rate_legacy = bswap_32(sample_rate_legacy);
    start_time_nsec = bswap_64(start_time_nsec);
    tick_time_10nsec = bswap_64(tick_time_10nsec);
    adc_count = bswap_32(adc_count);
    // scale_buff = bswap_64(scale_buff);
    packet_num = bswap_32(packet_num);
  }

  std::memcpy(&scale, &scale_buff, sizeof(scale));

  this->num_values = num_values;

  // Check firmware version for sampling rate format;
  //  < v3   : sample rate transmitted as int32
  // v3-v4.0 : sample rate transmitted as float (byte-swapped w/ type casting to uint)
  //  > v4.1 : sample rate transmitted as float (byte-swapped from raw bytes, no casting)
  if (this->ver_maj > 4 || (this->ver_maj == 4 && this->ver_min >= 1)) {
    std::memcpy(&(this->sample_rate), &sample_rate_legacy, sizeof(float));
  } else if (this->ver_maj >= 3) {
    this->sample_rate = sample_rate;
  } else {
    this->sample_rate = (float)sample_rate_legacy;
  }

  this->start_time_nsec = start_time_nsec;
  this->tick_time_nsec = tick_time_10nsec * 10;

  this->adc_count = adc_count;
  this->scale = scale;
  this->packet_num = packet_num;
}

UdpAcousticData::UdpAcousticData(std::vector<int8_t> &buff) {
  std::string buff_start(buff.begin(), buff.end());
  buff_start = buff_start.substr(0, 6);

  if (buff[0] == 'A' && buff[1] == 'C' && buff[2] != 'B' && buff.size() >= sizeof(Header)) {
    this->header = Header(buff);

    int data_size_bits = 16; // this->header.data_size_bits;
    if (this->header.data_size_bits != data_size_bits && FLAGS_debug_udp_data)
      VLOG(2) << "Acoustic data header: received data_size_bits="
              << (int)this->header.data_size_bits << "; expected " << data_size_bits;

    size_t _v4_correction = this->header.ver_maj >= 4 ? 0 : sizeof(Header::tick_time_nsec);

    if (buff.size() - (sizeof(Header) - _v4_correction) ==
        this->header.num_values * data_size_bits / 8) {
      this->unpack_data(buff);
    } else {
      LOG(INFO) << "Incomplete buffer; ignoring data payload";
      LOG(INFO) << this->header;
      LOG(INFO) << "Data available: " << buff.size() - (sizeof(Header) - _v4_correction);
      LOG(INFO) << "Data expected:  " << this->header.num_values * data_size_bits / 8;
    }
  } else {
    log_invalid_buffer(buff_start);
  }
}

bool UdpAcousticData::unpack_data(std::vector<int8_t> &buff) {
  size_t _v4_correction = this->header.ver_maj >= 4 ? 0 : sizeof(Header::tick_time_nsec);
  size_t offset = sizeof(Header) - _v4_correction;

  Eigen::MatrixX<int16_t> data(this->header.num_channels,
                               this->header.num_values / this->header.num_channels);
  data = Eigen::Map<Eigen::MatrixX<int16_t>>(reinterpret_cast<int16_t *>(buff.data() + offset),
                                             this->header.num_channels,
                                             this->header.num_values / this->header.num_channels);

  this->data = data;
  return true;
}

std::ostream &operator<<(std::ostream &os, const UdpAcousticData::Header &st) {
  std::ostringstream oss;
  oss << st;
  os << oss.str();
  return os;
}
std::ostringstream &operator<<(std::ostringstream &os, const UdpAcousticData::Header &st) {
  int width = 20;

  time_t start_time_sec = st.start_time_nsec / 1e9;
  struct tm *start_time_tm = std::gmtime(&start_time_sec);
  auto start_time_str = std::put_time(start_time_tm, "%Y-%m-%d %H:%M:%S %Z");

  os << "Acoustic Data Header:" << std::endl
     << std::left << std::setw(width) << std::setfill('.') << "ID" << ": " << st.id[0] << st.id[1]
     << std::endl
     << std::left << std::setw(width) << std::setfill('.') << "VERSION" << ": " << (int)st.ver_maj
     << "." << (int)st.ver_min << std::endl
     << std::left << std::setw(width) << std::setfill('.') << "ENDIAN" << ": " << st.endian
     << std::endl
     << std::left << std::setw(width) << std::setfill('.') << "NUM_CHANNELS" << ": "
     << (int)st.num_channels << " ch" << std::endl
     << std::left << std::setw(width) << std::setfill('.') << "DATA_SIZE" << ": "
     << (int)st.data_size_bits << " bits" << std::endl
     << std::left << std::setw(width) << std::setfill('.') << "NUM_VALUES" << ": " << st.num_values
     << std::endl
     << std::left << std::setw(width) << std::setfill('.') << "SAMPLE_RATE" << ": "
     << st.sample_rate << " Hz" << std::endl
     << std::left << std::setw(width) << std::setfill('.') << "START_TIME" << ": "
     << st.start_time_nsec << " nsec" << std::endl
     << std::left << std::setw(width) << std::setfill('.') << "" << ": " << start_time_str
     << std::endl
     << std::left << std::setw(width) << std::setfill('.') << "TICK_TIME" << ": "
     << st.tick_time_nsec << " nsec" << std::endl
     << std::left << std::setw(width) << std::setfill('.') << "ADC_COUNT" << ": " << st.adc_count
     << std::endl
     << std::left << std::setw(width) << std::setfill('.') << "SCALE" << ": " << st.scale
     << std::endl
     << std::left << std::setw(width) << std::setfill('.') << "PACKET_NUM" << ": " << st.packet_num
     << std::endl;

  return os;
}

std::ostream &operator<<(std::ostream &os, const UdpAcousticData &st) {
  std::ostringstream oss;
  oss << st;
  os << oss.str();
  return os;
}
std::ostringstream &operator<<(std::ostringstream &os, const UdpAcousticData &st) {
  os << "AcSense UDP protocol : Acoustic Data" << std::endl << st.header << std::endl;
  os << std::endl;

  return os;
}
