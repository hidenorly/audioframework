/*
  Copyright (C) 2021, 2023 hidenorly

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

#include "AccousticEchoCancelFilter.hpp"
#include "AccousticEchoCancelFilterPrimitive.hpp"
#include <iostream>
#include <cassert>

AccousticEchoCancelFilter::AccousticEchoCancelFilter(void)
{

}

AccousticEchoCancelFilter::~AccousticEchoCancelFilter()
{
}

void AccousticEchoCancelFilter::process(AudioBuffer& inBuf, AudioBuffer& refBuf)
{
  AudioFormat srcFormat = inBuf.getAudioFormat();
  AudioFormat refFormat = refBuf.getAudioFormat();
  if( srcFormat.equal(refFormat) ){
    int nChannelSamples = inBuf.getNumberOfSamples() * refFormat.getNumberOfChannels();

    int8_t* pRawInBuf = reinterpret_cast<int8_t*>( inBuf.getRawBufferPointer() );
    int8_t* pRawRefBuf = reinterpret_cast<int8_t*>( refBuf.getRawBufferPointer() );
    bool bHandled = false;

    switch( refFormat.getEncoding() ){
      case AudioFormat::ENCODING::PCM_8BIT:
        bHandled = AccousticEchoCancelFilterPrimitive::process( pRawInBuf, pRawRefBuf, nChannelSamples );
        break;
      case AudioFormat::ENCODING::PCM_16BIT:
        bHandled = AccousticEchoCancelFilterPrimitive::process( reinterpret_cast<int16_t*>(pRawInBuf), reinterpret_cast<int16_t*>(pRawRefBuf), nChannelSamples );
        break;
      case AudioFormat::ENCODING::PCM_32BIT:
        bHandled = AccousticEchoCancelFilterPrimitive::process( reinterpret_cast<int32_t*>(pRawInBuf), reinterpret_cast<int32_t*>(pRawRefBuf), nChannelSamples );
        break;
      case AudioFormat::ENCODING::PCM_FLOAT:
        bHandled = AccousticEchoCancelFilterPrimitive::process( reinterpret_cast<float*>(pRawInBuf), reinterpret_cast<float*>(pRawRefBuf), nChannelSamples );
        break;
      case AudioFormat::ENCODING::PCM_24BIT_PACKED:
        bHandled = AccousticEchoCancelFilterPrimitive::process24( pRawInBuf, pRawRefBuf, nChannelSamples );
        break;
      case AudioFormat::ENCODING::PCM_UNKNOWN:
      default:
        break;
    }
    assert( true == bHandled );
  }
}
