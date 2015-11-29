// Copyright (c) 2015 Tony Kirke.  Boost Software License - Version 1.0  (http://www.opensource.org/licenses/BSL-1.0)
#include "fm_discriminator.h"
#include <cmath>
#define TWOM_PI 2*M_PI
double fm_discriminator::sample(std::complex<double> c) {
  double new_phase = 0;
  double delta_phase = 0;

  new_phase = arg(c);
  if (new_phase > M_PI) new_phase -= TWOM_PI;

  delta_phase = new_phase - phase;
  if (delta_phase > M_PI) delta_phase -= TWOM_PI;
  if (delta_phase < -M_PI) delta_phase += TWOM_PI;

  phase = new_phase;

  return (delta_phase);
}
void fm_discriminator::process(const std::vector<std::complex<double>>& samples_in,
                               std::vector<double>& samples_out)
{
  size_t n = samples_in.size();
  samples_out.resize(n);
  for (size_t i = 0; i < n; i++) {
    samples_out[i] = sample(samples_in[i]);
  }
}
