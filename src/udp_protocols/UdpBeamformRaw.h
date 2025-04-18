/*******************************************************************/
/*    NAME: Oscar Viquez                                           */
/*    ORG:  Acbotics Research, LLC                                 */
/*    FILE: UdpBeamformRaw.h                                       */
/*    DATE: Apr 4th 2024                                           */
/*                                                                 */
/*    For help, contact us at: support@acbotics.com                */
/*******************************************************************/

#ifndef udp_beamform_raw_HEADER
#define udp_beamform_raw_HEADER

#include <set>

#include "UdpData.h"

struct UdpBeamformRaw : public UdpData {
  struct __attribute__((__packed__)) Header {
    char id[2];
    char sid[2];
    int8_t ver_maj;
    int8_t ver_min;
    char endian;

    int32_t num_elements;
    int32_t num_frequencies;
    int32_t num_bearings;
    int32_t num_elevations;

    int8_t index_size_bytes;
    int8_t data_size_bytes;

    int32_t sample_rate;
    double window_length_sec;
    int64_t start_time_nsec;

    double xform_pitch_deg;
    double xform_roll_deg;
    double xform_yaw_deg;

    char mode;
    char weighting_type;

    int32_t num_subpackets;
    int32_t packet_num;

    Header() {
      this->id[0] = 'A';
      this->id[1] = 'C';

      // Tag SID as "start of raw series" packet: BR
      // > the alternative is "raw series continuation" packet: BC
      this->sid[0] = 'B';
      this->sid[1] = 'R';

      this->ver_maj = 0;
      this->ver_min = 0;
      this->endian = '<';

      this->num_elements = 0;

      this->num_frequencies = 0;
      this->num_bearings = 0;
      this->num_elevations = 0;

      this->index_size_bytes = 8;
      this->data_size_bytes = 8;

      this->sample_rate = 0;
      this->window_length_sec = 0;
      this->start_time_nsec = -1;

      this->xform_pitch_deg = 0;
      this->xform_roll_deg = 0;
      this->xform_yaw_deg = 0;

      this->mode = 'R';
      this->weighting_type = '?';
      this->num_subpackets = 0;
      this->packet_num = 0;
    };
    Header(std::vector<int8_t> &buff);
    void decode(std::vector<int8_t> &buff);
    std::vector<int8_t> encode();
  } header;

  struct Payload {
    std::vector<double> array_x;
    std::vector<double> array_y;
    std::vector<double> array_z;

    std::vector<double> frequencies;

    std::vector<bool> element_mask;
    std::vector<double> element_weights;

    std::vector<double> bearings_rad;
    std::vector<double> elevations_rad;

    std::vector<std::vector<std::vector<double>>> beampattern;

  } data;

  UdpBeamformRaw();
  UdpBeamformRaw(std::vector<int8_t> &buff);

  bool add_cont_packet(std::vector<int8_t> &buff, int32_t index);
  bool unpack_data();
  std::vector<int8_t> encode();

protected:
  std::vector<std::vector<int8_t>> binary_payload;
  std::set<int32_t> binary_payload_indices;
};

std::ostream &operator<<(std::ostream &os, const UdpBeamformRaw &st);
std::ostringstream &operator<<(std::ostringstream &os, const UdpBeamformRaw &st);

std::ostream &operator<<(std::ostream &os, const UdpBeamformRaw::Header &st);
std::ostringstream &operator<<(std::ostringstream &os, const UdpBeamformRaw::Header &st);

std::ostream &operator<<(std::ostream &os, const UdpBeamformRaw::Payload &st);
std::ostringstream &operator<<(std::ostringstream &os, const UdpBeamformRaw::Payload &st);

#endif
