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

bool MixerPrimitive::mix( int8_t* pRawInBuf1, int8_t* pRawInBuf2, int8_t* pRawOutBuf, int nChannelSamples)
{
  return true;
}

bool MixerPrimitive::mix( int16_t* pRawInBuf1, int16_t* pRawInBuf2, int16_t* pRawOutBuf, int nChannelSamples)
{
  return true;
}

bool MixerPrimitive::mix( int32_t* pRawInBuf1, int32_t* pRawInBuf2, int32_t* pRawOutBuf, int nChannelSamples)
{
  return true;
}

bool MixerPrimitive::mix( float* pRawInBuf1, float* pRawInBuf2, float* pRawOutBuf, int nChannelSamples)
{
  return true;
}

bool MixerPrimitive::mix24( int8_t* pRawInBuf1, int8_t* pRawInBuf2, int8_t* pRawOutBuf, int nChannelSamples)
{
  return true;
}

#endif /* USE_TINY_MIXER_PRIMITIVE_IMPL */