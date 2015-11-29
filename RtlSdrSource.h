#pragma once
/*
 *  Audio output handling for SoftFM
 *
 *  Copyright (C) 2013, Joris van Rantwijk.
 *
 *  .WAV file writing by Sidney Cadot,
 *  adapted for SoftFM by Joris van Rantwijk.
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License along
 *  with this program; if not, see http://www.gnu.org/licenses/gpl-2.0.html
 */

#include <cstdint>
#include <string>
#include <vector>
#include <complex>

class RtlSdrSource
{
public:

    static const int default_block_length = 65536;

    /** Open RTL-SDR device. */
    RtlSdrSource(int dev_index);

    /** Close RTL-SDR device. */
    ~RtlSdrSource();

    /**
     * Configure RTL-SDR tuner and prepare for streaming.
     *
     * sample_rate  :: desired sample rate in Hz.
     * frequency    :: desired center frequency in Hz.
     * tuner_gain   :: desired tuner gain in 0.1 dB, or INT_MIN for auto-gain.
     * block_length :: preferred number of samples per block.
     *
     * Return true for success, false if an error occurred.
     */
    bool configure(std::uint32_t sample_rate,
                   std::uint32_t frequency,
                   int tuner_gain,
                   int block_length=default_block_length,
                   bool agcmode=false);

    /** Return current sample frequency in Hz. */
    std::uint32_t get_sample_rate();

    /** Return current center frequency in Hz. */
    std::uint32_t get_frequency();

    /** Return current tuner gain in units of 0.1 dB. */
    int get_tuner_gain();

    /** Return a list of supported tuner gain settings in units of 0.1 dB. */
    std::vector<int> get_tuner_gains();

    /** Return name of opened RTL-SDR device. */
    std::string get_device_name() const
    {
        return m_devname;
    }

    /**
     * Fetch a bunch of samples from the device.
     *
     * This function must be called regularly to maintain streaming.
     * Return true for success, false if an error occurred.
     */
    bool get_samples(std::vector<std::complex<double>>& samples);

    /** Return the last error, or return an empty string if there is no error. */
    std::string error()
    {
        std::string ret(m_error);
        m_error.clear();
        return ret;
    }

    /** Return true if the device is OK, return false if there is an error. */
    operator bool() const
    {
        return m_dev && m_error.empty();
    }

    /** Return a list of supported devices. */
    static std::vector<std::string> get_device_names();

private:
    struct rtlsdr_dev * m_dev;
    int                 m_block_length;
    std::string         m_devname;
    std::string         m_error;
};

