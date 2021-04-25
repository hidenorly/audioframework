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

  bool bNotHandle = false;

  switch( srcFormat.getEncoding() ){
    case AudioFormat::ENCODING::PCM_8BIT:
      switch( dstEncoding ){
        case AudioFormat::ENCODING::PCM_8BIT:
        case AudioFormat::ENCODING::PCM_UNKNOWN:
          bNotHandle = true;
          break;
        case AudioFormat::ENCODING::PCM_16BIT:
          PcmFormatConvert::convertPcm8ToPcm16( srcRawBuf, reinterpret_cast<uint16_t*>(dstRawBuf), nSrcSamples );
          break;
        case AudioFormat::ENCODING::PCM_24BIT_PACKED:
          PcmFormatConvert::convertPcm8ToPcm24( srcRawBuf, reinterpret_cast<uint8_t*>(dstRawBuf), nSrcSamples );
          break;
        case AudioFormat::ENCODING::PCM_32BIT:
          PcmFormatConvert::convertPcm8ToPcm32( srcRawBuf, reinterpret_cast<uint32_t*>(dstRawBuf), nSrcSamples );
          break;
        case AudioFormat::ENCODING::PCM_FLOAT:
          PcmFormatConvert::convertPcm8ToFloat( srcRawBuf, reinterpret_cast<float*>(dstRawBuf), nSrcSamples );
          break;
      }
      break;

    case AudioFormat::ENCODING::PCM_16BIT:
      switch( dstEncoding ){
        case AudioFormat::ENCODING::PCM_8BIT:
          PcmFormatConvert::convertPcm16ToPcm8( reinterpret_cast<uint16_t*>(srcRawBuf), reinterpret_cast<uint8_t*>(dstRawBuf), nSrcSamples );
          break;
        case AudioFormat::ENCODING::PCM_16BIT:
        case AudioFormat::ENCODING::PCM_UNKNOWN:
          bNotHandle = true;
          break;
        case AudioFormat::ENCODING::PCM_24BIT_PACKED:
          PcmFormatConvert::convertPcm16ToPcm24( reinterpret_cast<uint16_t*>(srcRawBuf), reinterpret_cast<uint8_t*>(dstRawBuf), nSrcSamples );
          break;
        case AudioFormat::ENCODING::PCM_32BIT:
          PcmFormatConvert::convertPcm16ToPcm32( reinterpret_cast<uint16_t*>(srcRawBuf), reinterpret_cast<uint32_t*>(dstRawBuf), nSrcSamples );
          break;
        case AudioFormat::ENCODING::PCM_FLOAT:
          PcmFormatConvert::convertPcm16ToFloat( reinterpret_cast<uint16_t*>(srcRawBuf), reinterpret_cast<float*>(dstRawBuf), nSrcSamples );
          break;
      }
      break;

    case AudioFormat::ENCODING::PCM_24BIT_PACKED:
      switch( dstEncoding ){
        case AudioFormat::ENCODING::PCM_8BIT:
          PcmFormatConvert::convertPcm24ToPcm8( reinterpret_cast<uint8_t*>(srcRawBuf), reinterpret_cast<uint8_t*>(dstRawBuf), nSrcSamples );
          break;
        case AudioFormat::ENCODING::PCM_16BIT:
          PcmFormatConvert::convertPcm24ToPcm16( reinterpret_cast<uint8_t*>(srcRawBuf), reinterpret_cast<uint16_t*>(dstRawBuf), nSrcSamples );
        break;
        case AudioFormat::ENCODING::PCM_UNKNOWN:
        case AudioFormat::ENCODING::PCM_24BIT_PACKED:
          bNotHandle = true;
          break;
        case AudioFormat::ENCODING::PCM_32BIT:
          PcmFormatConvert::convertPcm24ToPcm32( reinterpret_cast<uint8_t*>(srcRawBuf), reinterpret_cast<uint32_t*>(dstRawBuf), nSrcSamples );
          break;
        case AudioFormat::ENCODING::PCM_FLOAT:
          PcmFormatConvert::convertPcm24ToFloat( reinterpret_cast<uint8_t*>(srcRawBuf), reinterpret_cast<float*>(dstRawBuf), nSrcSamples );
          break;
      }
      break;

    case AudioFormat::ENCODING::PCM_32BIT:
      switch( dstEncoding ){
        case AudioFormat::ENCODING::PCM_8BIT:
          PcmFormatConvert::convertPcm32ToPcm8( reinterpret_cast<uint32_t*>(srcRawBuf), reinterpret_cast<uint8_t*>(dstRawBuf), nSrcSamples );
          break;
        case AudioFormat::ENCODING::PCM_16BIT:
          PcmFormatConvert::convertPcm32ToPcm16( reinterpret_cast<uint32_t*>(srcRawBuf), reinterpret_cast<uint16_t*>(dstRawBuf), nSrcSamples );
          break;
        case AudioFormat::ENCODING::PCM_24BIT_PACKED:
          PcmFormatConvert::convertPcm32ToPcm24( reinterpret_cast<uint32_t*>(srcRawBuf), reinterpret_cast<uint8_t*>(dstRawBuf), nSrcSamples );
          break;
        case AudioFormat::ENCODING::PCM_UNKNOWN:
        case AudioFormat::ENCODING::PCM_32BIT:
          bNotHandle = true;
          break;
        case AudioFormat::ENCODING::PCM_FLOAT:
          PcmFormatConvert::convertPcm32ToFloat( reinterpret_cast<uint32_t*>(srcRawBuf), reinterpret_cast<float*>(dstRawBuf), nSrcSamples );
          break;
      }
      break;

    case AudioFormat::ENCODING::PCM_FLOAT:
      switch( dstEncoding ){
        case AudioFormat::ENCODING::PCM_8BIT:
          PcmFormatConvert::convertFloatToPcm8( reinterpret_cast<float*>(srcRawBuf), reinterpret_cast<uint8_t*>(dstRawBuf), nSrcSamples );
          break;
        case AudioFormat::ENCODING::PCM_16BIT:
          PcmFormatConvert::convertFloatToPcm16( reinterpret_cast<float*>(srcRawBuf), reinterpret_cast<uint16_t*>(dstRawBuf), nSrcSamples );
          break;
        case AudioFormat::ENCODING::PCM_24BIT_PACKED:
          PcmFormatConvert::convertFloatToPcm24( reinterpret_cast<float*>(srcRawBuf), reinterpret_cast<uint8_t*>(dstRawBuf), nSrcSamples );
          break;
        case AudioFormat::ENCODING::PCM_32BIT:
          PcmFormatConvert::convertFloatToPcm32( reinterpret_cast<float*>(srcRawBuf), reinterpret_cast<uint32_t*>(dstRawBuf), nSrcSamples );
          break;
        case AudioFormat::ENCODING::PCM_UNKNOWN:
        case AudioFormat::ENCODING::PCM_FLOAT:
          bNotHandle = true;
          break;
      }
      break;

    case AudioFormat::ENCODING::PCM_UNKNOWN:
      bNotHandle = true;
      break;
  }

  if( bNotHandle ){
    dstBuf = srcBuf;
  }

  return dstBuf.getAudioFormat().getEncoding() == dstEncoding;
}

bool AudioFormatAdaptor::samplingRateConversion(AudioBuffer& srcBuf, AudioBuffer& dstBuf, int dstSamplingRate)
{
  return dstBuf.getAudioFormat().getSamplingRate() == dstSamplingRate;
}


bool AudioFormatAdaptor::channelConversion(AudioBuffer& srcBuf, AudioBuffer& dstBuf, AudioFormat::CHANNEL dstChannel)
{
  return dstBuf.getAudioFormat().getChannels() == dstChannel;
}
