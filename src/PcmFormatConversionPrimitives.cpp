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

#include "PcmFormatConversionPrimitives.hpp"

// from Pcm8
void PcmFormatConvert::convertPcm8ToPcm16(uint8_t* pSrc, uint16_t* pDst, int nSamples)
{
  for(int i=0; i<nSamples; i++){
    *pDst++ = (uint16_t)(*pSrc++) << 8;
  }
}

void PcmFormatConvert::convertPcm8ToPcm24(uint8_t* pSrc, uint8_t* pDst, int nSamples)
{
  for(int i=0; i<nSamples; i++){
    *pDst++ = 0;
    *pDst++ = 0;
    *pDst++ = *pSrc++;
  }
}

void PcmFormatConvert::convertPcm8ToPcm32(uint8_t* pSrc, uint32_t* pDst, int nSamples)
{
  for(int i=0; i<nSamples; i++){
    *pDst++ = (uint32_t)(*pSrc++) << 24;
  }
}

void PcmFormatConvert::convertPcm8ToFloat(uint8_t* pSrc, float* pDst, int nSamples)
{
  for(int i=0; i<nSamples; i++){
    *pDst++ = ((float)(*pSrc++) - (float)(1<<7) + 1.0f)/(float)(1<<7);
  }
}

// from Pcm16
void PcmFormatConvert::convertPcm16ToPcm8(uint16_t* pSrc, uint8_t* pDst, int nSamples)
{
  for(int i=0; i<nSamples; i++){
    *pDst++ = (uint8_t)((*pSrc++) >> 8);
  }
}

void PcmFormatConvert::convertPcm16ToPcm24(uint16_t* pSrc, uint8_t* pDst, int nSamples)
{
  for(int i=0; i<nSamples; i++){
    *pDst++ = 0;
    *pDst++ = (uint8_t)(*pSrc & 0x00FF);
    *pDst++ = (uint8_t)(((*pSrc++) & 0xFF00) >> 8);
  }
}

void PcmFormatConvert::convertPcm16ToPcm32(uint16_t* pSrc, uint32_t* pDst, int nSamples)
{
  for(int i=0; i<nSamples; i++){
    *pDst++ = (uint32_t)(*pSrc++) << 16;
  }
}

void PcmFormatConvert::convertPcm16ToFloat(uint16_t* pSrc, float* pDst, int nSamples)
{
  for(int i=0; i<nSamples; i++){
    *pDst++ = ((float)(*pSrc++) - (float)(1<<15) + 1.0f)/(float)(1<<15);
  }
}

// from pcm24
void PcmFormatConvert::convertPcm24ToPcm8(uint8_t* pSrc, uint8_t* pDst, int nSamples)
{
  for(int i=0; i<nSamples; i++){
    pSrc++;
    pSrc++;
    *pDst++ = (uint8_t)(*pSrc++);
  }
}

void PcmFormatConvert::convertPcm24ToPcm16(uint8_t* pSrc, uint16_t* pDst, int nSamples)
{
  for(int i=0; i<nSamples; i++){
    pSrc++;
    *pDst = (uint16_t)(*pSrc++);
    *pDst++ = *pDst + ((uint16_t)(*pSrc++) << 8);
  }
}

void PcmFormatConvert::convertPcm24ToPcm32(uint8_t* pSrc, uint32_t* pDst, int nSamples)
{
  for(int i=0; i<nSamples; i++){
    *pDst = (uint32_t)(*pSrc++) << 8;
    *pDst = *pDst + ((uint32_t)(*pSrc++) << 16);
    *pDst++ = *pDst + ((uint32_t)(*pSrc++) << 24);
  }
}

void PcmFormatConvert::convertPcm24ToFloat(uint8_t* pSrc, float* pDst, int nSamples)
{
  for(int i=0; i<nSamples; i++){
    uint32_t tmp = 0;
    convertPcm24ToPcm32(pSrc, &tmp);
    *pDst++ = ((float)tmp - (float)(1<<31) + 1.0f)/(float)(1<<31);
  }
}


// from pcm32
void PcmFormatConvert::convertPcm32ToPcm8(uint32_t* pSrc, uint8_t* pDst, int nSamples)
{
  for(int i=0; i<nSamples; i++){
    *pDst++ = (uint8_t)((*pSrc++) >> 24);
  }
}

void PcmFormatConvert::convertPcm32ToPcm16(uint32_t* pSrc, uint16_t* pDst, int nSamples)
{
  for(int i=0; i<nSamples; i++){
    *pDst++ = (uint16_t)((*pSrc++) >> 16);
  }
}

void PcmFormatConvert::convertPcm32ToPcm24(uint32_t* pSrc, uint8_t* pDst, int nSamples)
{
  for(int i=0; i<nSamples; i++){
    uint32_t tmp = *pSrc++;

    *pDst++ = (uint8_t)((tmp & 0x0000FF00) >> 8);
    *pDst++ = (uint8_t)((tmp & 0x00FF0000) >> 16);
    *pDst++ = (uint8_t)((tmp & 0xFF000000) >> 24);
  }
}

void PcmFormatConvert::convertPcm32ToFloat(uint32_t* pSrc, float* pDst, int nSamples)
{
  for(int i=0; i<nSamples; i++){
    *pDst++ = ((float)(*pSrc++) - (float)(1<<31) + 1.0f)/(float)(1<<31);
  }
}


// from float32
void PcmFormatConvert::convertFloatToPcm8(float* pSrc, uint8_t* pDst, int nSamples)
{
  for(int i=0; i<nSamples; i++){
    *pDst++ = (uint8_t)( ((*pSrc++)+1.0f) * (1<<7) - 1 );
  }
}

void PcmFormatConvert::convertFloatToPcm16(float* pSrc, uint16_t* pDst, int nSamples)
{
  for(int i=0; i<nSamples; i++){
    *pDst++ = (uint16_t)( ((*pSrc++)+1.0f) * (1<<15) - 1 );
  }
}

void PcmFormatConvert::convertFloatToPcm24(float* pSrc, uint8_t* pDst, int nSamples)
{
  for(int i=0; i<nSamples; i++){
    uint32_t tmp = 0;
    convertFloatToPcm32(pSrc, &tmp);

    *pDst++ = (uint8_t)((tmp & 0x0000FF00) >> 8);
    *pDst++ = (uint8_t)((tmp & 0x00FF0000) >> 16);
    *pDst++ = (uint8_t)((tmp & 0xFF000000) >> 24);
  }
}

void PcmFormatConvert::convertFloatToPcm32(float* pSrc, uint32_t* pDst, int nSamples)
{
  for(int i=0; i<nSamples; i++){
    *pDst++ = (uint32_t)( ((*pSrc++)+1.0f) * (1<<31) - 1 );
  }
}
