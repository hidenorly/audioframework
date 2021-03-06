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

#include "DelayFilter.hpp"
#include "AudioFormat.hpp"
#include "Buffer.hpp"
#include "FifoBuffer.hpp"
#include <algorithm>
#include <cassert>

std::vector<AudioFormat> DelayFilter::getSupportedAudioFormats(void)
{
    std::vector<AudioFormat> audioFormats;
    audioFormats.push_back( AudioFormat() );
    return audioFormats;
}

int DelayFilter::getRequiredWindowSizeUsec(void)
{
  return mWindowSize;
};


DelayFilter::DelayFilter(AudioFormat audioFormat, int delayUsec) : mAudioFormat(audioFormat)
{
  mWindowSize = std::max( DEFAULT_WINDOW_SIZE_USEC, delayUsec );
  if( delayUsec ){
    int samplingRate = audioFormat.getSamplingRate();
    float perSampleDurationUsec = 1000000.0f / samplingRate;
    int nDelaySamples = (int)((float)delayUsec / perSampleDurationUsec + 0.9999f);
    AudioBuffer delayZeroData(audioFormat, nDelaySamples);
    ByteBuffer zeroData( audioFormat.getChannelsSampleByte() * nDelaySamples, 0);
    delayZeroData.setRawBuffer( zeroData );
    mpDelayBuf = std::make_shared<FifoBuffer>( audioFormat );
    mpDelayBuf->write(delayZeroData);
  } else {
    mpDelayBuf.reset();
  }
}

DelayFilter::~DelayFilter()
{
  if( mpDelayBuf ){
    mpDelayBuf->unlock();
    mpDelayBuf.reset();
  }
}

void DelayFilter::process(AudioBuffer& srcBuf, AudioBuffer& dstBuf)
{
  if( mAudioFormat.equal( srcBuf.getAudioFormat() ) && mAudioFormat.equal( dstBuf.getAudioFormat() )){
    if( mpDelayBuf ){
      mpDelayBuf->write( srcBuf );
      mpDelayBuf->read( dstBuf );
    } else {
      dstBuf = srcBuf;
    }
  }
}


PerChannelDelayFilter::PerChannelDelayFilter(AudioFormat audioFormat, ChannelDelay channelDelay) : DelayFilter(audioFormat, 0), mChannelDelay(channelDelay)
{
  assert( audioFormat.getNumberOfChannels() == channelDelay.size() );
  mWindowSize = DEFAULT_WINDOW_SIZE_USEC;

  // get max delay size => window size
  for(auto& [ch, delayUsec] : channelDelay){
    mWindowSize = std::max( mWindowSize, delayUsec );
  }

  // setup per-channel delay to per-chanel FIFO buffer
  int samplingRate = audioFormat.getSamplingRate();
  AudioFormat chFormat( audioFormat.getEncoding(), samplingRate, AudioFormat::CHANNEL::CHANNEL_MONO );
  float perSampleDurationUsec = 1000000.0f / samplingRate;

  for(auto& [ch, delayUsec] : channelDelay){
    // create per-channel fifo buffer
    std::shared_ptr<FifoBuffer> pFifoBuffer = std::make_shared<FifoBuffer>( chFormat );
    mDelayBuf[ch] = pFifoBuffer;

    // enqueue delay zero data to fifo buffer
    int nDelaySamples = (int)((float)delayUsec / perSampleDurationUsec + 0.9999f);
    AudioBuffer delayZeroData(chFormat, nDelaySamples);
    ByteBuffer zeroData( chFormat.getChannelsSampleByte() * nDelaySamples, 0);
    delayZeroData.setRawBuffer( zeroData );
    pFifoBuffer->write(delayZeroData);
  }
}

PerChannelDelayFilter::~PerChannelDelayFilter()
{
  for(auto& [ch, pFifoBuf] : mDelayBuf){
    pFifoBuf->unlock();
  }
  for(auto& [ch, pFifoBuf] : mDelayBuf){
    pFifoBuf.reset();
  }
  mDelayBuf.clear();
}

void PerChannelDelayFilter::process(AudioBuffer& srcBuf, AudioBuffer& dstBuf)
{
  if( mAudioFormat.equal( srcBuf.getAudioFormat() ) && mAudioFormat.equal( dstBuf.getAudioFormat() )){

    // create temporary buffer
    AudioFormat tmpFormat( mAudioFormat.getEncoding(), mAudioFormat.getSamplingRate(), AudioFormat::CHANNEL::CHANNEL_MONO );
    int nSrcSamples = srcBuf.getNumberOfSamples();
    std::map<AudioFormat::CH, std::shared_ptr<AudioBuffer>> tmpBuffers;
    for(auto& [ch, delayUsec] : mChannelDelay){
     tmpBuffers[ch] = std::make_shared<AudioBuffer>( tmpFormat, nSrcSamples );
    }

    // read per-channel data
    for(int i=0; i<nSrcSamples; i++){
      AudioSample aSrcSample = srcBuf.getSample(i);
      for(auto& [ch, delayUsec] : mChannelDelay){
        AudioSample tmpSample(tmpFormat);
        tmpSample.setData( AudioFormat::CH::MONO, aSrcSample.getData(ch) );
        tmpBuffers[ch]->setSample(i, tmpSample);
      }
    }

    // enqueue to per-channel fifo buffer
    for(auto& [ch, pFifoBuf] : mDelayBuf){
      pFifoBuf->write( *tmpBuffers[ch] );
      tmpBuffers[ch] = std::make_shared<AudioBuffer>( tmpFormat, nSrcSamples);
      pFifoBuf->read( *tmpBuffers[ch] );
    }

    // dequeue from per-channel fifo buffer & write out to the destinated buffer
    for(int i=0; i<nSrcSamples; i++){
      AudioSample aDstSample( mAudioFormat );
      for(auto& [ch, delayUsec] : mChannelDelay){
        AudioSample aSrcSample = tmpBuffers[ch]->getSample(i);
        aDstSample.setData( ch, aSrcSample.getData(AudioFormat::CH::MONO) );
      }
      dstBuf.setSample(i, aDstSample);
    }

    for(auto& [ch, delayUsec] : mChannelDelay){
      tmpBuffers[ch].reset();
    }
    tmpBuffers.clear();
  }
}

