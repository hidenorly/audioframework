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

#include "Mixer.hpp"
#include "AudioFormatAdaptor.hpp"
#include "MixerPrimitive.hpp"
#include <vector>

#ifndef USE_TINY_MIXER_IMPL
  #define USE_TINY_MIXER_IMPL 1
#endif /* USE_TINY_MIXER_IMPL */

#if USE_TINY_MIXER_IMPL

bool Mixer::process( std::vector<AudioBuffer*> pInBuffers, AudioBuffer* pOutBuffer )
{
  bool result = false;
  if( !pInBuffers.empty() && pOutBuffer ){
    // normalize buffer format (allocate)
    AudioFormat dstFormat = pOutBuffer->getAudioFormat();
    std::vector<AudioBuffer*> normalizedInBuffers;
    normalizedInBuffers.reserve( pInBuffers.size() );
    std::vector<AudioBuffer*> allocatedBufferPointers;
    allocatedBufferPointers.reserve( pInBuffers.size() );
    int nSamples = pOutBuffer->getSamples();
    // format convert if different format
    for(AudioBuffer* pBuffer : pInBuffers){
      if( dstFormat.equal( pBuffer->getAudioFormat() ) ){
        normalizedInBuffers.push_back( pBuffer );
      } else {
        AudioBuffer* pTmpBuffer = new AudioBuffer( dstFormat, nSamples);
        if( AudioFormatAdaptor::convert(*pBuffer, *pTmpBuffer ) ){
          normalizedInBuffers.push_back( pTmpBuffer );
        }
        allocatedBufferPointers.push_back( pTmpBuffer );
      }
    }
    // do mix
    if( normalizedInBuffers.size() >= 2 ){
      result = doMix( normalizedInBuffers, pOutBuffer );
    } else {
      // only 1 source then just copy it. (no mix)
      result = true;
      *pOutBuffer = *normalizedInBuffers[0];
    }
    // release temporary buffer
    for(AudioBuffer* pBuffer : allocatedBufferPointers){
      delete pBuffer;
    }
    allocatedBufferPointers.clear();
  }

  return result;
}


bool Mixer::doMix( std::vector<AudioBuffer*> pInBuffers, AudioBuffer* pOutBuffer )
{
  bool result = false;
  AudioBuffer* pFinalOutBuffer = pOutBuffer;
  if( !pInBuffers.empty() && pOutBuffer ){
    AudioFormat format = pOutBuffer->getAudioFormat();
    int nSamples = pOutBuffer->getSamples();
    // loop a+b=c
    AudioBuffer tmpIn1Buffer( format, nSamples );
    tmpIn1Buffer = *pInBuffers[0];
    AudioBuffer* pInBuffer1 = &tmpIn1Buffer;
    AudioBuffer tmpOutBuffer( format, nSamples );
    AudioBuffer* pOutBuffer = &tmpOutBuffer;
    for(int i=1; i<pInBuffers.size(); i++){
      AudioBuffer* pInBuffer2 = pInBuffers[i];
      result = doMixPrimitive( pInBuffer1, pInBuffer2, pOutBuffer );
      if( (i+1) == pInBuffers.size() ){
        // end then copy to the final buffer
        *pFinalOutBuffer = result ? *pOutBuffer : *pInBuffer1;
      }
      if( result ){
        std::swap(pInBuffer1, pOutBuffer);
      }
    }
  }
  return result;
}

bool Mixer::doMixPrimitive( AudioBuffer* pInBuffer1, AudioBuffer* pInBuffer2, AudioBuffer* pOutBuffer )
{
  bool bHandled = false;

  if( pInBuffer1 && pInBuffer2 && pOutBuffer ){
    AudioFormat format = pOutBuffer->getAudioFormat();
    int nChannelSamples = pOutBuffer->getSamples() * format.getNumberOfChannels();
    int8_t* pRawInBuf1 = reinterpret_cast<int8_t*>( pInBuffer1->getRawBufferPointer() );
    int8_t* pRawInBuf2 = reinterpret_cast<int8_t*>( pInBuffer2->getRawBufferPointer() );
    int8_t* pRawOutBuf = reinterpret_cast<int8_t*>( pOutBuffer->getRawBufferPointer() );

    switch( format.getEncoding() ){
      case AudioFormat::ENCODING::PCM_8BIT:
        bHandled = MixerPrimitive::mix( pRawInBuf1, pRawInBuf2, pRawOutBuf, nChannelSamples);
        break;
      case AudioFormat::ENCODING::PCM_16BIT:
        bHandled = MixerPrimitive::mix( reinterpret_cast<int16_t*>(pRawInBuf1), reinterpret_cast<int16_t*>(pRawInBuf2), reinterpret_cast<int16_t*>(pRawOutBuf), nChannelSamples);
        break;
      case AudioFormat::ENCODING::PCM_32BIT:
        bHandled = MixerPrimitive::mix( reinterpret_cast<int32_t*>(pRawInBuf1), reinterpret_cast<int32_t*>(pRawInBuf2), reinterpret_cast<int32_t*>(pRawOutBuf), nChannelSamples);
        break;
      case AudioFormat::ENCODING::PCM_FLOAT:
        bHandled = MixerPrimitive::mix( reinterpret_cast<float*>(pRawInBuf1), reinterpret_cast<float*>(pRawInBuf2), reinterpret_cast<float*>(pRawOutBuf), nChannelSamples);
        break;
      case AudioFormat::ENCODING::PCM_24BIT_PACKED:
        bHandled = MixerPrimitive::mix24( pRawInBuf1, pRawInBuf2, pRawOutBuf, nChannelSamples);
        break;
      case AudioFormat::ENCODING::PCM_UNKNOWN:
        bHandled = false;
        break;
    }
  }

  return bHandled;
}

#endif /* USE_TINY_MIXER_IMPL */