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

#ifndef __PCMSAMPLINGRATECONVERSIONPRIMITIVES_HPP__
#define __PCMSAMPLINGRATECONVERSIONPRIMITIVES_HPP__

#include <stdint.h>

template<typename T> static bool convert(T* pSrc, T* pDst, int32_t srcRate, int32_t dstRate, int nSamples = 1)
{
  for(int i=0; i<nSamples; i++){
    if( ((i*dstRate) % srcRate) == 0 ){
      pSrc++;
    } else {
      *pDst++ = *pSrc++;
    }
  }
  return true;
};

class PcmSamplingRateConvert
{
public:
  static bool convert(uint8_t* pSrc, uint8_t* pDst, int32_t srcRate, int32_t dstRate, int nSamples = 1);
  static bool convert24(uint8_t* pSrc, uint8_t* pDst, int32_t srcRate, int32_t dstRate, int nSamples = 1);
  static bool convert(uint16_t* pSrc, uint16_t* pDst, int32_t srcRate, int32_t dstRate, int nSamples = 1);
  static bool convert(uint32_t* pSrc, uint32_t* pDst, int32_t srcRate, int32_t dstRate, int nSamples = 1);
  static bool convert(float* pSrc, float* pDst, int32_t srcRate, int32_t dstRate, int nSamples = 1);
};

#endif /* __PCMSAMPLINGRATECONVERSIONPRIMITIVES_HPP__ */