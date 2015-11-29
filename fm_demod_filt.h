#pragma once
// Copyright (c) 2015 Tony Kirke.  Boost Software License - Version 1.0  (http://www.opensource.org/licenses/BSL-1.0)

#include <cstdint>
#include <vector>

#include "fm_discriminator.h"
#include <spuce/filters/iir.h>
#include <spuce/filters/fir_decim.h>

//! FM demodulator that works at 4 times audio output rate
class fm_demod_filt
{
public:

  fm_demod_filt();
  void process(const std::vector<std::complex<double>>& samples_in,  std::vector<double>& audio);


private:
  void mono_to_interleaved_stereo(const std::vector<double>& samples_mono,  std::vector<double>& audio);
  fm_discriminator  m_phasedisc;
  
  spuce::fir<std::complex<double>>   m_filter;
  spuce::fir<double>   m_filter_mono;
  spuce::iir<double>   m_dcblock_mono;
  spuce::iir<double>   m_deemph_mono;
};

