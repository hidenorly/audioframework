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

#include "Volume.hpp"
#include "VolumePrimitive.hpp"

bool Volume::process( AudioBuffer* pInBuf, AudioBuffer* pOutBuf, float volume )
{
  bool bHandled = false;

  if( pInBuf && pOutBuf ){
    int nSamples = pInBuf->getSamples();
    if( pOutBuf->getSamples() != nSamples) {
      pOutBuf->resize( nSamples );
    }
    AudioFormat srcFormat = pInBuf->getAudioFormat();
    AudioFormat dstFormat = pOutBuf->getAudioFormat();
    if( srcFormat.equal(dstFormat) ){
      if( 100.0f!=volume ){
        int nChannelSamples = nSamples * dstFormat.getNumberOfChannels();

        int8_t* pRawInBuf = reinterpret_cast<int8_t*>( pInBuf->getRawBufferPointer() );
        int8_t* pRawOutBuf = reinterpret_cast<int8_t*>( pOutBuf->getRawBufferPointer() );

        switch( dstFormat.getEncoding() ){
          case AudioFormat::ENCODING::PCM_8BIT:
            bHandled = VolumePrimitive::volume( pRawInBuf, pRawOutBuf, volume, nChannelSamples );
            break;
          case AudioFormat::ENCODING::PCM_16BIT:
            bHandled = VolumePrimitive::volume( reinterpret_cast<int16_t*>(pRawInBuf), reinterpret_cast<int16_t*>(pRawOutBuf), volume, nChannelSamples );
            break;
          case AudioFormat::ENCODING::PCM_32BIT:
            bHandled = VolumePrimitive::volume( reinterpret_cast<int32_t*>(pRawInBuf), reinterpret_cast<int32_t*>(pRawOutBuf), volume, nChannelSamples );
            break;
          case AudioFormat::ENCODING::PCM_FLOAT:
            bHandled = VolumePrimitive::volume( reinterpret_cast<float*>(pRawInBuf), reinterpret_cast<float*>(pRawOutBuf), volume, nChannelSamples );
            break;
          case AudioFormat::ENCODING::PCM_24BIT_PACKED:
            bHandled = VolumePrimitive::volume24( pRawInBuf, pRawOutBuf, volume, nChannelSamples );
            break;
          case AudioFormat::ENCODING::PCM_UNKNOWN:
          default:
            bHandled = false;
            break;
        }
      } else {
        *pOutBuf = *pInBuf;
        bHandled = true;
      }
    }
  }

  return bHandled;
}

bool Volume::process( AudioBuffer& inBuf, AudioBuffer& outBuf, float volume )
{
  return process(&inBuf, &outBuf, volume);
}

