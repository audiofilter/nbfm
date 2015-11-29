// Copyright (c) 2015 Tony Kirke.  Boost Software License - Version 1.0  (http://www.opensource.org/licenses/BSL-1.0)
#include <cassert>
#include <cmath>
#include <complex>
#include <spuce/filters/design_iir.h>
#include <spuce/filters/design_fir.h>
#include "fm_demod_filt.h"

using namespace std;
using namespace spuce;

fm_demod_filt::fm_demod_filt()
{
  const double sample_rate_pcm = 48000;
  
  static constexpr double deemphasis = 50;
	iir_coeff *LPF = design_iir("butterworth", "LOW_PASS", 1,
                              1.0/(M_PI*deemphasis * sample_rate_pcm * 1.0e-6));
  
	m_deemph_mono.realloc(*LPF);

	iir_coeff *HPF = design_iir("butterworth", "HIGH_PASS", 2, 30.0/sample_rate_pcm);
	m_dcblock_mono.realloc(*HPF);
  
	// 4
	int rate = 4;
	auto taps = design_fir("remez","LOW_PASS", 4*rate+1, 0.125, 0, 0.1, 100.0);
	m_filter.set_taps(taps);
  // Same for audio filter after discrimintor
  m_filter_mono.set_taps(taps);

}

void fm_demod_filt::process(const std::vector<std::complex<double>>& samples_in,
                        std::vector<double>& audio)
{
  std::vector<std::complex<double>> filt_samples;
  filt_samples.resize(samples_in.size());

  m_filter.process(samples_in, filt_samples);

  std::vector<double> buf_mono;
  buf_mono.resize(filt_samples.size());

  m_phasedisc.process(filt_samples, buf_mono);
  
  // DC blocking and de-emphasis.
  m_dcblock_mono.process_inplace(buf_mono);
  m_deemph_mono.process_inplace(buf_mono);
  m_filter_mono.process_inplace(buf_mono);
	
  mono_to_interleaved_stereo(buf_mono, audio);
}

// Duplicate mono signal in left/right channels.
void fm_demod_filt::mono_to_interleaved_stereo(const std::vector<double>& samples_mono,
                                               std::vector<double>& audio)
{
  unsigned int n = samples_mono.size();
  audio.resize(2*n);
  for (unsigned int i = 0; i < n; i++) {
    double m = samples_mono[i];
    audio[2*i]   = m;
    audio[2*i+1] = m;
  }
}


