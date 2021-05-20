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
#include <iostream>

DelayFilter::DelayFilter(AudioFormat audioFormat, ChannelDelay channelDelay) : mAudioFormat(audioFormat), mChannelDelay(channelDelay)
{
  assert( audioFormat.getNumberOfChannels() == channelDelay.size() );
  int mProessSize = DEFAULT_WINDOW_SIZE_USEC;

  // get max delay size => window size
  for(auto& [ch, delayUsec] : channelDelay){
    mProessSize = std::max( mProessSize, delayUsec );
  }

  // setup per-channel delay to per-chanel FIFO buffer
  int samplingRate = audioFormat.getSamplingRate();
  AudioFormat chFormat( audioFormat.getEncoding(), samplingRate, AudioFormat::CHANNEL::CHANNEL_MONO );
  float perSampleDurationUsec = 1000000.0f / samplingRate;

  for(auto& [ch, delayUsec] : channelDelay){
    // create per-channel fifo buffer
    FifoBuffer* pFifoBuffer = new FifoBuffer( chFormat );
    mDelayBuf[ch] = pFifoBuffer;

    // enqueue delay zero data to fifo buffer
    int nDelaySamples = (int)((float)delayUsec / perSampleDurationUsec + 0.9999f);
    AudioBuffer delayZeroData(chFormat, nDelaySamples);
    ByteBuffer zeroData( chFormat.getChannelsSampleByte() * nDelaySamples, 0);
    delayZeroData.setRawBuffer( zeroData );
    pFifoBuffer->write(delayZeroData);
  }
}

DelayFilter::~DelayFilter()
{
  for(auto& [ch, pFifoBuf] : mDelayBuf){
    pFifoBuf->unlock();
  }
  for(auto& [ch, pFifoBuf] : mDelayBuf){
    delete pFifoBuf;
  }
  mDelayBuf.clear();
}

std::vector<AudioFormat> DelayFilter::getSupportedAudioFormats(void)
{
    std::vector<AudioFormat> audioFormats;
    audioFormats.push_back( AudioFormat() );
    return audioFormats;
}

int DelayFilter::getExpectedProcessingUSec(void)
{
  return DEFAULT_PROCESSING_TIME_USEC;
}

void DelayFilter::process(AudioBuffer& srcBuf, AudioBuffer& dstBuf)
{
  if( mAudioFormat.equal( srcBuf.getAudioFormat() ) && mAudioFormat.equal( dstBuf.getAudioFormat() )){

    // create temporary buffer
    AudioFormat tmpFormat( mAudioFormat.getEncoding(), mAudioFormat.getSamplingRate(), AudioFormat::CHANNEL::CHANNEL_MONO );
    int nSrcSamples = srcBuf.getSamples();
    std::map<AudioFormat::CH, AudioBuffer*> tmpBuffers;
    for(auto& [ch, delayUsec] : mChannelDelay){
     tmpBuffers[ch] = new AudioBuffer( tmpFormat, nSrcSamples );
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
      delete tmpBuffers[ch];
      tmpBuffers[ch] = new AudioBuffer( tmpFormat, nSrcSamples);
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
      delete tmpBuffers[ch];
    }
    tmpBuffers.clear();
  }
}

int DelayFilter::getRequiredWindowSizeUsec(void)
{
  return ( mProessSize > 0 ) ? mProessSize : DEFAULT_WINDOW_SIZE_USEC;
};
