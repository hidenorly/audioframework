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

bool VolumePrimitive::volume(int8_t* pRawInBuf, int8_t* pRawOutBuf, std::vector<float> volumes, int nChannelSamples)
{
  int nChannels = volumes.size();
  for(int i=0; i<nChannelSamples; i++){
    int16_t volumed = ((float)(*pRawInBuf++) * volumes[i % nChannels] / 100.0f);
    *pRawOutBuf++ = (int8_t)(std::max<int16_t>(INT8_MIN, std::min<int16_t>(volumed, INT8_MAX)));
  }
  return true;
}

bool VolumePrimitive::volume(int16_t* pRawInBuf, int16_t* pRawOutBuf, std::vector<float> volumes, int nChannelSamples)
{
  int nChannels = volumes.size();

  for(int i=0; i<nChannelSamples; i++){
    int32_t volumed = ((float)(*pRawInBuf++) * volumes[i % nChannels] / 100.0f);
    *pRawOutBuf++ = (int16_t)(std::max<int32_t>(INT16_MIN, std::min<int32_t>(volumed, INT16_MAX)));
  }
  return true;
}

bool VolumePrimitive::volume(int32_t* pRawInBuf, int32_t* pRawOutBuf, std::vector<float> volumes, int nChannelSamples)
{
  int nChannels = volumes.size();
  for(int i=0; i<nChannelSamples; i++){
    float volumed = ((float)(*pRawInBuf++) * volumes[i % nChannels] / 100.0f);
    *pRawOutBuf++ = (int32_t)(std::max<float>((float)INT32_MIN, std::min<float>(volumed, (float)INT32_MAX)));
  }
  return true;
}

bool VolumePrimitive::volume(float* pRawInBuf, float* pRawOutBuf, std::vector<float> volumes, int nChannelSamples)
{
  int nChannels = volumes.size();
  for(int i=0; i<nChannelSamples; i++){
    float volumed = ((float)(*pRawInBuf++) * volumes[i % nChannels] / 100.0f);
    *pRawOutBuf++ = std::max<float>(-1.0f, std::min<float>(volumed, 1.0f));
  }
  return true;
}

bool VolumePrimitive::volume24(int8_t* pRawInBuf, int8_t* pRawOutBuf, std::vector<float> volumes, int nChannelSamples)
{
  int nChannels = volumes.size();
  for(int i=0; i<nChannelSamples; i++){
    float volumed =  ((float)((int32_t)(*pRawInBuf) + (int32_t)(*(pRawInBuf+1) << 8) + (int32_t)(*(pRawInBuf+2) << 16)) * volumes[i % nChannels] / 100.0f);
    int32_t tmp = (int32_t)(std::max<float>(-8388608.0f, std::min<float>(volumed, 8388607.0f)));
    *pRawOutBuf++ = tmp & 0xFF;
    *pRawOutBuf++ = (tmp & 0xFF00) >> 8;
    *pRawOutBuf++ = (tmp & 0xFF0000) >> 16;
    pRawInBuf = pRawInBuf + 3;
  }
  return true;
}

#endif /* USE_TINY_VOLUME_PRIMITIVE_IMPL */