#pragma once
// Copyright (c) 2015 Tony Kirke. License MIT  (http://www.opensource.org/licenses/mit-license.php)
#include <complex>
#include <vector>
class fm_discriminator {
public:
  double phase;

  fm_discriminator() { phase = 0; }

  //! \brief FM Demodulate a signal
  double sample(std::complex<double> in);
  void process(const std::vector<std::complex<double>>& samples_in,
               std::vector<double>& samples_out);

};
