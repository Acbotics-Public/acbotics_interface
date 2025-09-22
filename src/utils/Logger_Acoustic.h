#ifndef LOGGER_ACOUSTIC_HEADER
#define LOGGER_ACOUSTIC_HEADER
#include "udp_protocols/UdpAcousticData.h"
#include "utils/Types.h"
#include <ctime>
#include <fstream>
#include <glog/logging.h>
#include <memory>
#include <sndfile.h>
#include <sndfile.hh>
#include <time.h>

class Logger_Acoustic {
public:
  Logger_Acoustic(std::string logger_dir);
  Logger_Acoustic();

  void Start();
  virtual void Stop();
  virtual void Set();

  virtual void Log_ACO_Data(std::shared_ptr<UdpAcousticData> aco_data);
  virtual void Initialize_from_aco(std::shared_ptr<UdpAcousticData> aco_data);
  virtual void StartFile();

  struct FilenameTime {
    std::time_t fname_time;
    std::time_t rollover_time;
    std::string fname_str;

    FilenameTime() : FilenameTime(-1) {}
    FilenameTime(int rollover_min);
    void reset(int rollover_min);

  protected:
    int rollover_min_default = 5;
  };

protected:
  bool running;
  bool file_initialized;
  int8_t num_channels;
  FilenameTime fntime;
  float rollover_min = 5;
  std::string logger_outdir;
  int sample_rate;
  std::string output_filename;
  Eigen::MatrixX<int16_t> buff_mat;
};

class Logger_Acoustic_CSV : public Logger_Acoustic {
public:
  void Log_ACO_Data(std::shared_ptr<UdpAcousticData> aco_data) override;
  void StartFile() override;
  void Initialize_from_aco(std::shared_ptr<UdpAcousticData> aco_data) override;
  //   Logger_Acoustic_CSV(std::string logger_dir);
protected:
  std::ofstream ofil;
  std::ostringstream csv_header;
};

class Logger_Acoustic_WAV : public Logger_Acoustic {
public:
  void Log_ACO_Data(std::shared_ptr<UdpAcousticData> aco_data) override;
  void StartFile() override;

protected:
  SndfileHandle ofil_wav;
};

class Logger_Acoustic_FLAC : public Logger_Acoustic {
public:
  void Log_ACO_Data(std::shared_ptr<UdpAcousticData> aco_data) override;
  void StartFile() override;
  void Initialize_from_aco(std::shared_ptr<UdpAcousticData> aco_data) override;

protected:
  int num_files;
  std::vector<int> start_idx_per_file;
  std::vector<int> ch_per_file;
  std::vector<std::string> output_filenames;
  std::vector<SndfileHandle> output_files;
};
#endif