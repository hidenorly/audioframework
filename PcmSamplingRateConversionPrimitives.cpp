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

#include "PcmSamplingRateConversionPrimitives.hpp"

// TODO: rewrite with template
// DO NOT USE THIS TINY SRC in the production device
// SHOULD REPLACE WITH HIGH QUALITY SRC

#ifndef USE_TINY_SRC_IMPL
  #define USE_TINY_SRC_IMPL 1
#endif /* USE_TINY_SRC_IMPL */

#if USE_TINY_SRC_IMPL

bool PcmSamplingRateConvert::convert(uint8_t* pSrc, uint8_t* pDst, int32_t srcRate, int32_t dstRate, int nSamples)
{
  if( dstRate < srcRate ){
    for(int i=0; i<nSamples; i++){
      if( ((i*dstRate) % srcRate) == 0 ){
        pSrc++;
      } else {
        *pDst++ = *pSrc++;
      }
    }
  } else {
    for(int i=0; i<nSamples; i++){
      *pDst++ = *pSrc;
      if( ((i*srcRate) % dstRate) == 0 ){
        pSrc++;
      }
    }
  }
  return true;
}

bool PcmSamplingRateConvert::convert24(uint8_t* pSrc, uint8_t* pDst, int32_t srcRate, int32_t dstRate, int nSamples)
{
  if( dstRate < srcRate ){
    for(int i=0; i<nSamples; i++){
      if( ((i*dstRate) % srcRate) == 0 ){
        pSrc++;
        pSrc++;
        pSrc++;
      } else {
        *pDst++ = *pSrc++;
        *pDst++ = *pSrc++;
        *pDst++ = *pSrc++;
      }
    }
  } else {
    for(int i=0; i<nSamples; i++){
      *pDst++ = *pSrc;
      *pDst++ = *(pSrc+1);
      *pDst++ = *(pSrc+2);
      if( ((i*srcRate) % dstRate) == 0 ){
        pSrc++;
        pSrc++;
        pSrc++;
      }
    }
  }
  return true;
}


bool PcmSamplingRateConvert::convert(uint16_t* pSrc, uint16_t* pDst, int32_t srcRate, int32_t dstRate, int nSamples)
{
  if( dstRate < srcRate ){
    for(int i=0; i<nSamples; i++){
      if( ((i*dstRate) % srcRate) == 0 ){
        pSrc++;
      } else {
        *pDst++ = *pSrc++;
      }
    }
  } else {
    for(int i=0; i<nSamples; i++){
      *pDst++ = *pSrc;
      if( ((i*srcRate) % dstRate) == 0 ){
        pSrc++;
      }
    }
  }
  return true;
}

bool PcmSamplingRateConvert::convert(uint32_t* pSrc, uint32_t* pDst, int32_t srcRate, int32_t dstRate, int nSamples)
{
  if( dstRate < srcRate ){
    for(int i=0; i<nSamples; i++){
      if( ((i*dstRate) % srcRate) == 0 ){
        pSrc++;
      } else {
        *pDst++ = *pSrc++;
      }
    }
  } else {
    for(int i=0; i<nSamples; i++){
      *pDst++ = *pSrc;
      if( ((i*srcRate) % dstRate) == 0 ){
        pSrc++;
      }
    }
  }
  return true;
}

bool PcmSamplingRateConvert::convert(float* pSrc, float* pDst, int32_t srcRate, int32_t dstRate, int nSamples)
{
  if( dstRate < srcRate ){
    for(int i=0; i<nSamples; i++){
      if( ((i*dstRate) % srcRate) == 0 ){
        pSrc++;
      } else {
        *pDst++ = *pSrc++;
      }
    }
  } else {
    for(int i=0; i<nSamples; i++){
      *pDst++ = *pSrc;
      if( ((i*srcRate) % dstRate) == 0 ){
        pSrc++;
      }
    }
  }
  return true;
}

#endif /* USE_TINY_SRC_IMPL */
