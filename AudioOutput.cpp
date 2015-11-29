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

#define _FILE_OFFSET_BITS 64

#include <unistd.h>
#include <fcntl.h>
#include <cstdio>
#include <cstring>
#include <cerrno>
#include <algorithm>
#include <cassert>
#include <cmath>

#ifdef __APPLE__
#include <RtAudio.h>
#else
#include <alsa/asoundlib.h>
#endif

#include "AudioOutput.h"

using namespace std;

#ifdef __APPLE__
void errorCallback( RtAudioError::Type type, const std::string &errorText )
{
  // This example error handling function does exactly the same thing
  // as the embedded RtAudio::error() function.
  std::cout << "in errorCallback" << std::endl;
  if ( type == RtAudioError::WARNING )
    std::cerr << '\n' << errorText << "\n\n";
  else if ( type != RtAudioError::WARNING )
    throw( RtAudioError( errorText, type ) );
}

typedef signed short MY_TYPE;
unsigned channels=2;

#ifdef __APPLE__
unsigned bufferFrames = 512;
const int NUM_SECS = 4;
const int WRAP_SIZE = NUM_SECS * 48000 * 2;
static int begin_read = 0;
static int max_buffer_level = 0;
static int min_buffer_level = 0;
fifo<int16_t>   non_m_fifo(WRAP_SIZE);
#endif

int audio_callback( void *outputBuffer, void * /*inputBuffer*/, unsigned int nBufferFrames,
         double /*streamTime*/, RtAudioStreamStatus status, void *data )
{
  unsigned int i, j;
  extern unsigned int channels;
  MY_TYPE *buffer = (MY_TYPE *) outputBuffer;
	fifo<std::uint16_t>* Vals = 	reinterpret_cast<fifo<std::uint16_t>*>(data);
  if ( status )
    std::cout << "Stream underflow detected!" << std::endl;

	int fill = Vals->fill_amount();
	
	if (Vals->half_full()) {
		if (begin_read == 0) {
			min_buffer_level = fill;
			max_buffer_level = fill;
		}
		begin_read = 1;
	}

	if (fill > max_buffer_level) {
		max_buffer_level = fill;
		if (min_buffer_level/(float)WRAP_SIZE  > 0.75)
			std::cout << "New max level = " << max_buffer_level/(float)WRAP_SIZE << "\n";
	}
	if (begin_read && fill < min_buffer_level) {
		min_buffer_level = fill;
		if (min_buffer_level/(float)WRAP_SIZE  < 0.25)
			std::cout << "New min level = " << min_buffer_level/(float)WRAP_SIZE << "\n";
	}

	MY_TYPE last;
	for ( j=0; j<channels; j++ ) {
		for ( i=0; i<nBufferFrames; i++ ) {
			if (begin_read) {
				if ( ((i%256)==0) && ((min_buffer_level/(float)WRAP_SIZE) < 0.25) ) {
					*buffer++ = 0;
				} else {
					last = (MY_TYPE)(Vals->read());
					*buffer++ = last;
				}
			} else {
				*buffer++ = 0;
			}
		}
	}

  return 0;
}


#endif

/* ****************  class AudioOutput  **************** */

// Encode a list of samples as signed 16-bit little-endian integers.
void AudioOutput::samplesToInt16(const std::vector<double>& samples,
                                 std::vector<uint8_t>& bytes)
{
    bytes.resize(2 * samples.size());

    std::vector<double>::const_iterator i = samples.begin();
    std::vector<double>::const_iterator n = samples.end();
    std::vector<uint8_t>::iterator k = bytes.begin();

    while (i != n) {
      double s = *(i++);
      s = max(-1.0, min(1.0, s));
      long v = lrint(s * 32767);
      unsigned long u = v;
      *(k++) = u & 0xff;
      *(k++) = (u >> 8) & 0xff;
    }
}


/* ****************  class RawAudioOutput  **************** */

// Construct raw audio writer.
RawAudioOutput::RawAudioOutput(const string& filename)
{
    if (filename == "-") {

        m_fd = STDOUT_FILENO;

    } else {

        m_fd = open(filename.c_str(), O_WRONLY | O_CREAT | O_TRUNC, 0666);
        if (m_fd < 0) {
            m_error  = "can not open '" + filename + "' (" +
                       strerror(errno) + ")";
            m_zombie = true;
            return;
        }

    }
}


// Destructor.
RawAudioOutput::~RawAudioOutput()
{
    // Close file descriptor.
    if (m_fd >= 0 && m_fd != STDOUT_FILENO) {
        close(m_fd);
    }
}


// Write audio data.
bool RawAudioOutput::write(const std::vector<double>& samples)
{
    if (m_fd < 0)
        return false;

    // Convert samples to bytes.
    samplesToInt16(samples, m_bytebuf);

    // Write data.
    size_t p = 0;
    size_t n = m_bytebuf.size();
    while (p < n) {

        ssize_t k = ::write(m_fd, m_bytebuf.data() + p, n - p);
        if (k <= 0) {
            if (k == 0 || errno != EINTR) {
                m_error = "write failed (";
                m_error += strerror(errno);
                m_error += ")";
                return false;
            }
        } else {
            p += k;
        }
    }

    return true;
}


/* ****************  class WavAudioOutput  **************** */

// Construct .WAV writer.
WavAudioOutput::WavAudioOutput(const std::string& filename,
                               unsigned int samplerate,
                               bool stereo)
  : numberOfChannels(stereo ? 2 : 1)
  , sampleRate(samplerate)
{
    m_stream = fopen(filename.c_str(), "wb");
    if (m_stream == NULL) {
        m_error  = "can not open '" + filename + "' (" +
                   strerror(errno) + ")";
        m_zombie = true;
        return;
    }

    // Write initial header with a dummy sample count.
    // This will be replaced with the actual header once the WavFile is closed.
    if (!write_header(0x7fff0000)) {
        m_error = "can not write to '" + filename + "' (" +
                  strerror(errno) + ")";
        m_zombie = true;
    }
}


// Destructor.
WavAudioOutput::~WavAudioOutput()
{
    // We need to go back and fill in the header ...

    if (!m_zombie) {

        const unsigned bytesPerSample = 2;

        const long currentPosition = ftell(m_stream);

        assert((currentPosition - 44) % bytesPerSample == 0);

        const unsigned totalNumberOfSamples = (currentPosition - 44) / bytesPerSample;

        assert(totalNumberOfSamples % numberOfChannels == 0);

        // Put header in front

        if (fseek(m_stream, 0, SEEK_SET) == 0) {
            write_header(totalNumberOfSamples);
        }
    }

    // Done writing the file

    if (m_stream) {
        fclose(m_stream);
    }
}


// Write audio data.
bool WavAudioOutput::write(const std::vector<double>& samples)
{
    if (m_zombie)
        return false;

    // Convert samples to bytes.
    samplesToInt16(samples, m_bytebuf);

    // Write samples to file.
    size_t k = fwrite(m_bytebuf.data(), 1, m_bytebuf.size(), m_stream);
    if (k != m_bytebuf.size()) {
        m_error = "write failed (";
        m_error += strerror(errno);
        m_error += ")";
        return false;
    }

    return true;
}


// (Re)write .WAV header.
bool WavAudioOutput::write_header(unsigned int nsamples)
{
    const unsigned bytesPerSample = 2;
    const unsigned bitsPerSample  = 16;

    enum wFormatTagId
    {
        WAVE_FORMAT_PCM        = 0x0001,
        WAVE_FORMAT_IEEE_FLOAT = 0x0003
    };

    assert(nsamples % numberOfChannels == 0);

    // synthesize header

    uint8_t wavHeader[44];

    encode_chunk_id    (wavHeader +  0, "RIFF");
    set_value<uint32_t>(wavHeader +  4, 36 + nsamples * bytesPerSample);
    encode_chunk_id    (wavHeader +  8, "WAVE");
    encode_chunk_id    (wavHeader + 12, "fmt ");
    set_value<uint32_t>(wavHeader + 16, 16);
    set_value<uint16_t>(wavHeader + 20, WAVE_FORMAT_PCM);
    set_value<uint16_t>(wavHeader + 22, numberOfChannels);
    set_value<uint32_t>(wavHeader + 24, sampleRate                                    ); // sample rate
    set_value<uint32_t>(wavHeader + 28, sampleRate * numberOfChannels * bytesPerSample); // byte rate
    set_value<uint16_t>(wavHeader + 32,              numberOfChannels * bytesPerSample); // block size
    set_value<uint16_t>(wavHeader + 34, bitsPerSample);
    encode_chunk_id    (wavHeader + 36, "data");
    set_value<uint32_t>(wavHeader + 40, nsamples * bytesPerSample);

    return fwrite(wavHeader, 1, 44, m_stream) == 44;
}


void WavAudioOutput::encode_chunk_id(uint8_t * ptr, const char * chunkname)
{
    for (unsigned i = 0; i < 4; ++i)
    {
        assert(chunkname[i] != '\0');
        ptr[i] = chunkname[i];
    }
    assert(chunkname[4] == '\0');
}


template <typename T>
void WavAudioOutput::set_value(uint8_t * ptr, T value)
{
    for (size_t i = 0; i < sizeof(T); ++i)
    {
        ptr[i] = value & 0xff;
        value >>= 8;
    }
}


/* ****************  class StreamAudioOutput  **************** */

// Construct output stream.
StreamAudioOutput::StreamAudioOutput(const std::string& devname,
																		 unsigned int samplerate,
																		 bool stereo)
{
    m_nchannels = stereo ? 2 : 1;

#ifndef __APPLE__		
    m_pcm = NULL;
    int r = snd_pcm_open(&m_pcm, devname.c_str(),
                         SND_PCM_STREAM_PLAYBACK, SND_PCM_NONBLOCK);

    if (r < 0) {
        m_error = "can not open PCM device '" + devname + "' (" +
                  strerror(-r) + ")";
        m_zombie = true;
        return;
    }

    snd_pcm_nonblock(m_pcm, 0);

    r = snd_pcm_set_params(m_pcm,
                           SND_PCM_FORMAT_S16_LE,
                           SND_PCM_ACCESS_RW_INTERLEAVED,
                           m_nchannels,
                           samplerate,
                           1,               // allow soft resampling
                           500000);         // latency in us

    if (r < 0) {
        m_error = "can not set PCM parameters (";
        m_error += strerror(-r);
        m_error += ")";
        m_zombie = true;
    }
#else
		m_rt_params.deviceId = 0;
		m_rt_params.nChannels = m_nchannels;
		m_rt_params.firstChannel = 0;
    m_rt_params.deviceId = dac.getDefaultOutputDevice();

		RtAudio::StreamOptions options;
		options.flags = RTAUDIO_HOG_DEVICE;
		options.flags |= RTAUDIO_SCHEDULE_REALTIME;

		std::cout << "m_pcm size = " << m_nchannels * bufferFrames << "\n";
		//		non_m_pcm = (int16_t *) calloc( WRAP_SIZE, sizeof( int16_t ) );

		//std::cout << "Addres = " << &non_m_fifo << "\n";
		try {
			dac.openStream( &m_rt_params, NULL, RTAUDIO_SINT16, 48000, &bufferFrames, &audio_callback, reinterpret_cast<void *>(&non_m_fifo), &options, &errorCallback );
			dac.startStream();
		}
		catch ( RtAudioError& e ) {
			e.printMessage();
		}
#endif
}


// Destructor.
StreamAudioOutput::~StreamAudioOutput()
{
    // Close device.
#ifndef __APPLE__
    if (m_pcm != NULL) {
			snd_pcm_close(m_pcm);
    }
#endif
}


// Write audio data.
bool StreamAudioOutput::write(const std::vector<double>& samples)
{
    if (m_zombie)
        return false;

#ifndef __APPLE__			
    // Convert samples to bytes.
    samplesToInt16(samples, m_bytebuf);

    // Write data.
    unsigned int p = 0;
    unsigned int n = samples.size() / m_nchannels;
    unsigned int framesize = 2 * m_nchannels;

    while (p < n) {
        int k = snd_pcm_writei(m_pcm,  m_bytebuf.data() + p * framesize, n - p);
        if (k < 0) {
            m_error = "write failed (";
            m_error += strerror(errno);
            m_error += ")";
            // After an underrun, ALSA keeps returning error codes until we
            // explicitly fix the stream.
            snd_pcm_recover(m_pcm, k, 0);
            return false;
        } else {
            p += k;
        }
    }
#else
		for (int i=0;i<samples.size();i++) non_m_fifo.write(floor(32768.0*samples[i]));
#endif

    return true;
}

/* end */
