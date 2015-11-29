
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

MonoFM provides:
 * real-time playback to soundcard or dumping to file
 * command-line interface (no GUI, no visualization, nothing fancy)

MonoFM requires:
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

To install MonoFM, install dependencies (see .travis.yml), download and unpack the source code and go to the
top level directory. Then do like this:

```sh
mkdir build
cd build
cmake ..
make
./nbfm -f <radio-frequency-in-MHz>
```

CMake tries to find librtlsdr. If this fails, you need to specify the location of the library in one the following ways:

```sh
cmake .. -DCMAKE_INSTALL_PREFIX=/path/rtlsdr
cmake .. -DRTLSDR_INCLUDE_DIR=/path/rtlsdr/include -DRTLSDR_LIBRARY_PATH=/path/rtlsdr/lib/librtlsdr.a
PKG_CONFIG_PATH=/path/rtlsdr/lib/pkgconfig cmake ..
```

--

License

See files for Licensing. If no License, info, then code is forked code from SoftFM
, copyright (C) 2013, Joris van Rantwijk

This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License along
with this program; if not, see http://www.gnu.org/licenses/gpl-2.0.html

--
