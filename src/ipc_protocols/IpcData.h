/*******************************************************************/
/*    NAME: Oscar Viquez                                           */
/*    ORG:  Acbotics Research, LLC                                 */
/*    FILE: IpcData.h                                              */
/*    DATE: Feb 17th 2025                                          */
/*                                                                 */
/*    For help, contact us at: support@acbotics.com                */
/*******************************************************************/

#ifndef ipc_data_HEADER
#define ipc_data_HEADER

struct IpcData {
  struct __attribute__((__packed__)) Header {
    int64_t start_time_nsec;
    int32_t packet_num;

    Header() {
      this->start_time_nsec = -1;
      this->packet_num = 0;
    };
  } header;

  IpcData();
  virtual void csv_header(std::ostream &oss);
  virtual void csv_serialize(std::ostream &oss);
};

std::ostream &operator<<(std::ostream &os, const IpcData &st);
std::ostringstream &operator<<(std::ostringstream &os, const IpcData &st);

std::ostream &operator<<(std::ostream &os, const IpcData::Header &st);
std::ostringstream &operator<<(std::ostringstream &os, const IpcData::Header &st);

#endif
