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
#include <cstring>
#include <cassert>


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

ByteBuffer AudioSample::getRawBuffer()
{
  return mBuf;
}

uint8_t* AudioSample::getRawBufferPointer()
{
  return mBuf.data();
}

IAudioBuffer::~IAudioBuffer()
{

}

uint8_t* IAudioBuffer::getRawBufferPointer(void)
{
  return mBuf.data();
}

ByteBuffer& IAudioBuffer::getRawBuffer(void)
{
  return mBuf;
}

void IAudioBuffer::setRawBuffer(ByteBuffer& buf)
{
  mBuf = buf;
}

AudioFormat IAudioBuffer::getAudioFormat(void)
{
  return mFormat;
}

bool IAudioBuffer::isSameAudioFormat(IAudioBuffer& buf)
{
  return mFormat.equal( buf.getAudioFormat() );
}

void IAudioBuffer::append(IAudioBuffer& buf)
{
  if( isSameAudioFormat( buf )){
    ByteBuffer extBuf = buf.getRawBuffer();

    int newSize = extBuf.size() + mBuf.size();
    mBuf.reserve( newSize );

    std::copy( extBuf.begin(), extBuf.end(), std::back_inserter( mBuf ) );
  }
}

void IAudioBuffer::setAudioFormat( AudioFormat format )
{
  mFormat = format;
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

IAudioBuffer& IAudioBuffer::operator=(IAudioBuffer& buf)
{
  mBuf = buf.getRawBuffer();  // copy data
  // copy attributes
  mFormat = buf.getAudioFormat();

  return *this;
}

int IAudioBuffer::getNumberOfSamples(void)
{
  return mFormat.getChannelsSampleByte() ? mBuf.size() / mFormat.getChannelsSampleByte() : mBuf.size();
}

int AudioBuffer::getWindowSizeUsec(void)
{
  return 1000 * getNumberOfSamples() / mFormat.getSamplingRate();
}

void AudioBuffer::setAudioFormat( AudioFormat format )
{
  bool bFormatChanged = !format.equal( mFormat );
  int samples = getNumberOfSamples();
  mFormat = format;
  resize( samples, bFormatChanged );
}

void AudioBuffer::resize( int samples, bool bClear )
{
  int bufSize = mFormat.getChannelsSampleByte() * samples;
  if( bClear ){
    ByteBuffer buf( bufSize,0 );
    mBuf = buf;
  } else {
//    if( bufSize > mBuf.size() ){
//      mBuf.reserve( bufSize );
//      ByteBuffer buf( bufSize - mBuf.size(), 0 );
//      std::copy( buf.begin(), buf.end(), std::back_inserter( mBuf ) );
//    } else {
      mBuf.resize( bufSize );
//    }
  }
  assert( bufSize == mBuf.size() );
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

bool AudioBuffer::isSameChannelMap(AudioFormat::ChannelMapper& mapper)
{
  bool result = true;

  int nChannels = mFormat.getNumberOfChannels();
  for(const auto& [dstCh, srcCh] : mapper){
    result = ( dstCh == srcCh );
    nChannels--;
    if( !result ) break;
  }
  result = result & !nChannels;

  return result;
}

AudioBuffer AudioBuffer::getSelectedChannelData(AudioFormat outAudioFormat, AudioFormat::ChannelMapper& mapper)
{
  // extract corresponding channel's data & reconstruct the buffer
  int nSrcSamples = getNumberOfSamples();
  AudioBuffer dstBuf( outAudioFormat, nSrcSamples );
  if( !isSameChannelMap(mapper) ){
    for(int i=0; i<nSrcSamples; i++){
      AudioSample aSrcSample = getSample(i);
      AudioSample aDstSample(outAudioFormat);
      for(const auto& [dstCh, srcCh] : mapper){
        aDstSample.setData( dstCh, aSrcSample.getData(srcCh) );
      }
      dstBuf.setSample(i, aDstSample);
    }
  } else {
    dstBuf = *this;
  }
  return dstBuf;
}

CompressAudioBuffer::CompressAudioBuffer(AudioFormat format, int nChunkSize) : mChunkSize(nChunkSize)
{
  mFormat = format;
  mBuf = ByteBuffer( nChunkSize, 0 );
}

CompressAudioBuffer& CompressAudioBuffer::operator=(CompressAudioBuffer& buf)
{
  mBuf = buf.getRawBuffer();
  mFormat = buf.getAudioFormat();

  return *this;
}

void CompressAudioBuffer::setAudioFormat( AudioFormat format )
{
  if( !mFormat.equal(format) ){
    int nChunkSize = mBuf.size();
    mBuf.clear();
    mBuf = ByteBuffer( nChunkSize, 0 );
  }
  mFormat = format;
}

void CompressAudioBuffer::append(IAudioBuffer& buf)
{
  ByteBuffer extBuf = buf.getRawBuffer();

  int newSize = extBuf.size() + mBuf.size();
  mBuf.reserve( newSize );

  std::copy( extBuf.begin(), extBuf.end(), std::back_inserter( mBuf ) );
}
