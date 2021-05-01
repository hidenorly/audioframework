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

#include "ChannelConversionPrimitives.hpp"

// TODO: rewrite with template
// DO NOT USE THIS TINY CHANNEL CONVERSION in the production device
// SHOULD REPLACE WITH HIGH QUALITY SRC

#ifndef USE_TINY_CC_IMPL
  #define USE_TINY_CC_IMPL 1
#endif /* USE_TINY_CC_IMPL */

#if USE_TINY_CC_IMPL

bool ChannelConverter::convert(uint8_t* pSrc, uint8_t* pDst, AudioFormat::CHANNEL srcChannel, AudioFormat::CHANNEL dstChannel, int nSamples)
{
  bool result = true;
  int nDstChannels = AudioFormat::getNumberOfChannels(dstChannel);

  switch( srcChannel ){
    case AudioFormat::CHANNEL::CHANNEL_MONO:
      for(int i=0; i<nSamples; i++){
          for(int j=0; j<nDstChannels; j++){
            *pDst++ = *pSrc;
          }
          pSrc++;
      }
      break;
    case AudioFormat::CHANNEL::CHANNEL_STEREO:
      break;
    case AudioFormat::CHANNEL::CHANNEL_4CH:
      break;
    case AudioFormat::CHANNEL::CHANNEL_5CH:
      break;
    case AudioFormat::CHANNEL::CHANNEL_5_1CH:
      break;
    case AudioFormat::CHANNEL::CHANNEL_5_1_2CH:
      break;
    case AudioFormat::CHANNEL::CHANNEL_5_0_2CH:
      break;
    case AudioFormat::CHANNEL::CHANNEL_7_1CH:
      break;
    case AudioFormat::CHANNEL::CHANNEL_UNKNOWN:
      result = false;
      break;
  }
  return result;
}

bool ChannelConverter::convert(uint16_t* pSrc, uint16_t* pDst, AudioFormat::CHANNEL srcChannel, AudioFormat::CHANNEL dstChannel, int nSamples)
{
  return true;
}

bool ChannelConverter::convert24(uint8_t* pSrc, uint8_t* pDst, AudioFormat::CHANNEL srcChannel, AudioFormat::CHANNEL dstChannel, int nSamples)
{
  return true;
}

bool ChannelConverter::convert(uint32_t* pSrc, uint32_t* pDst, AudioFormat::CHANNEL srcChannel, AudioFormat::CHANNEL dstChannel, int nSamples)
{
  return true;
}

bool ChannelConverter::convert(float* pSrc, float* pDst, AudioFormat::CHANNEL srcChannel, AudioFormat::CHANNEL dstChannel, int nSamples)
{
  return true;
}

#endif /* USE_TINY_CC_IMPL */
