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

#include "MixerPrimitive.hpp"

#ifndef USE_TINY_MIXER_PRIMITIVE_IMPL
  #define USE_TINY_MIXER_PRIMITIVE_IMPL 1
#endif /* USE_TINY_MIXER_PRIMITIVE_IMPL */

#if USE_TINY_MIXER_PRIMITIVE_IMPL

#include <cstdint>
#include <algorithm>
#include <functional>

bool MixerPrimitive::mix( int8_t* pRawInBuf1, int8_t* pRawInBuf2, int8_t* pRawOutBuf, int nChannelSamples)
{
  for(int i=0; i<nChannelSamples; i++){
    int16_t mixed = (*pRawInBuf1) + (*pRawInBuf2);
    *pRawOutBuf = std::max<int16_t>(INT8_MIN, std::min<int16_t>(mixed, INT8_MAX));
    pRawInBuf1++;
    pRawInBuf2++;
    pRawOutBuf++;
  }
  return true;
}

bool MixerPrimitive::mix( int16_t* pRawInBuf1, int16_t* pRawInBuf2, int16_t* pRawOutBuf, int nChannelSamples)
{
  for(int i=0; i<nChannelSamples; i++){
    int32_t mixed = (*pRawInBuf1) + (*pRawInBuf2);
    *pRawOutBuf = std::max<int32_t>(INT16_MIN, std::min<int32_t>(mixed, INT16_MAX));
    pRawInBuf1++;
    pRawInBuf2++;
    pRawOutBuf++;
  }
  return true;
}

bool MixerPrimitive::mix( int32_t* pRawInBuf1, int32_t* pRawInBuf2, int32_t* pRawOutBuf, int nChannelSamples)
{
  for(int i=0; i<nChannelSamples; i++){
    int64_t mixed = (*pRawInBuf1) + (*pRawInBuf2);
    *pRawOutBuf = std::max<int64_t>(-INT32_MIN, std::min<int64_t>(mixed, INT32_MAX));
    pRawInBuf1++;
    pRawInBuf2++;
    pRawOutBuf++;
  }
  return true;
}

bool MixerPrimitive::mix( float* pRawInBuf1, float* pRawInBuf2, float* pRawOutBuf, int nChannelSamples)
{
  for(int i=0; i<nChannelSamples; i++){
    float mixed = (*pRawInBuf1) + (*pRawInBuf2);
    *pRawOutBuf = std::max<float>(-1.0f, std::min<float>(mixed, 1.0f));
    pRawInBuf1++;
    pRawInBuf2++;
    pRawOutBuf++;
  }
  return true;
}

bool MixerPrimitive::mix24( int8_t* pRawInBuf1, int8_t* pRawInBuf2, int8_t* pRawOutBuf, int nChannelSamples)
{
  for(int i=0; i<nChannelSamples; i++){
    int32_t mixed = (int32_t)(*pRawInBuf1) + (int32_t)(*(pRawInBuf1+1) << 8) + (int32_t)(*(pRawInBuf1+2) << 16) + (int32_t)(*pRawInBuf2) + (int32_t)(*(pRawInBuf2+1) << 8)+ (int32_t)(*(pRawInBuf2+2) << 16);
    mixed = std::max<int32_t>(-8388608, std::min<int32_t>(mixed, 8388607));
    *pRawOutBuf++ = mixed & 0xFF;
    *pRawOutBuf++ = (mixed & 0xFF00) >> 8;
    *pRawOutBuf++ = (mixed & 0xFF0000) >> 16;
    pRawInBuf1 = pRawInBuf1 + 3;
    pRawInBuf2 = pRawInBuf2 + 3;
  }
  return true;
}

#endif /* USE_TINY_MIXER_PRIMITIVE_IMPL */