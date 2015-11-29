#ifndef SOFTFM_AUDIOOUTPUT_H
#define SOFTFM_AUDIOOUTPUT_H
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
#include <cstdio>
#include <string>
#include <vector>

#include <RtAudio.h>
#include "fifo.h"

/** Base class for writing audio data to file or playback. */
class AudioOutput
{
public:

    /** Destructor. */
    virtual ~AudioOutput() { }

    /**
     * Write audio data.
     *
     * Return true on success.
     * Return false if an error occurs.
     */
    virtual bool write(const std::vector<double>& samples) = 0;

    /** Return the last error, or return an empty string if there is no error. */
    std::string error()
    {
        std::string ret(m_error);
        m_error.clear();
        return ret;
    }

    /** Return true if the stream is OK, return false if there is an error. */
    operator bool() const
    {
        return (!m_zombie) && m_error.empty();
    }

protected:
    /** Constructor. */
    AudioOutput() : m_zombie(false) { }

    /** Encode a list of samples as signed 16-bit little-endian integers. */
    static void samplesToInt16(const std::vector<double>& samples,
                               std::vector<std::uint8_t>& bytes);

    std::string m_error;
    bool        m_zombie;

private:
    AudioOutput(const AudioOutput&);            // no copy constructor
    AudioOutput& operator=(const AudioOutput&); // no assignment operator
};


/** Write audio data as raw signed 16-bit little-endian data. */
class RawAudioOutput : public AudioOutput
{
public:

    /**
     * Construct raw audio writer.
     *
     * filename :: file name (including path) or "-" to write to stdout
     */
    RawAudioOutput(const std::string& filename);

    ~RawAudioOutput();
    bool write(const std::vector<double>& samples);

private:
    int m_fd;
    std::vector<std::uint8_t> m_bytebuf;
};


/** Write audio data as .WAV file. */
class WavAudioOutput : public AudioOutput
{
public:

    /**
     * Construct .WAV writer.
     *
     * filename     :: file name (including path) or "-" to write to stdout
     * samplerate   :: audio sample rate in Hz
     * stereo       :: true if the output stream contains stereo data
     */
    WavAudioOutput(const std::string& filename,
                   unsigned int samplerate,
                   bool stereo);

    ~WavAudioOutput();
    bool write(const std::vector<double>& samples);

private:

    /** (Re-)Write .WAV header. */
    bool write_header(unsigned int nsamples);

    static void encode_chunk_id(std::uint8_t * ptr, const char * chunkname);

    template <typename T>
    static void set_value(std::uint8_t * ptr, T value);

    const unsigned numberOfChannels;
    const unsigned sampleRate;
    std::FILE *m_stream;
    std::vector<std::uint8_t> m_bytebuf;
};


/** Write audio data to ALSA device. */
class StreamAudioOutput : public AudioOutput
{
public:

    /**
     * Construct ALSA output stream.
     *
     * dename       :: ALSA PCM device
     * samplerate   :: audio sample rate in Hz
     * stereo       :: true if the output stream contains stereo data
     */
    StreamAudioOutput(const std::string& devname,
                    unsigned int samplerate,
                    bool stereo);

    ~StreamAudioOutput();
    bool write(const std::vector<double>& samples);

private:
    unsigned int         m_nchannels;
#ifdef __APPLE__
   	RtAudio dac;
	  RtAudio::StreamParameters m_rt_params;
#else
    std::vector<std::uint8_t> m_bytebuf;
    struct _snd_pcm *    m_pcm;
#endif
};

#endif
