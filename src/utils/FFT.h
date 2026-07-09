/*******************************************************************/
/*    NAME: Oscar Viquez                                           */
/*    ORG:  Acbotics Research, LLC                                 */
/*    FILE: FFT.h                                                  */
/*    DATE: May 21th 2024                                          */
/*                                                                 */
/*    For help, contact us at: support@acbotics.com                */
/*******************************************************************/

#ifndef fft_HEADER
#define fft_HEADER

#include <Eigen/Dense>
#include <iostream>
#include <pthread.h>

// includes from within project
#include "utils/FreqDomainBase.h"
// #include "utils/UdpSocketIn.h"

DECLARE_bool(debug_fft);

class FFT : public FreqDomainBase {
public:
  std::vector<std::shared_ptr<tsQueue<std::shared_ptr<IpcFFT>>>> v_q_fft;
  FFT() : FreqDomainBase() {
    this->adc_scale = 2.5;

    this->thread_name = "fft_thr";
    this->use_channel_filter = false;
  };
  static std::shared_ptr<FFT> create()
  {
    return std::make_shared<FFT>();
  }

  void run() override;
  void register_client(QueueClient &client);
  void register_client(std::shared_ptr<tsQueue<std::shared_ptr<IpcFFT>>> q_fft);
  void set_adc_scale(double new_adc_scale);

  std::shared_ptr<tsQueue<std::shared_ptr<UdpAcousticData>>>get_input_queue()
  {
    return this->q_aco;
  };

  void set_channel_filter(int num_ch);
  void set_channel_filter(std::vector<int> channel_filter);

  bool use_channel_filter;
  std::vector<int> channel_filter;

protected:
  static void *_run_fft_thread(void *arg);
  void run_fft_thread(void);
  size_t num_channels;

  double adc_scale;
};

#endif
