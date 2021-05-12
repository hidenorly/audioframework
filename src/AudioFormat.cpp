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

#include "AudioFormat.hpp"

AudioFormat::AudioFormat(AudioFormat::ENCODING encoding, int samplingRate, AudioFormat::CHANNEL channel):mEncoding(encoding),mSamplingRate(samplingRate),mChannel(channel)
{

}

AudioFormat::~AudioFormat()
{

}

int AudioFormat::getSampleByte(AudioFormat::ENCODING encoding)
{
  int sampleByte = 0;

  switch( encoding ){
    case PCM_8BIT:
      sampleByte = 1;
      break;
    case PCM_16BIT:
    case PCM_UNKNOWN:
      sampleByte = 2;
      break;
    case PCM_24BIT_PACKED:
      sampleByte = 3;
      break;
    case PCM_32BIT:
    case PCM_FLOAT:
      sampleByte = 4;
      break;
  }

  return sampleByte;
}

int AudioFormat::getNumberOfChannels(AudioFormat::CHANNEL channel)
{
  int numOfChannels = 0;

  switch( channel ){
    case AudioFormat::CHANNEL::CHANNEL_MONO:
      numOfChannels = 1;
      break;
    case AudioFormat::CHANNEL::CHANNEL_STEREO:
    case AudioFormat::CHANNEL::CHANNEL_UNKNOWN:
      numOfChannels = 2;
      break;
    case AudioFormat::CHANNEL::CHANNEL_4CH:
      numOfChannels = 4;
      break;
    case AudioFormat::CHANNEL::CHANNEL_5CH:
      numOfChannels = 5;
      break;
    case AudioFormat::CHANNEL::CHANNEL_5_1CH:
      numOfChannels = 6;
      break;
    case AudioFormat::CHANNEL::CHANNEL_5_1_2CH:
      numOfChannels = 8;
      break;
    case AudioFormat::CHANNEL::CHANNEL_5_0_2CH:
      numOfChannels = 7;
      break;
    case AudioFormat::CHANNEL::CHANNEL_7_1CH:
      numOfChannels = 8;
      break;
  }

  return numOfChannels;
}

int AudioFormat::getChannelsSampleByte(AudioFormat::ENCODING encoding, AudioFormat::CHANNEL channel)
{
  return getSampleByte(encoding) * getNumberOfChannels(channel);
}

AudioFormat::ENCODING AudioFormat::getEncoding(void)
{
  return mEncoding;
}

int AudioFormat::getNumberOfChannels(void)
{
  return getNumberOfChannels(mChannel);
}


std::string AudioFormat::getEncodingString(AudioFormat::ENCODING encoding)
{
  std::string result = "PCM_UNKNOWN";

  switch( encoding ){
    case PCM_8BIT:
      result = "PCM_8BIT";
      break;
    case PCM_16BIT:
      result = "PCM_16BIT";
      break;
    case PCM_24BIT_PACKED:
      result = "PCM_24BIT";
      break;
    case PCM_32BIT:
      result = "PCM_32BIT";
      break;
    case PCM_FLOAT:
      result = "PCM_FLOAT";
      break;
    case PCM_UNKNOWN:
      result = "PCM_UNKNOWN";
      break;
  }

  return result;
}

std::string AudioFormat::getEncodingString(void)
{
  return getEncodingString( mEncoding );
}

AudioFormat::CHANNEL AudioFormat::getChannels(void)
{
  return mChannel;
}

int AudioFormat::getSampleByte(void)
{
  return getSampleByte(mEncoding);
}

int AudioFormat::getChannelsSampleByte(void)
{
  return getSampleByte() * getNumberOfChannels(mChannel);
}

int AudioFormat::getSamplingRate(void)
{
  return mSamplingRate;
}

bool AudioFormat::equal(AudioFormat arg2){
  return ( (mEncoding == arg2.getEncoding()) && (mChannel == arg2.getChannels()) && (mSamplingRate == arg2.getSamplingRate()));
}

int AudioFormat::getOffSetInSample(AudioFormat::AudioFormat::CHANNEL channel, AudioFormat::CH ch)
{
  struct formatOffset
  {
  public:
    AudioFormat::AudioFormat::CHANNEL channel;
    AudioFormat::CH ch;
    int offSet;
    formatOffset(AudioFormat::AudioFormat::CHANNEL channel, AudioFormat::CH ch, int offSet):channel(channel),ch(ch),offSet(offSet){};
  };

  static formatOffset formatOffsetTable[]=
  {
    formatOffset(AudioFormat::CHANNEL::CHANNEL_MONO, AudioFormat::CH::MONO, 0),

    formatOffset(AudioFormat::CHANNEL::CHANNEL_STEREO, AudioFormat::CH::L, 0),
    formatOffset(AudioFormat::CHANNEL::CHANNEL_STEREO, AudioFormat::CH::R, 1),

    formatOffset(AudioFormat::CHANNEL::CHANNEL_4CH, AudioFormat::CH::FL, 0),
    formatOffset(AudioFormat::CHANNEL::CHANNEL_4CH, AudioFormat::CH::FR, 1),
    formatOffset(AudioFormat::CHANNEL::CHANNEL_4CH, AudioFormat::CH::SL, 2),
    formatOffset(AudioFormat::CHANNEL::CHANNEL_4CH, AudioFormat::CH::SR, 3),

    formatOffset(AudioFormat::CHANNEL::CHANNEL_5CH, AudioFormat::CH::FL, 0),
    formatOffset(AudioFormat::CHANNEL::CHANNEL_5CH, AudioFormat::CH::C,  1),
    formatOffset(AudioFormat::CHANNEL::CHANNEL_5CH, AudioFormat::CH::FR, 2),
    formatOffset(AudioFormat::CHANNEL::CHANNEL_5CH, AudioFormat::CH::SL, 3),
    formatOffset(AudioFormat::CHANNEL::CHANNEL_5CH, AudioFormat::CH::SR, 4),

    formatOffset(AudioFormat::CHANNEL::CHANNEL_5_1CH, AudioFormat::CH::FL, 0),
    formatOffset(AudioFormat::CHANNEL::CHANNEL_5_1CH, AudioFormat::CH::C,  1),
    formatOffset(AudioFormat::CHANNEL::CHANNEL_5_1CH, AudioFormat::CH::FR, 2),
    formatOffset(AudioFormat::CHANNEL::CHANNEL_5_1CH, AudioFormat::CH::SL, 3),
    formatOffset(AudioFormat::CHANNEL::CHANNEL_5_1CH, AudioFormat::CH::SR, 4),
    formatOffset(AudioFormat::CHANNEL::CHANNEL_5_1CH, AudioFormat::CH::SW, 5),

    formatOffset(AudioFormat::CHANNEL::CHANNEL_5_0_2CH, AudioFormat::CH::FL, 0),
    formatOffset(AudioFormat::CHANNEL::CHANNEL_5_0_2CH, AudioFormat::CH::C,  1),
    formatOffset(AudioFormat::CHANNEL::CHANNEL_5_0_2CH, AudioFormat::CH::FR, 2),
    formatOffset(AudioFormat::CHANNEL::CHANNEL_5_0_2CH, AudioFormat::CH::SL, 3),
    formatOffset(AudioFormat::CHANNEL::CHANNEL_5_0_2CH, AudioFormat::CH::SR, 4),
    formatOffset(AudioFormat::CHANNEL::CHANNEL_5_0_2CH, AudioFormat::CH::SBL, 5),
    formatOffset(AudioFormat::CHANNEL::CHANNEL_5_0_2CH, AudioFormat::CH::SBR, 6),

    formatOffset(AudioFormat::CHANNEL::CHANNEL_5_1_2CH, AudioFormat::CH::FL, 0),
    formatOffset(AudioFormat::CHANNEL::CHANNEL_5_1_2CH, AudioFormat::CH::C,  1),
    formatOffset(AudioFormat::CHANNEL::CHANNEL_5_1_2CH, AudioFormat::CH::FR, 2),
    formatOffset(AudioFormat::CHANNEL::CHANNEL_5_1_2CH, AudioFormat::CH::SL, 3),
    formatOffset(AudioFormat::CHANNEL::CHANNEL_5_1_2CH, AudioFormat::CH::SR, 4),
    formatOffset(AudioFormat::CHANNEL::CHANNEL_5_1_2CH, AudioFormat::CH::SBL, 5),
    formatOffset(AudioFormat::CHANNEL::CHANNEL_5_1_2CH, AudioFormat::CH::SBR, 6),
    formatOffset(AudioFormat::CHANNEL::CHANNEL_5_1_2CH, AudioFormat::CH::SW, 7),

    formatOffset(AudioFormat::CHANNEL::CHANNEL_7_1CH, AudioFormat::CH::FL, 0),
    formatOffset(AudioFormat::CHANNEL::CHANNEL_7_1CH, AudioFormat::CH::C,  1),
    formatOffset(AudioFormat::CHANNEL::CHANNEL_7_1CH, AudioFormat::CH::FR, 2),
    formatOffset(AudioFormat::CHANNEL::CHANNEL_7_1CH, AudioFormat::CH::SL, 3),
    formatOffset(AudioFormat::CHANNEL::CHANNEL_7_1CH, AudioFormat::CH::SR, 4),
    formatOffset(AudioFormat::CHANNEL::CHANNEL_7_1CH, AudioFormat::CH::SBL, 5),
    formatOffset(AudioFormat::CHANNEL::CHANNEL_7_1CH, AudioFormat::CH::SBR, 6),
    formatOffset(AudioFormat::CHANNEL::CHANNEL_7_1CH, AudioFormat::CH::SW, 7),

    formatOffset(AudioFormat::AudioFormat::CHANNEL::CHANNEL_UNKNOWN, AudioFormat::CH::MONO, 0)
  };

  for(int i=0; (formatOffsetTable[i].channel!=AudioFormat::AudioFormat::CHANNEL::CHANNEL_UNKNOWN); i++){
    if( ( formatOffsetTable[i].channel == channel ) && ( formatOffsetTable[i].ch == ch ) ){
      return formatOffsetTable[i].offSet;
    }
  }

  return 0;
}

int AudioFormat::getOffSetByteInSample(AudioFormat::CH ch)
{
  return getSampleByte() * getOffSetInSample(mChannel, ch);
}



std::vector<AudioFormat> AudioBase::getSupportedAudioFormats(void)
{
  std::vector<AudioFormat> audioFormats;
  audioFormats.push_back( AudioFormat() );
  return audioFormats;
}

bool AudioBase::isAvailableFormat(AudioFormat format)
{
  bool bResult = false;

  std::vector<AudioFormat> audioFormats = getSupportedAudioFormats();
  for(auto& aFormat : audioFormats){
    bResult |= format.equal(aFormat);
    if( bResult ) break;
  }

  return bResult;
}