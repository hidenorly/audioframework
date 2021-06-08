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

typedef std::vector<uint8_t> ByteBuffer;

class AudioSample
{
protected:
  AudioFormat mFormat;
  ByteBuffer mBuf;

public:
  AudioSample(AudioFormat format = AudioFormat());
  AudioSample(AudioFormat format, ByteBuffer buf);
  virtual ~AudioSample();
  uint8_t* getData(AudioFormat::CH channel);
  void setData(AudioFormat::CH channel, uint8_t* pData);
  ByteBuffer getRawBuffer();
  uint8_t* getRawBufferPointer();
};

class IAudioBuffer
{
protected:
  AudioFormat mFormat;
  ByteBuffer mBuf;

public:
  virtual ~IAudioBuffer();
  virtual uint8_t* getRawBufferPointer(void);
  virtual ByteBuffer& getRawBuffer(void);
  virtual void setRawBuffer(ByteBuffer& buf);
  virtual AudioFormat getAudioFormat(void);
  virtual bool isSameAudioFormat(IAudioBuffer& buf);
  virtual void append(IAudioBuffer& buf);
  virtual void setAudioFormat( AudioFormat format );
  virtual int getSamples(void);
};

class AudioBuffer : public IAudioBuffer
{
public:
  AudioBuffer(AudioFormat format, int samples);
  AudioBuffer(AudioBuffer& buf);
  AudioBuffer();
  virtual ~AudioBuffer();

  int getWindowSizeUsec(void);
  AudioBuffer& operator=(AudioBuffer& buf);
  virtual void setAudioFormat( AudioFormat format );
  void resize( int samples );
  AudioSample getSample(int nOffset);
  void setSample(int nOffset, AudioSample& sample);
  bool isSameChannelMap(AudioFormat::ChannelMapper& mapper);
  AudioBuffer getSelectedChannelData(AudioFormat outAudioFormat, AudioFormat::ChannelMapper& mapper);
};

class CompressAudioBuffer : public IAudioBuffer
{
protected:
  int mChunkSize;
  static const int DEFAULT_CHUNK_SIZE = 256;

public:
  CompressAudioBuffer(AudioFormat format = AudioFormat(AudioFormat::ENCODING::COMPRESSED), int nChunkSize = DEFAULT_CHUNK_SIZE);
  CompressAudioBuffer& operator=(CompressAudioBuffer& buf);
  virtual void setAudioFormat( AudioFormat format, int nChunkSize = DEFAULT_CHUNK_SIZE );
  virtual void append(CompressAudioBuffer& buf);
};

#endif /* __BUFFER_HPP__ */