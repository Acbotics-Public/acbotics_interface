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

  void run();
  void register_client(QueueClient &client);

  void set_adc_scale(double new_adc_scale);

  void set_channel_filter(int num_ch);
  void set_channel_filter(std::vector<int> channel_filter);

  bool use_channel_filter;
  std::vector<int> channel_filter;

protected:
  static void *_run_fft_thread(void *arg);

  size_t num_channels;

  double adc_scale;
};

#endif
