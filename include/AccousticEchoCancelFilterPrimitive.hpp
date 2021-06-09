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

#ifndef __AEC_FILTER_PRIMITIVE_HPP__
#define __AEC_FILTER_PRIMITIVE_HPP__

#include "Buffer.hpp"

class AccousticEchoCancelFilterPrimitive
{
public:
  static bool process(int8_t* pSrc, int8_t* pRef, int nSamples = 1);
  static bool process24(int8_t* pSrc, int8_t* pRef, int nSamples = 1);
  static bool process(int16_t* pSrc, int16_t* pRef, int nSamples = 1);
  static bool process(int32_t* pSrc, int32_t* pRef, int nSamples = 1);
  static bool process(float* pSrc, float* pRef, int nSamples = 1);
};

#endif /* __AEC_FILTER_PRIMITIVE_HPP__ */