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

AudioSample::AudioSample(AudioFormat format, ByteBuffer buf) : mFormat(format), mBuf(buf)
{

}


AudioSample::AudioSample(AudioFormat format) : mFormat(format)
{
  int nSize = format.getChannelsSampleByte();
  mBuf.reserve( nSize );
  ByteBuffer zeroBuf( nSize, 0 );
  AudioSample(format, zeroBuf);
}

AudioSample::~AudioSample()
{

}

uint8_t* AudioSample::getData(AudioFormat::CH channel)
{
  uint8_t* result = mBuf.data();
  result = result + mFormat.getOffSetByteInSample( channel );
  return result;
}

void AudioSample::setData(AudioFormat::CH channel, uint8_t* pData)
{
  uint8_t* buf = mBuf.data();
  buf = buf + mFormat.getOffSetByteInSample( channel );
  memcpy( buf, pData, mFormat.getSampleByte() );
}


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

int AudioBuffer::getWindowSizeUsec(void)
{
  return 1000 * getSamples() / mFormat.getSamplingRate();
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

void AudioBuffer::append(AudioBuffer& buf)
{
    ByteBuffer extBuf = buf.getRawBuffer();

    int newSize = extBuf.size() + mBuf.size();
    mBuf.reserve( newSize );

    std::copy( extBuf.begin(), extBuf.end(), std::back_inserter( mBuf ) );
}

AudioSample AudioBuffer::getSample(int nOffset)
{
  int bufSize = mFormat.getChannelsSampleByte();
  ByteBuffer dstBuf( bufSize, 0 );
  uint8_t* rawDstBuf = dstBuf.data();
  uint8_t* rawSrcBuf = mBuf.data();

  memcpy( rawDstBuf, rawSrcBuf+bufSize*nOffset, bufSize );

  AudioSample sample( mFormat, dstBuf );

  return sample;
}

void AudioBuffer::setSample(int nOffset, AudioSample& sample)
{
  int bufSize = mFormat.getChannelsSampleByte();
  uint8_t* rawDstBuf = mBuf.data();
  uint8_t* rawSrcBuf = sample.getRawBufferPointer();

  memcpy( rawDstBuf+bufSize*nOffset, rawSrcBuf, bufSize );
}

AudioBuffer AudioBuffer::getSelectedChannelData(AudioFormat outAudioFormat, AudioFormat::ChannelMapper& mapper)
{
  // extract corresponding channel's data & reconstruct the buffer
  int nSrcSamples = getSamples();
  AudioBuffer dstBuf( outAudioFormat, nSrcSamples );
  for(int i=0; i<nSrcSamples; i++){
    AudioSample aSrcSample = getSample(i);
    AudioSample aDstSample(outAudioFormat);
    for(const auto& [dstCh, srcCh] : mapper){
      aDstSample.setData( dstCh, aSrcSample.getData(srcCh) );
    }
    dstBuf.setSample(i, aDstSample);
  }
  return dstBuf;
}

