/*******************************************************************/
/*    NAME: Oscar Viquez                                           */
/*    ORG:  Acbotics Research, LLC                                 */
/*    FILE: UdpAcousticData.h                                      */
/*    DATE: Apr 4th 2024                                           */
/*                                                                 */
/*    For help, contact us at: support@acbotics.com                */
/*******************************************************************/

#ifndef udp_acoustic_data_HEADER
#define udp_acoustic_data_HEADER

#include <Eigen/Dense>

#include "UdpData.h"

struct UdpAcousticData : public UdpData {
  struct __attribute__((__packed__)) Header {
    char id[2];
    int8_t ver_maj;
    int8_t ver_min;
    char endian;

    int8_t num_channels;
    int8_t data_size_bits;
    int32_t num_values;

    float sample_rate;
    int64_t start_time_nsec;
    uint64_t tick_time_nsec;

    int32_t adc_count;
    double scale;

    int32_t packet_num;

    Header() {
      this->id[0] = 'A';
      this->id[1] = 'C';
      this->ver_maj = 0;
      this->ver_min = 0;
      this->endian = '<';

      this->num_channels = 8;
      this->data_size_bits = 16;
      this->num_values = 0;

      this->sample_rate = 0;
      this->start_time_nsec = -1;
      this->tick_time_nsec = 0;

      this->adc_count = 0;
      this->scale = 1;

      this->packet_num = 0;
    };
    Header(std::vector<int8_t> &buff);
    void decode(std::vector<int8_t> &buff);
  } header;

  // May need better way to allocate this, if data_size becomes configurable
  Eigen::MatrixX<int16_t> data;

  UdpAcousticData();
  UdpAcousticData(std::vector<int8_t> &buff);
  bool unpack_data(std::vector<int8_t> &buff);
};

std::ostream &operator<<(std::ostream &os, const UdpAcousticData &st);
std::ostringstream &operator<<(std::ostringstream &os, const UdpAcousticData &st);

std::ostream &operator<<(std::ostream &os, const UdpAcousticData::Header &st);
std::ostringstream &operator<<(std::ostringstream &os, const UdpAcousticData::Header &st);

#endif
