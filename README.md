
  NFBM - Software decoder for narrow band FM with RTL-SDR


**This is for experimentation and not ready for general use**

 ---------------------------------------------------------------

NBFM is based on SoftFM which is a software-defined radio receiver for FM broadcast radio.
It is written in C++ and uses RTL-SDR to interface with RTL2832-based hardware.

Original Code has been modified

 * to use RTAudio instead of ALSA so it can work on Mac OSX
 * demonstrate parts of *spuce* library
 * modified to only do mono narrow-band FM demod
 * use hard-coded IF sampling rate of 1.152MHz
 * use hard-coded audio sample rate of 48kHz
 * Thus a fixed downsampling ratio of 24 is used to go from IF -> Audio
 * This is done in 1 stage using a spuce class 'decimator', which is then followed by a FM demod

  Installing
  ----------
  see instructions for SoftFM

  ----------
  License

  See files for Licensing. 

