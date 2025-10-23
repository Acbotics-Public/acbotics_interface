#include "utils/Logger_Acoustic.h"

Logger_Acoustic::Logger_Acoustic(std::string logger_dir) {
    this->file_initialized=false;
    this->running =false;
    this->num_channels=0;
    this->rollover_min=5;
    this->logger_outdir = logger_dir;
    this->sample_rate = 0;
    this->output_filename = "";
}

Logger_Acoustic::Logger_Acoustic() {
  Logger_Acoustic("/tmp/");
}


void Logger_Acoustic::Log_ACO_Data(std::shared_ptr<UdpAcousticData> aco_data){}

void Logger_Acoustic::StartFile(){}

void Logger_Acoustic::set_outdir(std::string logger_outdir)
{
  this->logger_outdir = logger_outdir;
}

void Logger_Acoustic::Start() {
  this->running = true;
  this->file_initialized = false;
}

void Logger_Acoustic::Stop() { this->running = false; }

void Logger_Acoustic::Initialize_from_aco(std::shared_ptr<UdpAcousticData> aco_data) {
  num_channels = aco_data->header.num_channels;
  sample_rate = int(aco_data->header.sample_rate);
}

void Logger_Acoustic_CSV::Initialize_from_aco(std::shared_ptr<UdpAcousticData> aco_data) {
  Logger_Acoustic::Initialize_from_aco(aco_data);
  csv_header << "packet_epoch_nsec,frame_tick_time_nsec,adc_count,packet_num,";
  for (int ii = 0; ii < num_channels - 1; ii++) {
    csv_header << ii << ",";
  }
  csv_header << num_channels - 1;
}
void Logger_Acoustic_FLAC::Initialize_from_aco(std::shared_ptr<UdpAcousticData> aco_data) {
  Logger_Acoustic::Initialize_from_aco(aco_data);

  num_files = std::ceil((double)num_channels / 8);
  for (int ii = 0; ii < num_files; ii++) {
    start_idx_per_file.push_back(8 * ii);
    ch_per_file.push_back(8);
  }
  if (num_channels % 8 > 0) {
    ch_per_file.at(ch_per_file.size() - 1) = num_channels % 8;
  }
}

void Logger_Acoustic_CSV::StartFile() {
  fntime.reset(this->rollover_min);
  this->output_filename = this->logger_outdir + "ACO_" + fntime.fname_str + ".csv";
  VLOG(5) << "=========== NEW FILE =========== ";
  LOG(INFO) << "Writing to file : " << this->output_filename;
  this->ofil = std::ofstream(this->output_filename);
  this->ofil << this->csv_header.str() << std::endl;
  this->file_initialized = true;                           

}

void Logger_Acoustic_WAV::StartFile() {
  fntime.reset(this->rollover_min);
  output_filename = this->logger_outdir + "ACO_" + fntime.fname_str + ".wav";
  VLOG(5) << "=========== NEW FILE =========== ";
  LOG(INFO) << "Writing to file : " << output_filename;

  ofil_wav = SndfileHandle(output_filename, SFM_WRITE, SF_FORMAT_WAV | SF_FORMAT_PCM_16,
                           num_channels, sample_rate);
  this->file_initialized = true;                           
}

void Logger_Acoustic_FLAC::StartFile() {
  fntime.reset(this->rollover_min);
  output_filenames.clear();
  output_files.clear();
  for (int ii = 0; ii < num_files; ii++) {
    output_filename =
        this->logger_outdir + "ACO_" + fntime.fname_str + "_" + std::to_string(ii) + ".flac";
    output_filenames.push_back(output_filename);

    output_files.push_back(SndfileHandle(output_filename, SFM_WRITE,
                                         SF_FORMAT_FLAC | SF_FORMAT_PCM_16, ch_per_file.at(ii),
                                         sample_rate));
  }
  VLOG(5) << "=========== NEW FILE(s) =========== ";
  LOG(INFO) << "Writing to files starting with : " << output_filenames.at(0);
  this->file_initialized = true;                           

}

void Logger_Acoustic::StopFile() {
}

void Logger_Acoustic_CSV::StopFile() {
    LOG(INFO) << "Closing file : " << output_filename;
    ofil.close();
}

void Logger_Acoustic_FLAC::StopFile() {

    for (int ff = 0; ff < num_files; ff++) {
      output_files.at(ff).writeSync();
      LOG(INFO) << "Closing file : " << output_filenames.at(ff);
    }
    output_filenames.clear();
    output_files.clear();
}

void Logger_Acoustic_WAV::StopFile() {
    ofil_wav.writeSync();
    LOG(INFO) << "Closing file : " << output_filename;
    ofil_wav = SndfileHandle();
}


void Logger_Acoustic_CSV::Log_ACO_Data(std::shared_ptr<UdpAcousticData> aco_data) {
  if (!this->running)
  {
    // we are not active. close out file if still open
    if (this->file_initialized)
    {
      this->StopFile();
      this->file_initialized=false;
    }
    return;
  }
  if (!ofil.is_open() || !this->file_initialized) {
    LOG(INFO) << "Starting CVS : " << ofil.is_open() << "  " << this->file_initialized;

    this->StartFile();
  }
  if (std::time(nullptr) > this->fntime.rollover_time) {
    this->StopFile();
    this->StartFile();
  }
  if (aco_data->data.size() > 0) {
    for (int ii = 0; ii < (aco_data->header.num_values / aco_data->header.num_channels); ii++) {
      ofil << std::fixed
           << aco_data->header.start_time_nsec //+
                  //(int64_t)(ii / (double)aco_data->header.sample_rate * 1e9)
           << ",";
      ofil << std::fixed
           << aco_data->header.tick_time_nsec //+
                  //(int64_t)(ii / (double)aco_data->header.sample_rate * 1e9)
           << ",";
      ofil << std::fixed << aco_data->header.adc_count + ii << ",";

      ofil << aco_data->header.packet_num << ",";
      for (int ch = 0; ch < num_channels - 1; ch++) {
        ofil << aco_data->data(ch, ii) << ",";
      }
      ofil << aco_data->data(num_channels - 1, ii) << std::endl;
    }
    ofil.flush();
  }
}
void Logger_Acoustic_FLAC::Log_ACO_Data(std::shared_ptr<UdpAcousticData> aco_data) {
  if (!this->running)
  {
    // we are not active. close out file if still open
    if (this->file_initialized)
    {
      this->StopFile();
      this->file_initialized=false;
    }
    return;
  }

  if (!this->file_initialized) {
    this->StartFile();
  }
  if (std::time(nullptr) > this->fntime.rollover_time) {
    this->StopFile();
    this->StartFile();
  }
  for (int ff = 0; ff < num_files; ff++) {
    buff_mat = aco_data->data.block(start_idx_per_file.at(ff), 0, ch_per_file.at(ff),
                                    aco_data->data.cols());
    size_t num_its = buff_mat.rows() * buff_mat.cols() * sizeof(aco_data->data(0));
    int16_t buffer[num_its] = {0};
    memcpy(&buffer, &buff_mat(0), num_its);
    output_files.at(ff).writef(buffer, buff_mat.cols());
  }
}

void Logger_Acoustic_WAV::Log_ACO_Data(std::shared_ptr<UdpAcousticData> aco_data) {
  if (!this->running)
  {
    // we are not active. close out file if still open
    if (this->file_initialized)
    {
      this->StopFile();
      this->file_initialized=false;
    }
    return;
  }

  if (!this->file_initialized) {
    this->StartFile();
  }
  if (std::time(nullptr) > this->fntime.rollover_time) {

    this->StopFile();
    this->StartFile();
  }
  size_t num_its = aco_data->data.rows() * aco_data->data.cols() * sizeof(aco_data->data(0));
  int16_t buffer[num_its] = {0};
  memcpy(&buffer, &aco_data->data(0), num_its);
  ofil_wav.writef(buffer, aco_data->data.cols());
}

