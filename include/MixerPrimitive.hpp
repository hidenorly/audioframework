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

#ifndef __MIXER_PRIMITIVE_HPP__
#define __MIXER_PRIMITIVE_HPP__

#include <stdint.h>

class MixerPrimitive
{
public:
  static bool mix( int8_t* pRawInBuf1, int8_t* pRawInBuf2, int8_t* pRawOutBuf, int nChannelSamples);
  static bool mix( int16_t* pRawInBuf1, int16_t* pRawInBuf2, int16_t* pRawOutBuf, int nChannelSamples);
  static bool mix( int32_t* pRawInBuf1, int32_t* pRawInBuf2, int32_t* pRawOutBuf, int nChannelSamples);
  static bool mix( float* pRawInBuf1, float* pRawInBuf2, float* pRawOutBuf, int nChannelSamples);
  static bool mix24( int8_t* pRawInBuf1, int8_t* pRawInBuf2, int8_t* pRawOutBuf, int nChannelSamples);
};

#endif /* __MIXER_PRIMITIVE_HPP__ */