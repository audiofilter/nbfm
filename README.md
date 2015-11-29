
  NFBM - Software decoder for narrow band FM with RTL-SDR
 ---------------------------------------------------------------

NBFM is based on SoftFM which is a software-defined radio receiver for FM broadcast radio.
It is written in C++ and uses RTL-SDR to interface with RTL2832-based hardware.

Original Code has been modified

 * to use RTAudio instead of ALSA so it can work on Mac OSX
 * demonstrate parts of *spuce* library
 * modified to only do mono FM demod
 * use hard-coded IF sampling rate of 1.152MHz
 * use hard-coded audio sample rate of 48kHz
 * Thus a fixed downsampling ratio of 24 is used to go from IF -> Audio
 * This is done in 1 stage using a spuce class 'decimator'

nbfm provides:
 * real-time playback to soundcard or dumping to file
 * command-line interface (no GUI, no visualization, nothing fancy)

nbfm requires:
 * RtAudio
 * C++11
 * spuce http://github.com/audiofilter/spuce.git
 * RTL-SDR library (http://sdr.osmocom.org/trac/wiki/rtl-sdr)
 * supported DVB-T receiver
 * medium-strong FM radio signal

  Installing
  ----------

The Osmocom RTL-SDR library must be installed before you can build SoftFM.
See http://sdr.osmocom.org/trac/wiki/rtl-sdr for more information.
SoftFM has been tested successfully with RTL-SDR 0.5.3.

To install nbfm, install dependencies (see .travis.yml), download and unpack the source code and go to the
top level directory. Then do like this:

```sh
mkdir build
cd build
cmake ..
make
./nbfm -f <radio-frequency-in-MHz>
```

If you have difficulties, see instructions for SoftFM

--

License

See files for Licensing. If no License, info, then code is forked code from SoftFM

--
