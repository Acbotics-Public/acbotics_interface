/*******************************************************************/
/*    NAME: Oscar Viquez                                           */
/*    ORG:  Acbotics Research, LLC                                 */
/*    FILE: UdpData.h                                              */
/*    DATE: Apr 26th 2024                                          */
/*                                                                 */
/*    For help, contact us at: support@acbotics.com                */
/*******************************************************************/

#ifndef udp_data_HEADER
#define udp_data_HEADER

#include <gflags/gflags.h>

DECLARE_bool(debug_udp_data);

struct UdpData {
  struct __attribute__((__packed__)) Header {
    int64_t start_time_nsec;
    char id[2];
    int16_t num_bytes;

    Header() {
      this->start_time_nsec = -1;
      this->id[0] = '?';
      this->id[1] = '?';
      this->num_bytes = 0;
    };
    Header(std::vector<int8_t> &buff);
    void decode(std::vector<int8_t> &buff);
  } header;

  UdpData();
  UdpData(std::vector<int8_t> &buff);
  virtual bool unpack_data(std::vector<int8_t> &buff);
  virtual void csv_header(std::ostream &oss);
  virtual void csv_serialize(std::ostream &oss);

  void log_invalid_buffer(std::string &buff_start);
};

std::ostream &operator<<(std::ostream &os, const UdpData &st);
std::ostringstream &operator<<(std::ostringstream &os, const UdpData &st);

std::ostream &operator<<(std::ostream &os, const UdpData::Header &st);
std::ostringstream &operator<<(std::ostringstream &os, const UdpData::Header &st);

#endif
