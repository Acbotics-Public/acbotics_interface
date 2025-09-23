#include <fstream>
#include <glog/logging.h>
#include <pthread.h>

// used for logging to audio files -- i.e. WAV, FLAC
#include <sndfile.h>
#include <sndfile.hh>

// includes from within project
#include "utils/LoggerBlock.h"

void *LoggerBlock::_run_logger_thread_audio(void *ptr)
{
  LoggerBlock *argPtr = static_cast<LoggerBlock *>(ptr);
  argPtr->run_log_thread_audio();
  pthread_exit(NULL);

}

void LoggerBlock::set_outdir(std::string logger_outdir)
{
  this->csv_logger.set_outdir(logger_outdir);
this->wav_logger.set_outdir(logger_outdir);
this->flac_logger.set_outdir(logger_outdir);
}

void LoggerBlock::start_logging(LOGGER logger){
{
  if (logger == LOGGER::ACO_CSV)
  {
    this->csv_logger.Start();

  }
  else if (logger == LOGGER::ACO_FLAC)
  {
    this->flac_logger.Start();    
  }
  else if (logger == LOGGER::ACO_WAV)
  {
    this->wav_logger.Start();
  }  
}

}
void LoggerBlock::stop_logging(LOGGER logger){
  if (logger == LOGGER::ACO_CSV)
  {
    this->csv_logger.Stop();

  }
  else if (logger == LOGGER::ACO_FLAC)
  {
    this->flac_logger.Stop();    
  }
  else if (logger == LOGGER::ACO_WAV)
  {
    this->wav_logger.Stop();
  }  
}



void LoggerBlock::run_threads()
{
  pthread_create(&_thread, NULL, _run_logger_thread_audio, this);
}
void LoggerBlock::stop_threads(){}


void LoggerBlock::run_log_thread_audio() {
  prctl(PR_SET_NAME, "ac_log_aco");
  VLOG(3) << "Starting audio logger in thread " << pthread_self();

  std::vector<std::shared_ptr<UdpAcousticData>> aco_data_vec;
  std::shared_ptr<UdpAcousticData> aco_data;

  while (this->keep_alive && !initialized) {
    if (this->q_aco->size() > 0) {
      // custom queue's front() peeks into the first element without popping
      aco_data = this->q_aco->front();
      csv_logger.Initialize_from_aco(aco_data);
      flac_logger.Initialize_from_aco(aco_data);
      wav_logger.Initialize_from_aco(aco_data);

      initialized = true;
    } else {
      usleep(10000);
    }
  }

  while (this->keep_alive) {
    if (true) {
      while (this->keep_alive) {
        aco_data_vec = this->q_aco->pop_all();
        // aco_data = argPtr->q_aco_dup["csv"]->pop();
        for (int iac = 0; iac < aco_data_vec.size(); iac++) {
          aco_data = aco_data_vec.at(iac);
            this->csv_logger.Log_ACO_Data(aco_data);
            this->wav_logger.Log_ACO_Data(aco_data);
            this->flac_logger.Log_ACO_Data(aco_data);
          }
        // rest here, to allow for external control switch
        usleep(10000);
      }
    }
     else {
      // if (argPtr->q_aco_dup["csv"]->size() > 0)
      this->q_aco->clear();
      usleep(100000);
     }
  }

  this->csv_logger.Stop();
  this->flac_logger.Stop();
  this->wav_logger.Stop();
}
