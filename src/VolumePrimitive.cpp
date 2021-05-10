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

#include "VolumePrimitive.hpp"

#ifndef USE_TINY_VOLUME_PRIMITIVE_IMPL
  #define USE_TINY_VOLUME_PRIMITIVE_IMPL 1
#endif /* USE_TINY_VOLUME_PRIMITIVE_IMPL */

#if USE_TINY_VOLUME_PRIMITIVE_IMPL

#include <cstdint>
#include <algorithm>
#include <functional>

bool VolumePrimitive::volume( int8_t* pRawInBuf, int8_t* pRawOutBuf, float volume, int nChannelSamples)
{
  for(int i=0; i<nChannelSamples; i++){
    int16_t volumed = ((float)(*pRawInBuf) * volume / 100.0f);
    *pRawOutBuf = std::max<int16_t>(INT8_MIN, std::min<int16_t>(volumed, INT8_MAX));
    pRawInBuf++;
    pRawOutBuf++;
  }
  return true;
}

bool VolumePrimitive::volume( int16_t* pRawInBuf, int16_t* pRawOutBuf, float volume, int nChannelSamples)
{
  for(int i=0; i<nChannelSamples; i++){
    int32_t volumed = ((float)(*pRawInBuf) * volume / 100.0f);
    *pRawOutBuf = std::max<int32_t>(INT16_MIN, std::min<int32_t>(volumed, INT16_MAX));
    pRawInBuf++;
    pRawOutBuf++;
  }
  return true;
}

bool VolumePrimitive::volume( int32_t* pRawInBuf, int32_t* pRawOutBuf, float volume, int nChannelSamples)
{
  for(int i=0; i<nChannelSamples; i++){
    float volumed = ((float)(*pRawInBuf) * volume / 100.0f);
    *pRawOutBuf = std::max<float>(INT32_MIN, std::min<float>(volumed, INT32_MAX));
    pRawInBuf++;
    pRawOutBuf++;
  }
  return true;
}

bool VolumePrimitive::volume( float* pRawInBuf, float* pRawOutBuf, float volume, int nChannelSamples)
{
  for(int i=0; i<nChannelSamples; i++){
    float volumed = ((float)(*pRawInBuf) * volume / 100.0f);
    *pRawOutBuf = std::max<float>(-1.0f, std::min<float>(volumed, 1.0f));
    pRawInBuf++;
    pRawOutBuf++;
  }
  return true;
}

bool VolumePrimitive::volume24( int8_t* pRawInBuf, int8_t* pRawOutBuf, float volume, int nChannelSamples)
{
  for(int i=0; i<nChannelSamples; i++){
    float volumed =  ((float)((int32_t)(*pRawInBuf) + (int32_t)(*(pRawInBuf+1) << 8) + (int32_t)(*(pRawInBuf+2) << 16)) * volume / 100.0f);
    *pRawOutBuf = std::max<float>(-8388608.0f, std::min<float>(volumed, 8388607.0f));
    pRawInBuf = pRawInBuf + 3;
    pRawOutBuf = pRawOutBuf + 3;
  }
  return true;
}

#endif /* USE_TINY_VOLUME_PRIMITIVE_IMPL */
