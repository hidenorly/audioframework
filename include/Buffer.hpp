/* 
  Copyright (C) 2021 hidenorly

   Licensed under the Apache License, Version 2.0 (the "License");
   you may not use this file except in compliance with the License.
   You may obtain a copy of the License at

       http://www.apache.org/licenses/LICENSE-2.0

   Unless required by applicable law or agreed to in writing, software
   distributed under the License is distributed on an "AS IS" BASIS,
   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
   See the License for the specific language governing permissions and
   limitations under the License.
*/

#ifndef __BUFFER_HPP__
#define __BUFFER_HPP__

#include <vector>
#include "AudioFormat.hpp"
#include <map>

/*
  @desc ByteBuffer. Note that this is based on std::vector therefore vector related methods are available such as data(), size(), [] oprators.
*/
typedef std::vector<uint8_t> ByteBuffer;

/*
  @desc This enables to set/get per-sample and per-channel access
*/
class AudioSample
{
protected:
  AudioFormat mFormat;
  ByteBuffer mBuf;

public:
  AudioSample(AudioFormat format = AudioFormat());
  AudioSample(AudioFormat format, ByteBuffer buf);
  virtual ~AudioSample();

  /*  @desc get specifed channel's sample
      @arg channel: specifying the channel
      @return specified channel's data. Note that this should be fit to the specified AudioFormat's encoding such as 16bit LPCM data, etc. */
  uint8_t* getData(AudioFormat::CH channel);

  /*  @desc set specifed channel's sample
      @arg channel: specifying the data's channel
      @arg pData: specified channel's data. Note that this should be fit to the specified AudioFormat's encoding such as 16bit LPCM data, etc. */
  void setData(AudioFormat::CH channel, uint8_t* pData);

  /* @desc get one sample ByteBuffer which includes all of channel data
     @return ButeBuffer */
  ByteBuffer getRawBuffer();

  /* @desc get one sample data pointer which includes all of channel data
     @return the raw bufer pointer. Note that the user must access as the AudioFormat such as encoding, channel but this is just 1 sample. */
  uint8_t* getRawBufferPointer();
};

/*
  @desc This enables to handle buffer of both compressed audio data and PCM (non-compressed audio data)
*/
class IAudioBuffer
{
protected:
  AudioFormat mFormat;
  ByteBuffer mBuf;

public:
  virtual ~IAudioBuffer();
  /* @desc get audio data pointer which includes all of channel data
     @return the raw buffer pointer. Note that the user must access within the size of getRawBuffer().size() */
  virtual uint8_t* getRawBufferPointer(void);

  /* @desc get audio buffer which includes all of channel data
     @return the ByteBuffer as REFERENCE. To avoid any change for this IAudioBuffer instance, IAudioBuffer tmp; tmp=this->getRawBuffer(); Do IAudioBuffer copiedBuffer = tmp; */
  virtual ByteBuffer& getRawBuffer(void);

  /* @desc replace internal raw audio buffer
     @arg ByteBuffer instance. You must use the same AudioFormat's ByteBuffer. */
  virtual void setRawBuffer(ByteBuffer& buf);

  /* @desc copy the specified data to this instance */
  IAudioBuffer& operator=(IAudioBuffer& buf);
  /* @desc get the instance's AudioFormat
     @return AudioFormat instance */
  virtual AudioFormat getAudioFormat(void);

  /* @desc check this instance's AudioFormat with the specified IAudioBuffer's format
     @return true: same format, false: different format */
  virtual bool isSameAudioFormat(IAudioBuffer& buf);

  /* @desc add specified IAudioBuffer's buffer to this instance's buffer's tail
     @arg IAudioBuffer */
  virtual void append(IAudioBuffer& buf);

  /* @desc change AudioFormat. Usually this might be useful after setrawBuffer if the format is different */
  virtual void setAudioFormat( AudioFormat format );

  /* @desc get number of samples which this instance has */
  virtual int getNumberOfSamples(void);
};

/*
  @desc This enables to handle PCM (non-compressed audio data)
*/
class AudioBuffer : public IAudioBuffer
{
public:
  AudioBuffer(AudioFormat format, int samples);
  AudioBuffer(AudioBuffer& buf);
  AudioBuffer();
  virtual ~AudioBuffer();

  /* @desc get corresponding time at the format's sampling rate
    @return usec. number of samples * duration of one sample */
  int getWindowSizeUsec(void);

  /* @desc change AudioFormat.
           Note that buffer will be cleared if format is different from this instance's current format. */
  virtual void setAudioFormat( AudioFormat format );

  /* @desc change buffer's sample size.
           Specifying smaller then current means cut off the data.
           Specifying larger than current means adding 0
     @arg samples : the number of samples 
     @arg bClear : true : force clear the buffer */
  void resize( int samples, bool bClear = true );

  /* @desc get specified one sample
     @arg nOffset : number of sample offset from the beginning of this buffer */
  AudioSample getSample(int nOffset);

  /* @desc replace the sample with specified sample
     @arg nOffset : number of sample offset from the beginning of this buffer
     @arg AudioSample : the sample's data which must fit with this instance's format */
  void setSample(int nOffset, AudioSample& sample);

  /* @desc check specified channel mapper is same as this instance's channels
     @arg mapper: ChannelMapper
     @return true if e.g. channel is STEREO and ChannelMap is L->L, R->R */
  bool isSameChannelMap(AudioFormat::ChannelMapper& mapper);

  /* @desc Get specified channel's data
     @arg outAudioFormat: AudioFormat
     @arg mapper: ChannelMapper as this instance channel to the output buffer's channel
     @return specified channel map applied AudioBuffer */
  AudioBuffer getSelectedChannelData(AudioFormat outAudioFormat, AudioFormat::ChannelMapper& mapper);
};

/*
  @desc This enables to handle compressed audio
*/
class CompressAudioBuffer : public IAudioBuffer
{
protected:
  int mChunkSize;
  static const int DEFAULT_CHUNK_SIZE = 256;

public:
  CompressAudioBuffer(AudioFormat format = AudioFormat(AudioFormat::ENCODING::COMPRESSED), int nChunkSize = DEFAULT_CHUNK_SIZE);
  CompressAudioBuffer& operator=(CompressAudioBuffer& buf);

  /* @desc change AudioFormat. Usually */
  virtual void setAudioFormat( AudioFormat format );


  virtual void append(IAudioBuffer& buf);
};

#endif /* __BUFFER_HPP__ */