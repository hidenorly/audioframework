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


#include "Buffer.hpp"

AudioBuffer::AudioBuffer(AudioFormat format, int samples)
{
  setAudioFormat( format );
  resize( samples );
}

AudioBuffer::AudioBuffer(AudioBuffer& buf)
{

}

AudioBuffer::~AudioBuffer()
{

}

AudioBuffer::AudioBuffer()
{
  setAudioFormat( AudioFormat() );
  resize( 0 );
}

AudioBuffer& AudioBuffer::operator=(AudioBuffer& buf)
{
  mBuf = buf.getRawBuffer();  // copy data
  // copy attributes
  mFormat = buf.getAudioFormat();

  return *this;
}

bool AudioBuffer::isSameAudioFormat(AudioBuffer& buf)
{
  AudioFormat target = buf.getAudioFormat();
  return mFormat.equal( target );
}

int AudioBuffer::getSamples(void)
{
  return mBuf.size() / mFormat.getChannelsSampleByte();
}

void AudioBuffer::setAudioFormat( AudioFormat format )
{
  int samples = getSamples();
  mFormat = format;
  resize( samples );
}

void AudioBuffer::resize( int samples )
{
  int bufSize = mFormat.getChannelsSampleByte() * samples;
  ByteBuffer zeroBuf( bufSize, 0 );
  mBuf = zeroBuf;
}

