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

#include "AudioFormatAdaptor.hpp"
#include "PcmFormatConversionPrimitives.hpp"
#include "PcmSamplingRateConversionPrimitives.hpp"

bool AudioFormatAdaptor::convert(AudioBuffer& srcBuf, AudioBuffer& dstBuf)
{
  AudioFormat srcFormat = srcBuf.getAudioFormat();
  AudioFormat dstFormat = dstBuf.getAudioFormat();

  AudioFormat::ENCODING dstEncoding = dstFormat.getEncoding();
  int dstSamplingRate               = dstFormat.getSamplingRate();
  AudioFormat::CHANNEL dstChannel   = dstFormat.getChannels();

  if( srcFormat.getEncoding() != dstEncoding ){
    encodingConversion(srcBuf, dstBuf, dstEncoding);
    srcBuf = dstBuf;
  }
  if( srcFormat.getSamplingRate() != dstSamplingRate ){
    samplingRateConversion(srcBuf, dstBuf, dstSamplingRate);
    srcBuf = dstBuf;
  }
  if( srcFormat.getChannels() != dstChannel ){
    channelConversion(srcBuf, dstBuf, dstChannel);
    srcBuf = dstBuf;
  }

  return srcBuf.getAudioFormat().equal( dstFormat );
}

bool AudioFormatAdaptor::encodingConversion(AudioBuffer& srcBuf, AudioBuffer& dstBuf, AudioFormat::ENCODING dstEncoding)
{
  AudioFormat srcFormat = srcBuf.getAudioFormat();
  AudioFormat dstFormat(dstEncoding, srcFormat.getSamplingRate(), srcFormat.getChannels() );
  dstBuf.setAudioFormat(dstFormat);

  int nSrcSampleByte = AudioFormat::getSampleByte( srcFormat.getEncoding() );
  int nDstSampleByte = AudioFormat::getSampleByte( dstEncoding );

  uint8_t* srcRawBuf = srcBuf.getRawBufferPointer();
  uint8_t* dstRawBuf = dstBuf.getRawBufferPointer();
  int nSrcSamples = srcBuf.getRawBuffer().size() / nSrcSampleByte;
  int nDstSamples = dstBuf.getRawBuffer().size() / nDstSampleByte;

  assert(nSrcSamples == nDstSamples);

  struct CONVERT_FUNC_TABLE
  {
  public:
    AudioFormat::ENCODING srcEncoding;
    AudioFormat::ENCODING dstEncoding;
    void (*convert)(uint8_t* pSrc, uint8_t* pDst, int nSamples);

    CONVERT_FUNC_TABLE(AudioFormat::ENCODING srcEncoding, AudioFormat::ENCODING dstEncoding, void (*convert)(uint8_t* pSrc, uint8_t* pDst, int nSamples)):srcEncoding(srcEncoding),dstEncoding(dstEncoding),convert(convert){};
  };

  static CONVERT_FUNC_TABLE convertFuncTable[]=
  {
    CONVERT_FUNC_TABLE(AudioFormat::ENCODING::PCM_8BIT, AudioFormat::ENCODING::PCM_16BIT, PcmFormatConvert::convertPcm8ToPcm16),
    CONVERT_FUNC_TABLE(AudioFormat::ENCODING::PCM_8BIT, AudioFormat::ENCODING::PCM_24BIT_PACKED, PcmFormatConvert::convertPcm8ToPcm24),
    CONVERT_FUNC_TABLE(AudioFormat::ENCODING::PCM_8BIT, AudioFormat::ENCODING::PCM_32BIT, PcmFormatConvert::convertPcm8ToPcm32),
    CONVERT_FUNC_TABLE(AudioFormat::ENCODING::PCM_8BIT, AudioFormat::ENCODING::PCM_FLOAT, PcmFormatConvert::convertPcm8ToFloat),

    CONVERT_FUNC_TABLE(AudioFormat::ENCODING::PCM_16BIT, AudioFormat::ENCODING::PCM_8BIT, PcmFormatConvert::convertPcm16ToPcm8),
    CONVERT_FUNC_TABLE(AudioFormat::ENCODING::PCM_16BIT, AudioFormat::ENCODING::PCM_24BIT_PACKED, PcmFormatConvert::convertPcm16ToPcm24),
    CONVERT_FUNC_TABLE(AudioFormat::ENCODING::PCM_16BIT, AudioFormat::ENCODING::PCM_32BIT, PcmFormatConvert::convertPcm16ToPcm32),
    CONVERT_FUNC_TABLE(AudioFormat::ENCODING::PCM_16BIT, AudioFormat::ENCODING::PCM_FLOAT, PcmFormatConvert::convertPcm16ToFloat),

    CONVERT_FUNC_TABLE(AudioFormat::ENCODING::PCM_24BIT_PACKED, AudioFormat::ENCODING::PCM_8BIT, PcmFormatConvert::convertPcm24ToPcm8),
    CONVERT_FUNC_TABLE(AudioFormat::ENCODING::PCM_24BIT_PACKED, AudioFormat::ENCODING::PCM_16BIT, PcmFormatConvert::convertPcm24ToPcm16),
    CONVERT_FUNC_TABLE(AudioFormat::ENCODING::PCM_24BIT_PACKED, AudioFormat::ENCODING::PCM_32BIT, PcmFormatConvert::convertPcm24ToPcm32),
    CONVERT_FUNC_TABLE(AudioFormat::ENCODING::PCM_24BIT_PACKED, AudioFormat::ENCODING::PCM_FLOAT, PcmFormatConvert::convertPcm24ToFloat),

    CONVERT_FUNC_TABLE(AudioFormat::ENCODING::PCM_32BIT, AudioFormat::ENCODING::PCM_8BIT, PcmFormatConvert::convertPcm32ToPcm8),
    CONVERT_FUNC_TABLE(AudioFormat::ENCODING::PCM_32BIT, AudioFormat::ENCODING::PCM_16BIT, PcmFormatConvert::convertPcm32ToPcm16),
    CONVERT_FUNC_TABLE(AudioFormat::ENCODING::PCM_32BIT, AudioFormat::ENCODING::PCM_24BIT_PACKED, PcmFormatConvert::convertPcm32ToPcm24),
    CONVERT_FUNC_TABLE(AudioFormat::ENCODING::PCM_32BIT, AudioFormat::ENCODING::PCM_FLOAT, PcmFormatConvert::convertPcm32ToFloat),

    CONVERT_FUNC_TABLE(AudioFormat::ENCODING::PCM_FLOAT, AudioFormat::ENCODING::PCM_8BIT, PcmFormatConvert::convertFloatToPcm8),
    CONVERT_FUNC_TABLE(AudioFormat::ENCODING::PCM_FLOAT, AudioFormat::ENCODING::PCM_16BIT, PcmFormatConvert::convertFloatToPcm16),
    CONVERT_FUNC_TABLE(AudioFormat::ENCODING::PCM_FLOAT, AudioFormat::ENCODING::PCM_24BIT_PACKED, PcmFormatConvert::convertFloatToPcm24),
    CONVERT_FUNC_TABLE(AudioFormat::ENCODING::PCM_FLOAT, AudioFormat::ENCODING::PCM_32BIT, PcmFormatConvert::convertFloatToPcm32),

    CONVERT_FUNC_TABLE(AudioFormat::ENCODING::PCM_UNKNOWN, AudioFormat::ENCODING::PCM_UNKNOWN, nullptr)
  };

  CONVERT_FUNC_TABLE* pSelectedFunc = nullptr;
  AudioFormat::ENCODING srcEncoding = srcFormat.getEncoding();
  for(int i=0; (convertFuncTable[i].srcEncoding!=AudioFormat::ENCODING::PCM_UNKNOWN) && (convertFuncTable[i].dstEncoding!=AudioFormat::ENCODING::PCM_UNKNOWN) && (convertFuncTable[i].convert != nullptr); i++){
    if( (convertFuncTable[i].srcEncoding == srcEncoding ) && (convertFuncTable[i].dstEncoding == dstEncoding) ){
      pSelectedFunc = &convertFuncTable[i];
      break;
    }
  }

  if( pSelectedFunc && pSelectedFunc->convert ){
    pSelectedFunc->convert( srcRawBuf, dstRawBuf, nSrcSamples );
  } else {
    dstBuf = srcBuf;
  }

  return dstBuf.getAudioFormat().getEncoding() == dstEncoding;
}

bool AudioFormatAdaptor::samplingRateConversion(AudioBuffer& srcBuf, AudioBuffer& dstBuf, int dstSamplingRate)
{
  AudioFormat srcFormat = srcBuf.getAudioFormat();
  int nSrcSampleByte = AudioFormat::getSampleByte( srcFormat.getEncoding() );
  int nSrcSamples = srcBuf.getRawBuffer().size() / nSrcSampleByte;
  uint8_t* srcRawBuf = srcBuf.getRawBufferPointer();

  AudioFormat dstFormat(srcFormat.getEncoding(), dstSamplingRate, srcFormat.getChannels() );
  dstBuf.setAudioFormat(dstFormat);
  dstBuf.resize( (uint32_t) ((float)nSrcSamples * (float)dstSamplingRate / (float)srcFormat.getSamplingRate() + 0.99f));

  int nDstSampleByte = AudioFormat::getSampleByte( dstFormat.getEncoding() );
  uint8_t* dstRawBuf = dstBuf.getRawBufferPointer();
  int nDstSamples = dstBuf.getRawBuffer().size() / nDstSampleByte;

  bool bNotHandle = false;

  switch( srcFormat.getEncoding() ){
    case AudioFormat::ENCODING::PCM_8BIT:
      bNotHandle = !PcmSamplingRateConvert::convert(srcRawBuf, dstRawBuf, srcFormat.getSamplingRate(), dstSamplingRate, nSrcSamples);
      break;
    case AudioFormat::ENCODING::PCM_16BIT:
      bNotHandle = !PcmSamplingRateConvert::convert(reinterpret_cast<uint16_t*>(srcRawBuf), reinterpret_cast<uint16_t*>(dstRawBuf), srcFormat.getSamplingRate(), dstSamplingRate, nSrcSamples);
    case AudioFormat::ENCODING::PCM_32BIT:
      bNotHandle = !PcmSamplingRateConvert::convert(reinterpret_cast<uint32_t*>(srcRawBuf), reinterpret_cast<uint32_t*>(dstRawBuf), srcFormat.getSamplingRate(), dstSamplingRate, nSrcSamples);
    case AudioFormat::ENCODING::PCM_FLOAT:
      bNotHandle = !PcmSamplingRateConvert::convert(reinterpret_cast<float*>(srcRawBuf), reinterpret_cast<float*>(dstRawBuf), srcFormat.getSamplingRate(), dstSamplingRate, nSrcSamples);
      break;

    case AudioFormat::ENCODING::PCM_24BIT_PACKED:
      bNotHandle = !PcmSamplingRateConvert::convert24(srcRawBuf, dstRawBuf, srcFormat.getSamplingRate(), dstSamplingRate, nSrcSamples);
      break;

    case AudioFormat::ENCODING::PCM_UNKNOWN:
      bNotHandle = true;
      break;
  }

  if( bNotHandle ){
    dstBuf = srcBuf;
  }

  return dstBuf.getAudioFormat().getSamplingRate() == dstSamplingRate;
}


bool AudioFormatAdaptor::channelConversion(AudioBuffer& srcBuf, AudioBuffer& dstBuf, AudioFormat::CHANNEL dstChannel)
{
  return dstBuf.getAudioFormat().getChannels() == dstChannel;
}
