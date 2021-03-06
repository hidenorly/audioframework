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

#include "ChannelDemultiplexer.hpp"
#include "AudioFormat.hpp"
#include "Buffer.hpp"
#include <iostream>

std::vector<std::shared_ptr<AudioBuffer>> ChannelDemuxer::perChannelDemux(std::shared_ptr<AudioBuffer> pSrcBuffer)
{
  std::vector<std::shared_ptr<AudioBuffer>> pOutBufs;
  if( pSrcBuffer ){
    std::vector<uint8_t*> pRawOutBufs;
    AudioFormat srcFormat = pSrcBuffer->getAudioFormat();
    int nSamples = pSrcBuffer->getNumberOfSamples();
    int nChannels = srcFormat.getNumberOfChannels();
    // ensure per-channel buffer
    for( int i=0; i<nChannels; i++ ){
      pOutBufs.push_back( std::make_shared<AudioBuffer>(
        AudioFormat(
          srcFormat.getEncoding(),
          srcFormat.getSamplingRate(),
          AudioFormat::CHANNEL::CHANNEL_MONO), nSamples ) );
      pRawOutBufs.push_back( pOutBufs[i]->getRawBufferPointer() );
    }
    int nSampleBytes = srcFormat.getSampleByte(); // 1 ch
    uint8_t* pSrcBuf = pSrcBuffer->getRawBufferPointer();
    int pSamplelOffset = 0;
    for( int i=0; i<nSamples; i++ ){
      for( int j=0; j<nChannels; j++ ){
        uint8_t* pOutBase = pRawOutBufs[j];
        pOutBase = pOutBase + pSamplelOffset;
        for( int k=0; k<nSampleBytes; k++ ){
          *pOutBase = *pSrcBuf;
          pOutBase++; pSrcBuf++;
        }
      }
      pSamplelOffset = pSamplelOffset + nSampleBytes;
    }
  }
  return pOutBufs;
}

std::vector<std::shared_ptr<AudioBuffer>> ChannelDemuxer::perChannelDemux(std::shared_ptr<AudioBuffer> pSrcBuffer, std::vector<std::vector<AudioFormat::CH>> channels)
{
  std::vector<std::shared_ptr<AudioBuffer>> pOutBufs;
  if( pSrcBuffer ){
    std::vector<uint8_t*> pRawOutBufs;
    AudioFormat srcFormat = pSrcBuffer->getAudioFormat();
    int nSamples = pSrcBuffer->getNumberOfSamples();
    // ensure requested channels buffer
    int i=0;
    for( auto& aChannels : channels ){
      pOutBufs.push_back( std::make_shared<AudioBuffer>(
        AudioFormat(
          srcFormat.getEncoding(),
          srcFormat.getSamplingRate(),
          AudioFormat::getAudioChannel( aChannels.size() )
          ), nSamples ) );
      pRawOutBufs.push_back( pOutBufs[i++]->getRawBufferPointer() );
    }
    int nSampleBytes = srcFormat.getChannelsSampleByte();
    int nPerChannelBytes = nSampleBytes / srcFormat.getNumberOfChannels();
    uint8_t* pSrcBufBase = pSrcBuffer->getRawBufferPointer();
    for( int i=0; i<nSamples; i++ ){
      int j=0;
      for( auto& aChannels : channels ){
        int theNumOfChannels = aChannels.size();
        int k=0;
        for( auto& aChannel : aChannels ){
          uint8_t* pSrcBuf = pSrcBufBase + nSampleBytes * i + srcFormat.getOffSetByteInSample(aChannel);
          uint8_t* pOutBase = pRawOutBufs[j] + nPerChannelBytes*theNumOfChannels*i + nPerChannelBytes * k;
          for( int l=0; l<nPerChannelBytes; l++ ){
            *pOutBase++ = *pSrcBuf++;
          }
          k++;
        }
        j++;
      }
    }
  }
  return pOutBufs;
}