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

#ifndef __AUDIO_FORMAT_HPP__
#define __AUDIO_FORMAT_HPP__

#include <string>

class AudioFormat
{
public:
  enum ENCODING {
    PCM_8BIT,
    PCM_16BIT,
    PCM_24BIT_PACKED,
    PCM_32BIT,
    PCM_FLOAT,
    PCM_UNKNOWN,
    ENCODING_DEFAULT = PCM_16BIT
  };

  static int getSampleByte(ENCODING encoding)
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


  enum CHANNEL {
    CHANNEL_MONO,
    CHANNEL_STEREO,
    CHANNEL_4CH,
    CHANNEL_5CH,
    CHANNEL_5_1CH,
    CHANNEL_5_1_2CH,
    CHANNEL_5_0_2CH,
    CHANNEL_7_1CH,
    CHANNEL_UNKNOWN,
    CHANNEL_DEFAULT = CHANNEL_STEREO
  };

  static int getNumberOfChannels(CHANNEL channel)
  {
    int numOfChannels = 0;

    switch( channel ){
      case CHANNEL_MONO:
        numOfChannels = 1;
        break;
      case CHANNEL_STEREO:
      case CHANNEL_UNKNOWN:
        numOfChannels = 2;
        break;
      case CHANNEL_4CH:
        numOfChannels = 4;
        break;
      case CHANNEL_5CH:
        numOfChannels = 5;
        break;
      case CHANNEL_5_1CH:
        numOfChannels = 6;
        break;
      case CHANNEL_5_1_2CH:
        numOfChannels = 8;
        break;
      case CHANNEL_5_0_2CH:
        numOfChannels = 7;
        break;
      case CHANNEL_7_1CH:
        numOfChannels = 8;
        break;
    }

    return numOfChannels;
  }

  static int getChannelsSampleByte(ENCODING encoding, CHANNEL channel)
  {
    return getSampleByte(encoding) * getNumberOfChannels(channel);
  }

  enum SAMPLING_RATE {
    SAMPLING_RATE_8_KHZ = 8000,
    SAMPLING_RATE_16_KHZ = 16000,
    SAMPLING_RATE_44_1_KHZ = 44100,
    SAMPLING_RATE_48_KHZ = 48000,
    SAMPLING_RATE_88_2_KHZ = 88200,
    SAMPLING_RATE_96_KHZ = 96000,
    SAMPLING_RATE_192_KHZ = 192000,
    SAMPLING_RATE_DEFAULT = SAMPLING_RATE_48_KHZ,
    SAMPLING_RATE_UNKNOWN,
  };

protected:
  ENCODING mEncoding;
  SAMPLING_RATE mSamplingRate;
  CHANNEL mChannel;

public:
  AudioFormat(ENCODING encoding = ENCODING_DEFAULT, SAMPLING_RATE samplingRate = SAMPLING_RATE_DEFAULT, CHANNEL channel = CHANNEL_DEFAULT):mEncoding(encoding),mSamplingRate(samplingRate),mChannel(channel){};
  virtual ~AudioFormat(){};

  ENCODING getEncoding(void)
  {
    return mEncoding;
  }

  int getNumberOfChannels(void)
  {
    return getNumberOfChannels(mChannel);
  }


  static std::string getEncodingString(ENCODING encoding)
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

  std::string getEncodingString(void){
    return getEncodingString( mEncoding );
  }


  CHANNEL getChannels()
  {
    return mChannel;
  }

  int getChannelsSampleByte(void)
  {
    return getSampleByte(mEncoding) * getNumberOfChannels(mChannel);
  }

  int getSamplingRate(void){
    return mSamplingRate;
  }

  bool equal(AudioFormat& arg2){
    return ( (mEncoding == arg2.getEncoding()) && (mChannel == arg2.getChannels()) && (mSamplingRate == arg2.getSamplingRate()));
  }

};

class AudioBase
{
public:
  virtual std::vector<AudioFormat> getSupportedAudioFormats(void){
    std::vector<AudioFormat> audioFormats;
    audioFormats.push_back( AudioFormat() );
    return audioFormats;
  };

  bool isAvailableFormat(AudioFormat format){
    bool bResult = false;

    std::vector<AudioFormat> audioFormats = getSupportedAudioFormats();
    for(AudioFormat aFormat : audioFormats){
      bResult |= format.equal(aFormat);
      if( bResult ) break;
    }

    return bResult;
  }
};

#endif /* __AUDIO_FORMAT_HPP__ */
