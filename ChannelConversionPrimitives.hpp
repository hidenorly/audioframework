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

#ifndef __CHANNELCONVERSIONPRIMITIVES_HPP__
#define __CHANNELCONVERSIONPRIMITIVES_HPP__

#include <stdint.h>
#include "AudioFormat.hpp"

class ChannelConverter
{
public:
  static bool convert2to1(uint8_t* pSrc, uint8_t* pDst);
  static bool convert2to4(uint8_t* pSrc, uint8_t* pDst);
  static bool convert2to5(uint8_t* pSrc, uint8_t* pDst);
  static bool convert2to5_1(uint8_t* pSrc, uint8_t* pDst);
  static bool convert2to5_0_2(uint8_t* pSrc, uint8_t* pDst);
  static bool convert2to5_1_2(uint8_t* pSrc, uint8_t* pDst);
  static bool convert2to7_1(uint8_t* pSrc, uint8_t* pDst);

  static bool convert4to1(uint8_t* pSrc, uint8_t* pDst);
  static bool convert4to2(uint8_t* pSrc, uint8_t* pDst);
  static bool convert4to5(uint8_t* pSrc, uint8_t* pDst);
  static bool convert4to5_1(uint8_t* pSrc, uint8_t* pDst);
  static bool convert4to5_0_2(uint8_t* pSrc, uint8_t* pDst);
  static bool convert4to5_1_2(uint8_t* pSrc, uint8_t* pDst);
  static bool convert4to7_1(uint8_t* pSrc, uint8_t* pDst);

  static bool convert5to1(uint8_t* pSrc, uint8_t* pDst);
  static bool convert5to2(uint8_t* pSrc, uint8_t* pDst);
  static bool convert5to4(uint8_t* pSrc, uint8_t* pDst);
  static bool convert5to5_1(uint8_t* pSrc, uint8_t* pDst);
  static bool convert5to5_0_2(uint8_t* pSrc, uint8_t* pDst);
  static bool convert5to5_1_2(uint8_t* pSrc, uint8_t* pDst);
  static bool convert5to7_1(uint8_t* pSrc, uint8_t* pDst);

  static bool convert5_1to1(uint8_t* pSrc, uint8_t* pDst);
  static bool convert5_1to2(uint8_t* pSrc, uint8_t* pDst);
  static bool convert5_1to4(uint8_t* pSrc, uint8_t* pDst);
  static bool convert5_1to5(uint8_t* pSrc, uint8_t* pDst);
  static bool convert5_1to5_0_2(uint8_t* pSrc, uint8_t* pDst);
  static bool convert5_1to5_1_2(uint8_t* pSrc, uint8_t* pDst);
  static bool convert5_1to7_1(uint8_t* pSrc, uint8_t* pDst);

  static bool convert5_0_2to1(uint8_t* pSrc, uint8_t* pDst);
  static bool convert5_0_2to2(uint8_t* pSrc, uint8_t* pDst);
  static bool convert5_0_2to4(uint8_t* pSrc, uint8_t* pDst);
  static bool convert5_0_2to5(uint8_t* pSrc, uint8_t* pDst);
  static bool convert5_0_2to5_1(uint8_t* pSrc, uint8_t* pDst);
  static bool convert5_0_2to5_1_2(uint8_t* pSrc, uint8_t* pDst);
  static bool convert5_0_2to7_1(uint8_t* pSrc, uint8_t* pDst);

  static bool convert5_1_2to1(uint8_t* pSrc, uint8_t* pDst);
  static bool convert5_1_2to2(uint8_t* pSrc, uint8_t* pDst);
  static bool convert5_1_2to4(uint8_t* pSrc, uint8_t* pDst);
  static bool convert5_1_2to5(uint8_t* pSrc, uint8_t* pDst);
  static bool convert5_1_2to5_1(uint8_t* pSrc, uint8_t* pDst);
  static bool convert5_1_2to5_0_2(uint8_t* pSrc, uint8_t* pDst);
  static bool convert5_1_2to7_1(uint8_t* pSrc, uint8_t* pDst);

  static bool convert7_1to1(uint8_t* pSrc, uint8_t* pDst);
  static bool convert7_1to2(uint8_t* pSrc, uint8_t* pDst);
  static bool convert7_1to4(uint8_t* pSrc, uint8_t* pDst);
  static bool convert7_1to5(uint8_t* pSrc, uint8_t* pDst);
  static bool convert7_1to5_1(uint8_t* pSrc, uint8_t* pDst);
  static bool convert7_1to5_0_2(uint8_t* pSrc, uint8_t* pDst);
  static bool convert7_1to5_1_2(uint8_t* pSrc, uint8_t* pDst);

  static bool convert(uint8_t* pSrc, uint8_t* pDst, AudioFormat::CHANNEL srcChannel, AudioFormat::CHANNEL dstChannel, int nSamples = 1);
  static bool convert(uint16_t* pSrc, uint16_t* pDst, AudioFormat::CHANNEL srcChannel, AudioFormat::CHANNEL dstChannel, int nSamples = 1);
  static bool convert16(uint8_t* pSrc, uint8_t* pDst, AudioFormat::CHANNEL srcChannel, AudioFormat::CHANNEL dstChannel, int nSamples = 1){ return convert(reinterpret_cast<uint16_t*>(pSrc), reinterpret_cast<uint16_t*>(pDst), srcChannel, dstChannel, nSamples); };
  static bool convert24(uint8_t* pSrc, uint8_t* pDst, AudioFormat::CHANNEL srcChannel, AudioFormat::CHANNEL dstChannel, int nSamples = 1);
  static bool convert(uint32_t* pSrc, uint32_t* pDst, AudioFormat::CHANNEL srcChannel, AudioFormat::CHANNEL dstChannel, int nSamples = 1);
  static bool convert32(uint8_t* pSrc, uint8_t* pDst, AudioFormat::CHANNEL srcChannel, AudioFormat::CHANNEL dstChannel, int nSamples = 1){ return convert(reinterpret_cast<uint32_t*>(pSrc), reinterpret_cast<uint32_t*>(pDst), srcChannel, dstChannel, nSamples); };
  static bool convert(float* pSrc, float* pDst, AudioFormat::CHANNEL srcChannel, AudioFormat::CHANNEL dstChannel, int nSamples = 1);
  static bool convertFloat(uint8_t* pSrc, uint8_t* pDst, AudioFormat::CHANNEL srcChannel, AudioFormat::CHANNEL dstChannel, int nSamples = 1){ return convert(reinterpret_cast<float*>(pSrc), reinterpret_cast<float*>(pDst), srcChannel, dstChannel, nSamples); };
};

#endif /* __CHANNELCONVERSIONPRIMITIVES_HPP__ */