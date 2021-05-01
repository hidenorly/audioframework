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
  int srcSamplingRate = srcFormat.getSamplingRate();

  AudioFormat dstFormat(srcFormat.getEncoding(), dstSamplingRate, srcFormat.getChannels() );
  dstBuf.setAudioFormat(dstFormat);
  dstBuf.resize( (uint32_t) ((float)nSrcSamples * (float)dstSamplingRate / (float)srcFormat.getSamplingRate() + 0.99f));

  int nDstSampleByte = AudioFormat::getSampleByte( dstFormat.getEncoding() );
  uint8_t* dstRawBuf = dstBuf.getRawBufferPointer();
  int nDstSamples = dstBuf.getRawBuffer().size() / nDstSampleByte;

  int nSamples = (srcSamplingRate > dstSamplingRate) ? nSrcSamples : nDstSamples;

  bool bHandled = false;

  switch( srcFormat.getEncoding() ){
    case AudioFormat::ENCODING::PCM_8BIT:
      bHandled = PcmSamplingRateConvert::convert(srcRawBuf, dstRawBuf, srcFormat.getSamplingRate(), dstSamplingRate, nSamples);
      break;
    case AudioFormat::ENCODING::PCM_16BIT:
      bHandled = PcmSamplingRateConvert::convert(reinterpret_cast<uint16_t*>(srcRawBuf), reinterpret_cast<uint16_t*>(dstRawBuf), srcFormat.getSamplingRate(), dstSamplingRate, nSamples);
      break;
    case AudioFormat::ENCODING::PCM_32BIT:
      bHandled = PcmSamplingRateConvert::convert(reinterpret_cast<uint32_t*>(srcRawBuf), reinterpret_cast<uint32_t*>(dstRawBuf), srcFormat.getSamplingRate(), dstSamplingRate, nSamples);
      break;
    case AudioFormat::ENCODING::PCM_FLOAT:
      bHandled = PcmSamplingRateConvert::convert(reinterpret_cast<float*>(srcRawBuf), reinterpret_cast<float*>(dstRawBuf), srcFormat.getSamplingRate(), dstSamplingRate, nSamples);
      break;
    case AudioFormat::ENCODING::PCM_24BIT_PACKED:
      bHandled = PcmSamplingRateConvert::convert24(srcRawBuf, dstRawBuf, srcFormat.getSamplingRate(), dstSamplingRate, nSamples);
      break;
    case AudioFormat::ENCODING::PCM_UNKNOWN:
      bHandled = false;
      break;
  }

  if( !bHandled ){
    dstBuf = srcBuf;
  }

  return dstBuf.getAudioFormat().getSamplingRate() == dstSamplingRate;
}


bool AudioFormatAdaptor::channelConversion(AudioBuffer& srcBuf, AudioBuffer& dstBuf, AudioFormat::CHANNEL dstChannel)
{
  AudioFormat srcFormat = srcBuf.getAudioFormat();
  AudioFormat dstFormat(srcFormat.getEncoding(), srcFormat.getSamplingRate(), dstChannel );
  dstBuf.setAudioFormat(dstFormat);

  int nSrcSamples = srcBuf.getSamples();
  int nDstSamples = dstBuf.getSamples();

  assert(nSrcSamples == nDstSamples);

  struct ChannelMapList
  {
    AudioFormat::CH srcCh;
    AudioFormat::CH dstCh;
    ChannelMapList(AudioFormat::CH srcCh, AudioFormat::CH dstCh):srcCh(srcCh), dstCh(dstCh){};
  };

  struct CONVERT_CH_TABLE
  {
  public:
    AudioFormat::CHANNEL srcChannel;
    AudioFormat::CHANNEL dstChannel;
    std::vector<ChannelMapList> chMapList;

    CONVERT_CH_TABLE(AudioFormat::CHANNEL srcChannel, AudioFormat::CHANNEL dstChannel, std::vector<ChannelMapList> chMapList):srcChannel(srcChannel),dstChannel(dstChannel),chMapList(chMapList){};
  };

  static CONVERT_CH_TABLE convertChTable[]=
  {
    CONVERT_CH_TABLE(AudioFormat::CHANNEL::CHANNEL_MONO, AudioFormat::CHANNEL::CHANNEL_STEREO, std::vector<ChannelMapList>{ 
      {AudioFormat::CH::L, AudioFormat::CH::MONO}, 
      {AudioFormat::CH::R, AudioFormat::CH::MONO} }),
    CONVERT_CH_TABLE(AudioFormat::CHANNEL::CHANNEL_MONO, AudioFormat::CHANNEL::CHANNEL_4CH, std::vector<ChannelMapList>{ 
      {AudioFormat::CH::L, AudioFormat::CH::MONO}, 
      {AudioFormat::CH::R, AudioFormat::CH::MONO},
      {AudioFormat::CH::SL, AudioFormat::CH::MONO}, 
      {AudioFormat::CH::SR, AudioFormat::CH::MONO} }),
    CONVERT_CH_TABLE(AudioFormat::CHANNEL::CHANNEL_MONO, AudioFormat::CHANNEL::CHANNEL_5CH, std::vector<ChannelMapList>{ 
      {AudioFormat::CH::L, AudioFormat::CH::MONO}, 
      {AudioFormat::CH::R, AudioFormat::CH::MONO},
      {AudioFormat::CH::SL, AudioFormat::CH::MONO}, 
      {AudioFormat::CH::SR, AudioFormat::CH::MONO}, 
      {AudioFormat::CH::C, AudioFormat::CH::MONO} }),
    CONVERT_CH_TABLE(AudioFormat::CHANNEL::CHANNEL_MONO, AudioFormat::CHANNEL::CHANNEL_5_1CH, std::vector<ChannelMapList>{ 
      {AudioFormat::CH::L, AudioFormat::CH::MONO}, 
      {AudioFormat::CH::R, AudioFormat::CH::MONO},
      {AudioFormat::CH::SL, AudioFormat::CH::MONO}, 
      {AudioFormat::CH::SR, AudioFormat::CH::MONO}, 
      {AudioFormat::CH::C, AudioFormat::CH::MONO}, 
      {AudioFormat::CH::SW, AudioFormat::CH::MONO} }),
    CONVERT_CH_TABLE(AudioFormat::CHANNEL::CHANNEL_MONO, AudioFormat::CHANNEL::CHANNEL_5_0_2CH, std::vector<ChannelMapList>{ 
      {AudioFormat::CH::L, AudioFormat::CH::MONO}, 
      {AudioFormat::CH::R, AudioFormat::CH::MONO},
      {AudioFormat::CH::SL, AudioFormat::CH::MONO}, 
      {AudioFormat::CH::SR, AudioFormat::CH::MONO}, 
      {AudioFormat::CH::C, AudioFormat::CH::MONO}, 
      {AudioFormat::CH::SBL, AudioFormat::CH::MONO}, 
      {AudioFormat::CH::SBR, AudioFormat::CH::MONO} }),
    CONVERT_CH_TABLE(AudioFormat::CHANNEL::CHANNEL_MONO, AudioFormat::CHANNEL::CHANNEL_5_1_2CH, std::vector<ChannelMapList>{ 
      {AudioFormat::CH::L, AudioFormat::CH::MONO}, 
      {AudioFormat::CH::R, AudioFormat::CH::MONO},
      {AudioFormat::CH::SL, AudioFormat::CH::MONO}, 
      {AudioFormat::CH::SR, AudioFormat::CH::MONO}, 
      {AudioFormat::CH::C, AudioFormat::CH::MONO}, 
      {AudioFormat::CH::SW, AudioFormat::CH::MONO}, 
      {AudioFormat::CH::SBL, AudioFormat::CH::MONO}, 
      {AudioFormat::CH::SBR, AudioFormat::CH::MONO} }),
    CONVERT_CH_TABLE(AudioFormat::CHANNEL::CHANNEL_MONO, AudioFormat::CHANNEL::CHANNEL_7_1CH, std::vector<ChannelMapList>{ 
      {AudioFormat::CH::L, AudioFormat::CH::MONO}, 
      {AudioFormat::CH::R, AudioFormat::CH::MONO},
      {AudioFormat::CH::SL, AudioFormat::CH::MONO}, 
      {AudioFormat::CH::SR, AudioFormat::CH::MONO}, 
      {AudioFormat::CH::C, AudioFormat::CH::MONO}, 
      {AudioFormat::CH::SW, AudioFormat::CH::MONO}, 
      {AudioFormat::CH::SBL, AudioFormat::CH::MONO}, 
      {AudioFormat::CH::SBR, AudioFormat::CH::MONO} }),

    CONVERT_CH_TABLE(AudioFormat::CHANNEL::CHANNEL_STEREO, AudioFormat::CHANNEL::CHANNEL_MONO, std::vector<ChannelMapList>{ 
      {AudioFormat::CH::MONO, AudioFormat::CH::L}, 
      {AudioFormat::CH::MONO, AudioFormat::CH::R} }),
    CONVERT_CH_TABLE(AudioFormat::CHANNEL::CHANNEL_STEREO, AudioFormat::CHANNEL::CHANNEL_4CH, std::vector<ChannelMapList>{ 
      {AudioFormat::CH::L, AudioFormat::CH::L}, 
      {AudioFormat::CH::R, AudioFormat::CH::R},
      {AudioFormat::CH::SL, AudioFormat::CH::L}, 
      {AudioFormat::CH::SR, AudioFormat::CH::R} }),
    CONVERT_CH_TABLE(AudioFormat::CHANNEL::CHANNEL_STEREO, AudioFormat::CHANNEL::CHANNEL_5CH, std::vector<ChannelMapList>{ 
      {AudioFormat::CH::L, AudioFormat::CH::L}, 
      {AudioFormat::CH::R, AudioFormat::CH::R},
      {AudioFormat::CH::SL, AudioFormat::CH::L}, 
      {AudioFormat::CH::SR, AudioFormat::CH::R},
      {AudioFormat::CH::C, AudioFormat::CH::L},
      {AudioFormat::CH::C, AudioFormat::CH::R} }),
    CONVERT_CH_TABLE(AudioFormat::CHANNEL::CHANNEL_STEREO, AudioFormat::CHANNEL::CHANNEL_5_1CH, std::vector<ChannelMapList>{ 
      {AudioFormat::CH::L, AudioFormat::CH::L}, 
      {AudioFormat::CH::R, AudioFormat::CH::R},
      {AudioFormat::CH::SL, AudioFormat::CH::L}, 
      {AudioFormat::CH::SR, AudioFormat::CH::R},
      {AudioFormat::CH::C, AudioFormat::CH::L},
      {AudioFormat::CH::C, AudioFormat::CH::R},
      {AudioFormat::CH::SW, AudioFormat::CH::L},
      {AudioFormat::CH::SW, AudioFormat::CH::R} }),
    CONVERT_CH_TABLE(AudioFormat::CHANNEL::CHANNEL_STEREO, AudioFormat::CHANNEL::CHANNEL_5_0_2CH, std::vector<ChannelMapList>{ 
      {AudioFormat::CH::L, AudioFormat::CH::L}, 
      {AudioFormat::CH::R, AudioFormat::CH::R},
      {AudioFormat::CH::SL, AudioFormat::CH::L}, 
      {AudioFormat::CH::SR, AudioFormat::CH::R},
      {AudioFormat::CH::C, AudioFormat::CH::L},
      {AudioFormat::CH::C, AudioFormat::CH::R},
      {AudioFormat::CH::SBL, AudioFormat::CH::L},
      {AudioFormat::CH::SBR, AudioFormat::CH::R} }),
    CONVERT_CH_TABLE(AudioFormat::CHANNEL::CHANNEL_STEREO, AudioFormat::CHANNEL::CHANNEL_5_1_2CH, std::vector<ChannelMapList>{ 
      {AudioFormat::CH::L, AudioFormat::CH::L}, 
      {AudioFormat::CH::R, AudioFormat::CH::R},
      {AudioFormat::CH::SL, AudioFormat::CH::L}, 
      {AudioFormat::CH::SR, AudioFormat::CH::R},
      {AudioFormat::CH::C, AudioFormat::CH::L},
      {AudioFormat::CH::C, AudioFormat::CH::R},
      {AudioFormat::CH::SW, AudioFormat::CH::L}, 
      {AudioFormat::CH::SW, AudioFormat::CH::R}, 
      {AudioFormat::CH::SBL, AudioFormat::CH::L},
      {AudioFormat::CH::SBR, AudioFormat::CH::R} }),
    CONVERT_CH_TABLE(AudioFormat::CHANNEL::CHANNEL_STEREO, AudioFormat::CHANNEL::CHANNEL_7_1CH, std::vector<ChannelMapList>{ 
      {AudioFormat::CH::L, AudioFormat::CH::L}, 
      {AudioFormat::CH::R, AudioFormat::CH::R},
      {AudioFormat::CH::SL, AudioFormat::CH::L}, 
      {AudioFormat::CH::SR, AudioFormat::CH::R},
      {AudioFormat::CH::C, AudioFormat::CH::L},
      {AudioFormat::CH::C, AudioFormat::CH::R},
      {AudioFormat::CH::SW, AudioFormat::CH::L}, 
      {AudioFormat::CH::SW, AudioFormat::CH::R}, 
      {AudioFormat::CH::SBL, AudioFormat::CH::L},
      {AudioFormat::CH::SBR, AudioFormat::CH::R} }),

    CONVERT_CH_TABLE(AudioFormat::CHANNEL::CHANNEL_4CH, AudioFormat::CHANNEL::CHANNEL_MONO, std::vector<ChannelMapList>{ 
      {AudioFormat::CH::MONO, AudioFormat::CH::SL}, 
      {AudioFormat::CH::MONO, AudioFormat::CH::SR},
      {AudioFormat::CH::MONO, AudioFormat::CH::L}, 
      {AudioFormat::CH::MONO, AudioFormat::CH::R} }),
    CONVERT_CH_TABLE(AudioFormat::CHANNEL::CHANNEL_4CH, AudioFormat::CHANNEL::CHANNEL_STEREO, std::vector<ChannelMapList>{ 
      {AudioFormat::CH::L, AudioFormat::CH::SL}, 
      {AudioFormat::CH::R, AudioFormat::CH::SR},
      {AudioFormat::CH::L, AudioFormat::CH::L}, 
      {AudioFormat::CH::R, AudioFormat::CH::R} }),
    CONVERT_CH_TABLE(AudioFormat::CHANNEL::CHANNEL_4CH, AudioFormat::CHANNEL::CHANNEL_5CH, std::vector<ChannelMapList>{ 
      {AudioFormat::CH::L, AudioFormat::CH::L}, 
      {AudioFormat::CH::R, AudioFormat::CH::R},
      {AudioFormat::CH::SL, AudioFormat::CH::SL}, 
      {AudioFormat::CH::SR, AudioFormat::CH::SR},
      {AudioFormat::CH::C, AudioFormat::CH::SL}, 
      {AudioFormat::CH::C, AudioFormat::CH::SR},
      {AudioFormat::CH::C, AudioFormat::CH::L}, 
      {AudioFormat::CH::C, AudioFormat::CH::R},}),
    CONVERT_CH_TABLE(AudioFormat::CHANNEL::CHANNEL_4CH, AudioFormat::CHANNEL::CHANNEL_5_1CH, std::vector<ChannelMapList>{ 
      {AudioFormat::CH::L, AudioFormat::CH::L}, 
      {AudioFormat::CH::R, AudioFormat::CH::R},
      {AudioFormat::CH::SL, AudioFormat::CH::SL}, 
      {AudioFormat::CH::SR, AudioFormat::CH::SR},
      {AudioFormat::CH::C, AudioFormat::CH::SL}, 
      {AudioFormat::CH::C, AudioFormat::CH::SR},
      {AudioFormat::CH::C, AudioFormat::CH::L}, 
      {AudioFormat::CH::C, AudioFormat::CH::R},
      {AudioFormat::CH::SW, AudioFormat::CH::SL}, 
      {AudioFormat::CH::SW, AudioFormat::CH::SR},
      {AudioFormat::CH::SW, AudioFormat::CH::L}, 
      {AudioFormat::CH::SW, AudioFormat::CH::R},}),
    CONVERT_CH_TABLE(AudioFormat::CHANNEL::CHANNEL_4CH, AudioFormat::CHANNEL::CHANNEL_5_0_2CH, std::vector<ChannelMapList>{ 
      {AudioFormat::CH::L, AudioFormat::CH::L}, 
      {AudioFormat::CH::R, AudioFormat::CH::R},
      {AudioFormat::CH::SL, AudioFormat::CH::SL}, 
      {AudioFormat::CH::SR, AudioFormat::CH::SR},
      {AudioFormat::CH::C, AudioFormat::CH::SL}, 
      {AudioFormat::CH::C, AudioFormat::CH::SR},
      {AudioFormat::CH::C, AudioFormat::CH::L}, 
      {AudioFormat::CH::C, AudioFormat::CH::R},
      {AudioFormat::CH::SBL, AudioFormat::CH::SL}, 
      {AudioFormat::CH::SBL, AudioFormat::CH::L}, 
      {AudioFormat::CH::SBR, AudioFormat::CH::SR},
      {AudioFormat::CH::SBR, AudioFormat::CH::R},}),
    CONVERT_CH_TABLE(AudioFormat::CHANNEL::CHANNEL_4CH, AudioFormat::CHANNEL::CHANNEL_5_1_2CH, std::vector<ChannelMapList>{ 
      {AudioFormat::CH::L, AudioFormat::CH::L}, 
      {AudioFormat::CH::R, AudioFormat::CH::R},
      {AudioFormat::CH::SL, AudioFormat::CH::SL}, 
      {AudioFormat::CH::SR, AudioFormat::CH::SR},
      {AudioFormat::CH::C, AudioFormat::CH::SL}, 
      {AudioFormat::CH::C, AudioFormat::CH::SR},
      {AudioFormat::CH::C, AudioFormat::CH::L}, 
      {AudioFormat::CH::C, AudioFormat::CH::R},
      {AudioFormat::CH::SBL, AudioFormat::CH::SL}, 
      {AudioFormat::CH::SBL, AudioFormat::CH::L}, 
      {AudioFormat::CH::SBR, AudioFormat::CH::SR},
      {AudioFormat::CH::SBR, AudioFormat::CH::R},
      {AudioFormat::CH::SW, AudioFormat::CH::SL}, 
      {AudioFormat::CH::SW, AudioFormat::CH::SR},
      {AudioFormat::CH::SW, AudioFormat::CH::L}, 
      {AudioFormat::CH::SW, AudioFormat::CH::R},}),
    CONVERT_CH_TABLE(AudioFormat::CHANNEL::CHANNEL_4CH, AudioFormat::CHANNEL::CHANNEL_7_1CH, std::vector<ChannelMapList>{ 
      {AudioFormat::CH::L, AudioFormat::CH::L}, 
      {AudioFormat::CH::R, AudioFormat::CH::R},
      {AudioFormat::CH::SL, AudioFormat::CH::SL}, 
      {AudioFormat::CH::SR, AudioFormat::CH::SR},
      {AudioFormat::CH::C, AudioFormat::CH::SL}, 
      {AudioFormat::CH::C, AudioFormat::CH::SR},
      {AudioFormat::CH::C, AudioFormat::CH::L}, 
      {AudioFormat::CH::C, AudioFormat::CH::R},
      {AudioFormat::CH::SBL, AudioFormat::CH::SL}, 
      {AudioFormat::CH::SBL, AudioFormat::CH::L}, 
      {AudioFormat::CH::SBR, AudioFormat::CH::SR},
      {AudioFormat::CH::SBR, AudioFormat::CH::R},
      {AudioFormat::CH::SW, AudioFormat::CH::SL}, 
      {AudioFormat::CH::SW, AudioFormat::CH::SR},
      {AudioFormat::CH::SW, AudioFormat::CH::L}, 
      {AudioFormat::CH::SW, AudioFormat::CH::R},}),

    CONVERT_CH_TABLE(AudioFormat::CHANNEL::CHANNEL_5CH, AudioFormat::CHANNEL::CHANNEL_MONO, std::vector<ChannelMapList>{ 
      {AudioFormat::CH::MONO, AudioFormat::CH::SL}, 
      {AudioFormat::CH::MONO, AudioFormat::CH::SR},
      {AudioFormat::CH::MONO, AudioFormat::CH::C},
      {AudioFormat::CH::MONO, AudioFormat::CH::L}, 
      {AudioFormat::CH::MONO, AudioFormat::CH::R} }),
    CONVERT_CH_TABLE(AudioFormat::CHANNEL::CHANNEL_5CH, AudioFormat::CHANNEL::CHANNEL_STEREO, std::vector<ChannelMapList>{ 
      {AudioFormat::CH::L, AudioFormat::CH::SL}, 
      {AudioFormat::CH::R, AudioFormat::CH::SR},
      {AudioFormat::CH::L, AudioFormat::CH::L}, 
      {AudioFormat::CH::R, AudioFormat::CH::R} }), // Ignore C
    CONVERT_CH_TABLE(AudioFormat::CHANNEL::CHANNEL_5CH, AudioFormat::CHANNEL::CHANNEL_4CH, std::vector<ChannelMapList>{ 
      {AudioFormat::CH::L, AudioFormat::CH::L}, 
      {AudioFormat::CH::R, AudioFormat::CH::R},
      {AudioFormat::CH::SL, AudioFormat::CH::SL}, 
      {AudioFormat::CH::SR, AudioFormat::CH::SR},}), // Ignore C
    CONVERT_CH_TABLE(AudioFormat::CHANNEL::CHANNEL_5CH, AudioFormat::CHANNEL::CHANNEL_5_1CH, std::vector<ChannelMapList>{ 
      {AudioFormat::CH::L, AudioFormat::CH::L}, 
      {AudioFormat::CH::R, AudioFormat::CH::R},
      {AudioFormat::CH::SL, AudioFormat::CH::SL}, 
      {AudioFormat::CH::SR, AudioFormat::CH::SR},
      {AudioFormat::CH::C, AudioFormat::CH::C}, 
      {AudioFormat::CH::SW, AudioFormat::CH::SL}, 
      {AudioFormat::CH::SW, AudioFormat::CH::SR},
      {AudioFormat::CH::SW, AudioFormat::CH::C},
      {AudioFormat::CH::SW, AudioFormat::CH::L}, 
      {AudioFormat::CH::SW, AudioFormat::CH::R},}),
    CONVERT_CH_TABLE(AudioFormat::CHANNEL::CHANNEL_5CH, AudioFormat::CHANNEL::CHANNEL_5_0_2CH, std::vector<ChannelMapList>{ 
      {AudioFormat::CH::L, AudioFormat::CH::L}, 
      {AudioFormat::CH::R, AudioFormat::CH::R},
      {AudioFormat::CH::SL, AudioFormat::CH::SL}, 
      {AudioFormat::CH::SR, AudioFormat::CH::SR},
      {AudioFormat::CH::C, AudioFormat::CH::C}, 
      {AudioFormat::CH::SBL, AudioFormat::CH::SL}, 
      {AudioFormat::CH::SBL, AudioFormat::CH::L}, 
      {AudioFormat::CH::SBR, AudioFormat::CH::SR},
      {AudioFormat::CH::SBR, AudioFormat::CH::R},}),
    CONVERT_CH_TABLE(AudioFormat::CHANNEL::CHANNEL_5CH, AudioFormat::CHANNEL::CHANNEL_5_1_2CH, std::vector<ChannelMapList>{ 
      {AudioFormat::CH::L, AudioFormat::CH::L}, 
      {AudioFormat::CH::R, AudioFormat::CH::R},
      {AudioFormat::CH::SL, AudioFormat::CH::SL}, 
      {AudioFormat::CH::SR, AudioFormat::CH::SR},
      {AudioFormat::CH::C, AudioFormat::CH::C}, 
      {AudioFormat::CH::SW, AudioFormat::CH::SL}, 
      {AudioFormat::CH::SW, AudioFormat::CH::SR},
      {AudioFormat::CH::SW, AudioFormat::CH::C},
      {AudioFormat::CH::SW, AudioFormat::CH::L}, 
      {AudioFormat::CH::SW, AudioFormat::CH::R},
      {AudioFormat::CH::SBL, AudioFormat::CH::SL}, 
      {AudioFormat::CH::SBL, AudioFormat::CH::L}, 
      {AudioFormat::CH::SBR, AudioFormat::CH::SR},
      {AudioFormat::CH::SBR, AudioFormat::CH::R},}),
    CONVERT_CH_TABLE(AudioFormat::CHANNEL::CHANNEL_5CH, AudioFormat::CHANNEL::CHANNEL_7_1CH, std::vector<ChannelMapList>{ 
      {AudioFormat::CH::L, AudioFormat::CH::L}, 
      {AudioFormat::CH::R, AudioFormat::CH::R},
      {AudioFormat::CH::SL, AudioFormat::CH::SL}, 
      {AudioFormat::CH::SR, AudioFormat::CH::SR},
      {AudioFormat::CH::C, AudioFormat::CH::C}, 
      {AudioFormat::CH::SW, AudioFormat::CH::SL}, 
      {AudioFormat::CH::SW, AudioFormat::CH::SR},
      {AudioFormat::CH::SW, AudioFormat::CH::C},
      {AudioFormat::CH::SW, AudioFormat::CH::L}, 
      {AudioFormat::CH::SW, AudioFormat::CH::R},
      {AudioFormat::CH::SBL, AudioFormat::CH::SL}, 
      {AudioFormat::CH::SBL, AudioFormat::CH::L}, 
      {AudioFormat::CH::SBR, AudioFormat::CH::SR},
      {AudioFormat::CH::SBR, AudioFormat::CH::R},}),

    CONVERT_CH_TABLE(AudioFormat::CHANNEL::CHANNEL_5_1_2CH, AudioFormat::CHANNEL::CHANNEL_MONO, std::vector<ChannelMapList>{ 
      {AudioFormat::CH::MONO, AudioFormat::CH::SBL}, 
      {AudioFormat::CH::MONO, AudioFormat::CH::SBR},
      {AudioFormat::CH::MONO, AudioFormat::CH::SL}, 
      {AudioFormat::CH::MONO, AudioFormat::CH::SR},
      {AudioFormat::CH::MONO, AudioFormat::CH::C},
      {AudioFormat::CH::MONO, AudioFormat::CH::SW},
      {AudioFormat::CH::MONO, AudioFormat::CH::L}, 
      {AudioFormat::CH::MONO, AudioFormat::CH::R} }),
    CONVERT_CH_TABLE(AudioFormat::CHANNEL::CHANNEL_5_1_2CH, AudioFormat::CHANNEL::CHANNEL_STEREO, std::vector<ChannelMapList>{ 
      {AudioFormat::CH::L, AudioFormat::CH::SBL}, 
      {AudioFormat::CH::R, AudioFormat::CH::SBR},
      {AudioFormat::CH::L, AudioFormat::CH::SL}, 
      {AudioFormat::CH::R, AudioFormat::CH::SR},
      {AudioFormat::CH::L, AudioFormat::CH::C},
      {AudioFormat::CH::R, AudioFormat::CH::C},
      {AudioFormat::CH::L, AudioFormat::CH::SW},
      {AudioFormat::CH::R, AudioFormat::CH::SW},
      {AudioFormat::CH::L, AudioFormat::CH::L}, 
      {AudioFormat::CH::R, AudioFormat::CH::R} }),
    CONVERT_CH_TABLE(AudioFormat::CHANNEL::CHANNEL_5_1_2CH, AudioFormat::CHANNEL::CHANNEL_4CH, std::vector<ChannelMapList>{ 
      {AudioFormat::CH::SL, AudioFormat::CH::SBL}, 
      {AudioFormat::CH::SR, AudioFormat::CH::SBR},
      {AudioFormat::CH::SL, AudioFormat::CH::SL}, 
      {AudioFormat::CH::SR, AudioFormat::CH::SR},
      {AudioFormat::CH::L, AudioFormat::CH::C},
      {AudioFormat::CH::R, AudioFormat::CH::C},
      {AudioFormat::CH::L, AudioFormat::CH::SW},
      {AudioFormat::CH::R, AudioFormat::CH::SW},
      {AudioFormat::CH::L, AudioFormat::CH::L}, 
      {AudioFormat::CH::R, AudioFormat::CH::R} }),
    CONVERT_CH_TABLE(AudioFormat::CHANNEL::CHANNEL_5_1_2CH, AudioFormat::CHANNEL::CHANNEL_5CH, std::vector<ChannelMapList>{ 
      {AudioFormat::CH::SL, AudioFormat::CH::SBL}, 
      {AudioFormat::CH::SR, AudioFormat::CH::SBR},
      {AudioFormat::CH::SL, AudioFormat::CH::SL}, 
      {AudioFormat::CH::SR, AudioFormat::CH::SR},
      {AudioFormat::CH::C, AudioFormat::CH::C},
      {AudioFormat::CH::L, AudioFormat::CH::L}, 
      {AudioFormat::CH::R, AudioFormat::CH::R} }), // SW ignored
    CONVERT_CH_TABLE(AudioFormat::CHANNEL::CHANNEL_5_1_2CH, AudioFormat::CHANNEL::CHANNEL_5_1CH, std::vector<ChannelMapList>{ 
      {AudioFormat::CH::SL, AudioFormat::CH::SBL}, 
      {AudioFormat::CH::SR, AudioFormat::CH::SBR},
      {AudioFormat::CH::SL, AudioFormat::CH::SL}, 
      {AudioFormat::CH::SR, AudioFormat::CH::SR},
      {AudioFormat::CH::C, AudioFormat::CH::C},
      {AudioFormat::CH::SW, AudioFormat::CH::SW},
      {AudioFormat::CH::L, AudioFormat::CH::L}, 
      {AudioFormat::CH::R, AudioFormat::CH::R} }),
    CONVERT_CH_TABLE(AudioFormat::CHANNEL::CHANNEL_5_1_2CH, AudioFormat::CHANNEL::CHANNEL_5_0_2CH, std::vector<ChannelMapList>{ 
      {AudioFormat::CH::SBL, AudioFormat::CH::SBL}, 
      {AudioFormat::CH::SBR, AudioFormat::CH::SBR},
      {AudioFormat::CH::SL, AudioFormat::CH::SL}, 
      {AudioFormat::CH::SR, AudioFormat::CH::SR},
      {AudioFormat::CH::C, AudioFormat::CH::C},
      {AudioFormat::CH::L, AudioFormat::CH::L}, 
      {AudioFormat::CH::R, AudioFormat::CH::R} }),
    CONVERT_CH_TABLE(AudioFormat::CHANNEL::CHANNEL_5_1_2CH, AudioFormat::CHANNEL::CHANNEL_7_1CH, std::vector<ChannelMapList>{ 
      {AudioFormat::CH::L, AudioFormat::CH::L}, 
      {AudioFormat::CH::R, AudioFormat::CH::R},
      {AudioFormat::CH::SL, AudioFormat::CH::SL}, 
      {AudioFormat::CH::SR, AudioFormat::CH::SR},
      {AudioFormat::CH::C, AudioFormat::CH::C}, 
      {AudioFormat::CH::SW, AudioFormat::CH::SW}, 
      {AudioFormat::CH::SBL, AudioFormat::CH::SBL}, 
      {AudioFormat::CH::SBR, AudioFormat::CH::SBR},}),

    CONVERT_CH_TABLE(AudioFormat::CHANNEL::CHANNEL_7_1CH, AudioFormat::CHANNEL::CHANNEL_MONO, std::vector<ChannelMapList>{ 
      {AudioFormat::CH::MONO, AudioFormat::CH::SBL}, 
      {AudioFormat::CH::MONO, AudioFormat::CH::SBR},
      {AudioFormat::CH::MONO, AudioFormat::CH::SL}, 
      {AudioFormat::CH::MONO, AudioFormat::CH::SR},
      {AudioFormat::CH::MONO, AudioFormat::CH::C},
      {AudioFormat::CH::MONO, AudioFormat::CH::SW},
      {AudioFormat::CH::MONO, AudioFormat::CH::L}, 
      {AudioFormat::CH::MONO, AudioFormat::CH::R} }),
    CONVERT_CH_TABLE(AudioFormat::CHANNEL::CHANNEL_7_1CH, AudioFormat::CHANNEL::CHANNEL_STEREO, std::vector<ChannelMapList>{ 
      {AudioFormat::CH::L, AudioFormat::CH::SBL}, 
      {AudioFormat::CH::R, AudioFormat::CH::SBR},
      {AudioFormat::CH::L, AudioFormat::CH::SL}, 
      {AudioFormat::CH::R, AudioFormat::CH::SR},
      {AudioFormat::CH::L, AudioFormat::CH::C},
      {AudioFormat::CH::R, AudioFormat::CH::C},
      {AudioFormat::CH::L, AudioFormat::CH::SW},
      {AudioFormat::CH::R, AudioFormat::CH::SW},
      {AudioFormat::CH::L, AudioFormat::CH::L}, 
      {AudioFormat::CH::R, AudioFormat::CH::R} }),
    CONVERT_CH_TABLE(AudioFormat::CHANNEL::CHANNEL_7_1CH, AudioFormat::CHANNEL::CHANNEL_4CH, std::vector<ChannelMapList>{ 
      {AudioFormat::CH::SL, AudioFormat::CH::SBL}, 
      {AudioFormat::CH::SR, AudioFormat::CH::SBR},
      {AudioFormat::CH::SL, AudioFormat::CH::SL}, 
      {AudioFormat::CH::SR, AudioFormat::CH::SR},
      {AudioFormat::CH::L, AudioFormat::CH::C},
      {AudioFormat::CH::R, AudioFormat::CH::C},
      {AudioFormat::CH::L, AudioFormat::CH::SW},
      {AudioFormat::CH::R, AudioFormat::CH::SW},
      {AudioFormat::CH::L, AudioFormat::CH::L}, 
      {AudioFormat::CH::R, AudioFormat::CH::R} }),
    CONVERT_CH_TABLE(AudioFormat::CHANNEL::CHANNEL_7_1CH, AudioFormat::CHANNEL::CHANNEL_5CH, std::vector<ChannelMapList>{ 
      {AudioFormat::CH::SL, AudioFormat::CH::SBL}, 
      {AudioFormat::CH::SR, AudioFormat::CH::SBR},
      {AudioFormat::CH::SL, AudioFormat::CH::SL}, 
      {AudioFormat::CH::SR, AudioFormat::CH::SR},
      {AudioFormat::CH::C, AudioFormat::CH::C},
      {AudioFormat::CH::L, AudioFormat::CH::L}, 
      {AudioFormat::CH::R, AudioFormat::CH::R} }), // SW ignored
    CONVERT_CH_TABLE(AudioFormat::CHANNEL::CHANNEL_7_1CH, AudioFormat::CHANNEL::CHANNEL_5_1CH, std::vector<ChannelMapList>{ 
      {AudioFormat::CH::SL, AudioFormat::CH::SBL}, 
      {AudioFormat::CH::SR, AudioFormat::CH::SBR},
      {AudioFormat::CH::SL, AudioFormat::CH::SL}, 
      {AudioFormat::CH::SR, AudioFormat::CH::SR},
      {AudioFormat::CH::C, AudioFormat::CH::C},
      {AudioFormat::CH::SW, AudioFormat::CH::SW},
      {AudioFormat::CH::L, AudioFormat::CH::L}, 
      {AudioFormat::CH::R, AudioFormat::CH::R} }),
    CONVERT_CH_TABLE(AudioFormat::CHANNEL::CHANNEL_7_1CH, AudioFormat::CHANNEL::CHANNEL_5_0_2CH, std::vector<ChannelMapList>{ 
      {AudioFormat::CH::SBL, AudioFormat::CH::SBL}, 
      {AudioFormat::CH::SBR, AudioFormat::CH::SBR},
      {AudioFormat::CH::SL, AudioFormat::CH::SL}, 
      {AudioFormat::CH::SR, AudioFormat::CH::SR},
      {AudioFormat::CH::C, AudioFormat::CH::C},
      {AudioFormat::CH::L, AudioFormat::CH::L}, 
      {AudioFormat::CH::R, AudioFormat::CH::R} }),
    CONVERT_CH_TABLE(AudioFormat::CHANNEL::CHANNEL_7_1CH, AudioFormat::CHANNEL::CHANNEL_7_1CH, std::vector<ChannelMapList>{ 
      {AudioFormat::CH::L, AudioFormat::CH::L}, 
      {AudioFormat::CH::R, AudioFormat::CH::R},
      {AudioFormat::CH::SL, AudioFormat::CH::SL}, 
      {AudioFormat::CH::SR, AudioFormat::CH::SR},
      {AudioFormat::CH::C, AudioFormat::CH::C}, 
      {AudioFormat::CH::SW, AudioFormat::CH::SW}, 
      {AudioFormat::CH::SBL, AudioFormat::CH::SBL}, 
      {AudioFormat::CH::SBR, AudioFormat::CH::SBR},}),

    CONVERT_CH_TABLE(AudioFormat::CHANNEL::CHANNEL_UNKNOWN, AudioFormat::CHANNEL::CHANNEL_UNKNOWN, std::vector<ChannelMapList>{ {AudioFormat::CH::L, AudioFormat::CH::L}, {AudioFormat::CH::R, AudioFormat::CH::R} })
  };

  CONVERT_CH_TABLE* pSelectedChMapper = nullptr;
  for(int i=0; ( (pSelectedChMapper[i].srcChannel != AudioFormat::CHANNEL::CHANNEL_UNKNOWN) && (pSelectedChMapper[i].dstChannel != AudioFormat::CHANNEL::CHANNEL_UNKNOWN) ) ; i++ ) {
    if( (pSelectedChMapper[i].srcChannel != AudioFormat::CHANNEL::CHANNEL_UNKNOWN) && (pSelectedChMapper[i].dstChannel != AudioFormat::CHANNEL::CHANNEL_UNKNOWN) ){
      pSelectedChMapper = &pSelectedChMapper[i];
      break;
    }
  }

  if( pSelectedChMapper ){
    // TODO: Check case of mix e.g. L->C, R->C then L+R -> C
    // The following getSelectedChannelData expects no mix case.
    AudioFormat::ChannelMapper mapper;
    for(auto& chMap : pSelectedChMapper->chMapList){
      mapper.insert( std::make_pair(chMap.dstCh, chMap.srcCh) );
    }
    AudioBuffer result = srcBuf.getSelectedChannelData(dstFormat, mapper);
    dstBuf = result;
  } else {
    dstBuf = srcBuf;
  }

  return dstBuf.getAudioFormat().getChannels() == dstChannel;
}
