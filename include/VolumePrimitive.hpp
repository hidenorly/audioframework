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

#ifndef __VOLUME_PRIMITIVE_HPP__
#define __VOLUME_PRIMITIVE_HPP__

#include <stdint.h>

class VolumePrimitive
{
public:
  static bool volume(int8_t* pRawInBuf, int8_t* pRawOutBuf, float volume, int nChannelSamples = 1);
  static bool volume(int16_t* pRawInBuf, int16_t* pRawOutBuf, float volume, int nChannelSamples = 1);
  static bool volume(int32_t* pRawInBuf, int32_t* pRawOutBuf, float volume, int nChannelSamples = 1);
  static bool volume(float* pRawInBuf, float* pRawOutBuf, float volume, int nChannelSamples = 1);
  static bool volume24(int8_t* pRawInBuf, int8_t* pRawOutBuf, float volume, int nChannelSamples = 1);
};

#endif /* __VOLUME_PRIMITIVE_HPP__ */