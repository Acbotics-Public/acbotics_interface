/*******************************************************************/
/*    NAME: Oscar Viquez                                           */
/*    ORG:  Acbotics Research, LLC                                 */
/*    FILE: FreqDomainBase.cpp                                     */
/*    DATE: Feb 18th 2025                                          */
/*                                                                 */
/*    For help, contact us at: support@acbotics.com                */
/*******************************************************************/

#include <glog/logging.h>

// includes from within project
#include "utils/FreqDomainBase.h"

void FreqDomainBase::set_sample_rate(double sample_rate) {
  if (this->sample_rate != sample_rate) {
    LOG(INFO) << this->thread_name << " :: Updating sampling rate: " << this->sample_rate << " to "
              << sample_rate;
    this->sample_rate = sample_rate;
    recompute_frequencies();
  }
}

void FreqDomainBase::set_NFFT(size_t NFFT) {
  if (this->NFFT != NFFT) {
    LOG(INFO) << this->thread_name << " :: Updating NFFT: " << this->NFFT << " to " << NFFT;
    this->NFFT = NFFT;
    this->noverlap = NFFT / 2;
    this->nstep = this->NFFT - this->noverlap;
    this->nfreq = this->NFFT / 2 + 1;
    recompute_frequencies();
  }
}

void FreqDomainBase::set_noverlap(size_t noverlap) {
  if (noverlap < this->NFFT) {
    LOG(INFO) << this->thread_name << " :: Updating noverlap: " << this->noverlap << " to "
              << noverlap;
    this->noverlap = noverlap;
    this->nstep = this->NFFT - this->noverlap;
    recompute_frequencies();
  } else
    LOG(WARNING) << this->thread_name << " :: The value of noverlap must be smaller than NFFT!";
}

double FreqDomainBase::get_sample_rate() { return this->sample_rate; }

size_t FreqDomainBase::get_NFFT() { return this->NFFT; }

size_t FreqDomainBase::get_noverlap() { return this->noverlap; }

size_t FreqDomainBase::get_nstep() { return this->NFFT - this->noverlap; }

Eigen::VectorXd FreqDomainBase::get_frequencies() { return this->frequency_vector; }

Eigen::VectorXd FreqDomainBase::get_frequencies_active() {
  return this->frequency_vector(this->active_frequencies);
}

void FreqDomainBase::add_frequency_band_min_max(double f_low, double f_high) {
  if (f_low < f_high) {
    VLOG(1) << this->thread_name << " :: Adding frequency bounds: " << f_low
            << " <= f <= " << f_high;
    this->frequency_bands.push_back({f_low, f_high});
    recompute_frequencies();
  }
}

void FreqDomainBase::add_frequency_band_center(double f_center, double b_width) {
  if (f_center > 0) {
    double f_low = f_center - b_width / 2;
    double f_high = f_center + b_width / 2;
    VLOG(1) << this->thread_name << " :: Adding frequency bounds: " << f_low
            << " <= f <= " << f_high;
    this->frequency_bands.push_back({f_low > 0 ? f_low : 0, f_high});
    recompute_frequencies();
  }
}

void FreqDomainBase::add_frequency_bin_closest(double f_center) {
  if (f_center > 0) {

    Eigen::ArrayXd v_dist = (this->frequency_vector.array() - f_center).abs();
    double min_dist = v_dist.minCoeff();
    int idx = 0;
    for (int ii = 0; ii < v_dist.size(); ii++) {
      if (v_dist[ii] == min_dist) {
        idx = ii;
        break;
      }
    }
    double f_closest = this->frequency_vector[idx];

    VLOG(1) << this->thread_name << " :: Adding frequency bin: " << f_closest;
    this->frequency_bands.push_back({f_closest > 0 ? f_closest : 0, f_closest});
    recompute_frequencies();
  }
}

void FreqDomainBase::recompute_frequencies() {

  this->frequency_vector =
      Eigen::VectorXd::LinSpaced(this->NFFT / 2 + 1, 0, 0.5) * this->sample_rate;

  this->frequency_weights = Eigen::VectorXd::Zero(this->frequency_vector.size());
  Eigen::VectorXd new_weights = Eigen::VectorXd::Zero(this->frequency_vector.size());

  for (std::pair<double, double> band : this->frequency_bands) {
    new_weights = (((this->frequency_vector.array() >= band.first) &&
                    (this->frequency_vector.array() <= band.second)) ||
                   this->frequency_weights.array() > 0)
                      .cast<double>();
    this->frequency_weights = new_weights;
  }

  this->frequency_weights /= this->frequency_weights.sum();

  this->active_frequencies.clear();
  for (int ff = 0; ff < this->nfreq; ff++) {
    if (this->frequency_weights[ff] > 0) {
      this->active_frequencies.push_back(ff);
    }
  }

  VLOG(3) << this->thread_name << " :: Active frequency bins: " << this->active_frequencies.size();

  this->_rx_runtime_update = true;
}

void FreqDomainBase::clear_frequency_bands() {
  this->frequency_weights = Eigen::VectorXd::Zero(nfreq);
  this->frequency_bands.clear();
}

void FreqDomainBase::reset_frequency_band_min_max(double f_low, double f_high) {
  clear_frequency_bands();
  add_frequency_band_min_max(f_low, f_high);
}
void FreqDomainBase::reset_frequency_band_center(double f_center, double b_width) {
  clear_frequency_bands();
  add_frequency_band_center(f_center, b_width);
}

Eigen::ArrayXXd FreqDomainBase::get_hann(size_t num_ch, size_t NFFT) {
  Eigen::ArrayXXd win;
  Eigen::VectorXd col;
  Eigen::VectorXd len;

  col = Eigen::VectorXd::Ones(num_ch);
  len = Eigen::VectorXd::LinSpaced(NFFT, 0, NFFT - 1);
  win = 0.5 - 0.5 * (col * 2 * M_PI * len.transpose() / (NFFT - 1)).array().cos();

  return win;
}
