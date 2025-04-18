/*******************************************************************/
/*    NAME: Oscar Viquez                                           */
/*    ORG:  Acbotics Research, LLC                                 */
/*    FILE: UdpBeamform2D.h                                        */
/*    DATE: Apr 4th 2024                                           */
/*                                                                 */
/*    For help, contact us at: support@acbotics.com                */
/*******************************************************************/

#ifndef udp_beamform_2d_HEADER
#define udp_beamform_2d_HEADER

#include <Eigen/Dense>

#include "UdpData.h"

struct UdpBeamform2D : public UdpData {
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

    int32_t packet_num;

    Header() {
      this->id[0] = 'A';
      this->id[1] = 'C';
      this->sid[0] = 'B';
      this->sid[1] = '2';
      this->ver_maj = 0;
      this->ver_min = 0;
      this->endian = '<';

      this->num_elements = 8;
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

      this->mode = 'm';
      this->weighting_type = '?';
      this->packet_num = 0;
    };
    Header(std::vector<int8_t> &buff);
    void decode(std::vector<int8_t> &buff);
    std::vector<int8_t> encode();
  } header;

  struct Payload {
    Eigen::VectorXd array_x;
    Eigen::VectorXd array_y;
    Eigen::VectorXd array_z;

    Eigen::VectorXd frequencies;

    Eigen::VectorX<bool> element_mask;
    Eigen::VectorXd element_weights;

    Eigen::VectorXd bearings_rad;
    Eigen::VectorXd elevations_rad;

    Eigen::MatrixXd beampattern;

  } data;

  UdpBeamform2D();
  UdpBeamform2D(std::vector<int8_t> &buff);
  bool unpack_data(std::vector<int8_t> &buff);
  std::vector<int8_t> encode();
  Eigen::MatrixXd get_1D();
};

std::ostream &operator<<(std::ostream &os, const UdpBeamform2D &st);
std::ostringstream &operator<<(std::ostringstream &os, const UdpBeamform2D &st);

std::ostream &operator<<(std::ostream &os, const UdpBeamform2D::Header &st);
std::ostringstream &operator<<(std::ostringstream &os, const UdpBeamform2D::Header &st);

std::ostream &operator<<(std::ostream &os, const UdpBeamform2D::Payload &st);
std::ostringstream &operator<<(std::ostringstream &os, const UdpBeamform2D::Payload &st);

#endif
