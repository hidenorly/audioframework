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

#include "AccousticEchoCancelFilterPrimitive.hpp"
#include <algorithm>

#ifndef USE_TINY_AEC_IMPL
  #define USE_TINY_AEC_IMPL 1
#endif /* USE_TINY_AEC_IMPL */

#if USE_TINY_AEC_IMPL

bool AccousticEchoCancelFilterPrimitive::process(int8_t* pSrc, int8_t* pRef, int nSamples)
{
  for(int i=0; i<nSamples; i++){
    *pSrc = (int8_t)(std::max<int16_t>(INT8_MIN, std::min<int16_t>((int16_t)*pSrc - (int16_t)*pRef, INT8_MAX)));
    pSrc++;
    pRef++;
  }
  return true;
}

bool AccousticEchoCancelFilterPrimitive::process24(int8_t* pSrc, int8_t* pRef, int nSamples)
{
  for(int i=0; i<nSamples; i++){
    int32_t src = ((int32_t)(*pSrc) + (int32_t)(*(pSrc+1) << 8) + (int32_t)(*(pSrc+2) << 16));
    int32_t ref = ((int32_t)(*pRef) + (int32_t)(*(pRef+1) << 8) + (int32_t)(*(pRef+2) << 16));
    int32_t tmp = (int32_t)(std::max<int32_t>(-8388608.0f, std::min<int32_t>(src - ref, 8388607.0f)));
    *pSrc++ = tmp & 0xFF;
    *pSrc++ = (tmp & 0xFF00) >> 8;
    *pSrc++ = (tmp & 0xFF0000) >> 16;
    pRef = pRef + 3;
  }
  return true;
}

bool AccousticEchoCancelFilterPrimitive::process(int16_t* pSrc, int16_t* pRef, int nSamples)
{
  for(int i=0; i<nSamples; i++){
    *pSrc = (int16_t)(std::max<int16_t>(INT16_MIN, std::min<int16_t>((int32_t)*pSrc - (int32_t)*pRef, INT16_MAX)));
    pSrc++;
    pRef++;
  }
  return true;
}

bool AccousticEchoCancelFilterPrimitive::process(int32_t* pSrc, int32_t* pRef, int nSamples)
{
  for(int i=0; i<nSamples; i++){
    *pSrc = (int32_t)(std::max<int64_t>(INT32_MIN, std::min<int64_t>((int64_t)*pSrc - (int64_t)*pRef, INT32_MAX)));
    pSrc++;
    pRef++;
  }
  return true;
}

bool AccousticEchoCancelFilterPrimitive::process(float* pSrc, float* pRef, int nSamples)
{
  for(int i=0; i<nSamples; i++){
    *pSrc = (float)(std::max<float>(-1.0f, std::min<float>(*pSrc - *pRef, 1.0f)));
    pSrc++;
    pRef++;
  }
  return true;
}

#endif /* USE_TINY_AEC_IMPL */