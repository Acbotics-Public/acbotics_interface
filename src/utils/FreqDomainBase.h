/*******************************************************************/
/*    NAME: Oscar Viquez                                           */
/*    ORG:  Acbotics Research, LLC                                 */
/*    FILE: FreqDomainBase.h                                       */
/*    DATE: Feb 18th 2025                                          */
/*                                                                 */
/*    For help, contact us at: support@acbotics.com                */
/*******************************************************************/

#ifndef freq_domain_base_HEADER
#define freq_domain_base_HEADER

// includes from within project
#include "utils/QueueClient.h"

class FreqDomainBase : virtual public QueueClient {
public:
  FreqDomainBase() : QueueClient() {

    this->sample_rate = 0;

    this->NFFT = 1024;
    this->noverlap = this->NFFT / 2;
    this->nstep = this->NFFT - this->noverlap;
    this->nfreq = this->NFFT / 2 + 1;

    this->frequency_vector = Eigen::VectorXd::LinSpaced(this->nfreq, 0, 0.5);
    this->frequency_weights = Eigen::VectorXd::Zero(nfreq);

    // assume thread will need update from defaults
    this->_rx_runtime_update = true;
  };

  void set_sample_rate(double sample_rate);
  void set_NFFT(size_t NFFT);
  void set_noverlap(size_t noverlap);

  double get_sample_rate();
  size_t get_NFFT();
  size_t get_noverlap();
  size_t get_nstep();

  Eigen::VectorXd get_frequencies();
  Eigen::VectorXd get_frequencies_active();

  void add_frequency_band_min_max(double f_low, double f_high);
  void add_frequency_band_center(double f_center, double b_width);

  void add_frequency_bin_closest(double f_center);

  void clear_frequency_bands();
  void reset_frequency_band_min_max(double f_low, double f_high);
  void reset_frequency_band_center(double f_center, double b_width);
  static Eigen::ArrayXXd get_hann(size_t num_ch, size_t NFFT);

protected:
  double sample_rate;

  size_t NFFT;
  size_t noverlap;
  size_t nstep;
  size_t nfreq;
  bool _rx_runtime_update;

  std::vector<std::pair<double, double>> frequency_bands;

  Eigen::VectorXd frequency_vector;
  Eigen::VectorXd frequency_weights;

  std::vector<int> active_frequencies;

  void recompute_frequencies();
};

#endif
