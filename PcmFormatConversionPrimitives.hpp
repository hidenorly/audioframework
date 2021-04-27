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

#ifndef __PCMFORMATCONVERSIONPRIMITIVES_HPP__
#define __PCMFORMATCONVERSIONPRIMITIVES_HPP__

#include <stdint.h>

class PcmFormatConvert
{
public:
  // from Pcm8
  static void convertPcm8ToPcm16(uint8_t* pSrc, uint16_t* pDst, int nSamples = 1);
  static void convertPcm8ToPcm24(uint8_t* pSrc, uint8_t* pDst, int nSamples = 1);
  static void convertPcm8ToPcm32(uint8_t* pSrc, uint32_t* pDst, int nSamples = 1);
  static void convertPcm8ToFloat(uint8_t* pSrc, float* pDst, int nSamples = 1);
  static void convertPcm8ToPcm16(uint8_t* pSrc, uint8_t* pDst, int nSamples){ convertPcm8ToPcm16(pSrc, reinterpret_cast<uint16_t*>(pDst), nSamples); }
  static void convertPcm8ToPcm32(uint8_t* pSrc, uint8_t* pDst, int nSamples){ convertPcm8ToPcm32(pSrc, reinterpret_cast<uint32_t*>(pDst), nSamples); }
  static void convertPcm8ToFloat(uint8_t* pSrc, uint8_t* pDst, int nSamples){ convertPcm8ToFloat(pSrc, reinterpret_cast<float*>(pDst), nSamples); }

  // from Pcm16
  static void convertPcm16ToPcm8(uint16_t* pSrc, uint8_t* pDst, int nSamples = 1);
  static void convertPcm16ToPcm24(uint16_t* pSrc, uint8_t* pDst, int nSamples = 1);
  static void convertPcm16ToPcm32(uint16_t* pSrc, uint32_t* pDst, int nSamples = 1);
  static void convertPcm16ToFloat(uint16_t* pSrc, float* pDst, int nSamples = 1);
  static void convertPcm16ToPcm8(uint8_t* pSrc, uint8_t* pDst, int nSamples){ convertPcm16ToPcm8(reinterpret_cast<uint16_t*>(pSrc), pDst, nSamples); }
  static void convertPcm16ToPcm24(uint8_t* pSrc, uint8_t* pDst, int nSamples){ convertPcm16ToPcm24(reinterpret_cast<uint16_t*>(pSrc), pDst, nSamples); }
  static void convertPcm16ToPcm32(uint8_t* pSrc, uint8_t* pDst, int nSamples){ convertPcm16ToPcm32(reinterpret_cast<uint16_t*>(pSrc), reinterpret_cast<uint32_t*>(pDst), nSamples); }
  static void convertPcm16ToFloat(uint8_t* pSrc, uint8_t* pDst, int nSamples){ convertPcm16ToFloat(reinterpret_cast<uint16_t*>(pSrc), reinterpret_cast<float*>(pDst), nSamples); }

  // from Pcm24
  static void convertPcm24ToPcm8(uint8_t* pSrc, uint8_t* pDst, int nSamples = 1);
  static void convertPcm24ToPcm16(uint8_t* pSrc, uint16_t* pDst, int nSamples = 1);
  static void convertPcm24ToPcm32(uint8_t* pSrc, uint32_t* pDst, int nSamples = 1);
  static void convertPcm24ToFloat(uint8_t* pSrc, float* pDst, int nSamples = 1);
  static void convertPcm24ToPcm16(uint8_t* pSrc, uint8_t* pDst, int nSamples){ convertPcm24ToPcm16(pSrc, reinterpret_cast<uint16_t*>(pDst), nSamples); }
  static void convertPcm24ToPcm32(uint8_t* pSrc, uint8_t* pDst, int nSamples){ convertPcm24ToPcm32(pSrc, reinterpret_cast<uint32_t*>(pDst), nSamples); }
  static void convertPcm24ToFloat(uint8_t* pSrc, uint8_t* pDst, int nSamples){ convertPcm24ToFloat(pSrc, reinterpret_cast<float*>(pDst), nSamples); }

  // from Pcm32
  static void convertPcm32ToPcm8(uint32_t* pSrc, uint8_t* pDst, int nSamples = 1);
  static void convertPcm32ToPcm24(uint32_t* pSrc, uint8_t* pDst, int nSamples = 1);
  static void convertPcm32ToPcm16(uint32_t* pSrc, uint16_t* pDst, int nSamples = 1);
  static void convertPcm32ToFloat(uint32_t* pSrc, float* pDst, int nSamples = 1);

  static void convertPcm32ToPcm8(uint8_t* pSrc, uint8_t* pDst, int nSamples){ convertPcm32ToPcm8(reinterpret_cast<uint32_t*>(pSrc), pDst, nSamples); }
  static void convertPcm32ToPcm24(uint8_t* pSrc, uint8_t* pDst, int nSamples){ convertPcm32ToPcm24(reinterpret_cast<uint32_t*>(pSrc), pDst, nSamples); }
  static void convertPcm32ToPcm16(uint8_t* pSrc, uint8_t* pDst, int nSamples){ convertPcm32ToPcm16(reinterpret_cast<uint32_t*>(pSrc), reinterpret_cast<uint16_t*>(pDst), nSamples); }
  static void convertPcm32ToFloat(uint8_t* pSrc, uint8_t* pDst, int nSamples){ convertPcm32ToFloat(reinterpret_cast<uint32_t*>(pSrc), reinterpret_cast<float*>(pDst), nSamples); }


  // from PcmFloat
  static void convertFloatToPcm8(float* pSrc, uint8_t* pDst, int nSamples = 1);
  static void convertFloatToPcm24(float* pSrc, uint8_t* pDst, int nSamples = 1);
  static void convertFloatToPcm16(float* pSrc, uint16_t* pDst, int nSamples = 1);
  static void convertFloatToPcm32(float* pSrc, uint32_t* pDst, int nSamples = 1);

  static void convertFloatToPcm8(uint8_t* pSrc, uint8_t* pDst, int nSamples){ convertFloatToPcm8(reinterpret_cast<float*>(pSrc), pDst, nSamples); }
  static void convertFloatToPcm24(uint8_t* pSrc, uint8_t* pDst, int nSamples){ convertFloatToPcm24(reinterpret_cast<float*>(pSrc), pDst, nSamples); }
  static void convertFloatToPcm16(uint8_t* pSrc, uint8_t* pDst, int nSamples){ convertFloatToPcm16(reinterpret_cast<float*>(pSrc), reinterpret_cast<uint16_t*>(pDst), nSamples); }
  static void convertFloatToPcm32(uint8_t* pSrc, uint8_t* pDst, int nSamples){ convertFloatToPcm32(reinterpret_cast<float*>(pSrc), reinterpret_cast<uint32_t*>(pDst), nSamples); }
};

#endif /* __PCMFORMATCONVERSIONPRIMITIVES_HPP__ */