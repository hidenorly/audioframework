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

#include "ChannelMultiplexer.hpp"
#include "AudioFormat.hpp"
#include "Buffer.hpp"

std::shared_ptr<AudioBuffer> ChannelMuxer::perChannelMux(std::vector<std::shared_ptr<AudioBuffer>> pSrcBufs, AudioFormat::CHANNEL outChannel)
{
  std::shared_ptr<AudioBuffer> pOutBuf;
  int nChannels = pSrcBufs.size();
  if( nChannels && ( nChannels == AudioFormat::getNumberOfChannels( outChannel ) ) ){
    AudioFormat srcFormat = pSrcBufs[0]->getAudioFormat();
    int nSamples = pSrcBufs[0]->getNumberOfSamples();

    bool isSameFormatSamples = true;
    for( auto& pBuf : pSrcBufs ){
      isSameFormatSamples = isSameFormatSamples & srcFormat.equal( pBuf->getAudioFormat() ) & ( nSamples == pBuf->getNumberOfSamples() );
      if( !isSameFormatSamples ) break;
    }
    if( isSameFormatSamples ){
      // ensure out buffer
      pOutBuf = std::make_shared<AudioBuffer>(
        AudioFormat(
          srcFormat.getEncoding(),
          srcFormat.getSamplingRate(),
          outChannel),
        nSamples );
      uint8_t* pRawOutBuf = pOutBuf->getRawBufferPointer();

      std::vector<uint8_t*> pRawSrcBufs;
      for( int i=0; i<nChannels; i++ ){
        pRawSrcBufs.push_back( pSrcBufs[i]->getRawBufferPointer() );
      }

      int nSampleBytes = srcFormat.getSampleByte();
      int pSamplelOffset = 0;
      for( int i=0; i<nSamples; i++ ){
        for( int j=0; j<nChannels; j++ ){
          uint8_t* pSrcBuf = pRawSrcBufs[j] + pSamplelOffset;
          for( int k=0; k<nSampleBytes; k++ ){
            *pRawOutBuf = *pSrcBuf;
            pRawOutBuf++; pSrcBuf++;
          }
        }
        pSamplelOffset = pSamplelOffset + nSampleBytes;
      }
    }
  }
  return pOutBuf;
}

std::shared_ptr<AudioBuffer> ChannelMuxer::perChannelMux(std::vector<std::shared_ptr<AudioBuffer>> pSrcBufs, std::vector<std::vector<AudioFormat::CH>> channels, AudioFormat::CHANNEL outChannel)
{
  std::shared_ptr<AudioBuffer> pOutBuf;
  if( pSrcBufs.size() && pSrcBufs.size() == channels.size() ){
    AudioFormat srcFormat = pSrcBufs[0]->getAudioFormat();
    int nSamples = pSrcBufs[0]->getNumberOfSamples();

    bool isSameEncodingSamples = true;
    for( auto& pBuf : pSrcBufs ){
      isSameEncodingSamples = isSameEncodingSamples & ( srcFormat.getEncoding() == pBuf->getAudioFormat().getEncoding() ) & ( nSamples == pBuf->getNumberOfSamples() );
      if( !isSameEncodingSamples ) break;
    }
    if( isSameEncodingSamples ){
      // src buffer pointer
      std::vector<uint8_t*> pRawSrcBufs;
      for( int i=0, c=pSrcBufs.size(); i<c; i++ ){
        pRawSrcBufs.push_back( pSrcBufs[i]->getRawBufferPointer() );
      }

      // ensure out buffer
      AudioFormat dstFormat( srcFormat.getEncoding(), srcFormat.getSamplingRate(), outChannel );
      int nOutSampleBytes = dstFormat.getChannelsSampleByte();
      pOutBuf = std::make_shared<AudioBuffer>( dstFormat, nSamples );
      uint8_t* pRawOutBufBase = pOutBuf->getRawBufferPointer();

      for( int i=0; i<nSamples; i++ ){
        for( int j=0, c=pSrcBufs.size(); j<c; j++ ){
          AudioFormat theSrcFormat = pSrcBufs[j]->getAudioFormat();
          int nNumberOfChannels = theSrcFormat.getNumberOfChannels();
          int theSampleBytes = theSrcFormat.getChannelsSampleByte();
          int thePerChannelSampleByte = theSrcFormat.getSampleByte();
          uint8_t* pSrcBuf = pRawSrcBufs[j] + theSampleBytes * i;
          uint8_t* pRawOutBufOffset = pRawOutBufBase + nOutSampleBytes * i;
          for( int k=0; k<nNumberOfChannels; k++ ){
            uint8_t* pRawOutBuf = pRawOutBufOffset + dstFormat.getOffSetByteInSample( channels[j][k] );
            for( int l=0; l<thePerChannelSampleByte; l++ ){
              *pRawOutBuf++ = *pSrcBuf++;
            }
          }
        }
      }
    }
  }
  return pOutBuf;
}