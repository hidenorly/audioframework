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
#include <algorithm>

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
    case PCM_24BIT_PACKED:
      sampleByte = 3;
      break;
    case PCM_32BIT:
    case PCM_FLOAT:
      sampleByte = 4;
      break;
    case PCM_16BIT:
    case PCM_UNKNOWN:
    default:
      sampleByte = 2;
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
    case AudioFormat::CHANNEL::CHANNEL_2_1CH:
      numOfChannels = 3;
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


AudioFormat::ChannelMapper AudioFormat::getSameChannelMapper(void)
{
  return getSameChannelMapper(getChannels());
}

AudioFormat::ChannelMapper AudioFormat::getSameChannelMapper(AudioFormat::CHANNEL channel)
{
  AudioFormat::ChannelMapper result;

  switch( channel ){
    case AudioFormat::CHANNEL::CHANNEL_MONO:
      result.insert_or_assign( AudioFormat::CH::MONO, AudioFormat::CH::MONO );
      break;
    case AudioFormat::CHANNEL::CHANNEL_STEREO:
      result.insert_or_assign( AudioFormat::CH::L, AudioFormat::CH::L );
      result.insert_or_assign( AudioFormat::CH::R, AudioFormat::CH::R );
      break;
    case AudioFormat::CHANNEL::CHANNEL_4CH:
      result.insert_or_assign( AudioFormat::CH::FL, AudioFormat::CH::FL );
      result.insert_or_assign( AudioFormat::CH::FR, AudioFormat::CH::FR );
      result.insert_or_assign( AudioFormat::CH::SL, AudioFormat::CH::SL );
      result.insert_or_assign( AudioFormat::CH::SR, AudioFormat::CH::SR );
      break;
    case AudioFormat::CHANNEL::CHANNEL_5CH:
      result.insert_or_assign( AudioFormat::CH::FL, AudioFormat::CH::FL );
      result.insert_or_assign( AudioFormat::CH::FR, AudioFormat::CH::FR );
      result.insert_or_assign( AudioFormat::CH::SL, AudioFormat::CH::SL );
      result.insert_or_assign( AudioFormat::CH::SR, AudioFormat::CH::SR );
      result.insert_or_assign( AudioFormat::CH::C, AudioFormat::CH::C );
      break;
    case AudioFormat::CHANNEL::CHANNEL_5_1CH:
      result.insert_or_assign( AudioFormat::CH::FL, AudioFormat::CH::FL );
      result.insert_or_assign( AudioFormat::CH::FR, AudioFormat::CH::FR );
      result.insert_or_assign( AudioFormat::CH::SL, AudioFormat::CH::SL );
      result.insert_or_assign( AudioFormat::CH::SR, AudioFormat::CH::SR );
      result.insert_or_assign( AudioFormat::CH::C, AudioFormat::CH::C );
      result.insert_or_assign( AudioFormat::CH::SW, AudioFormat::CH::SW );
      break;
    case AudioFormat::CHANNEL::CHANNEL_5_1_2CH:
    case AudioFormat::CHANNEL::CHANNEL_7_1CH:
      result.insert_or_assign( AudioFormat::CH::FL, AudioFormat::CH::FL );
      result.insert_or_assign( AudioFormat::CH::FR, AudioFormat::CH::FR );
      result.insert_or_assign( AudioFormat::CH::SL, AudioFormat::CH::SL );
      result.insert_or_assign( AudioFormat::CH::SR, AudioFormat::CH::SR );
      result.insert_or_assign( AudioFormat::CH::C, AudioFormat::CH::C );
      result.insert_or_assign( AudioFormat::CH::SW, AudioFormat::CH::SW );
      result.insert_or_assign( AudioFormat::CH::SBL, AudioFormat::CH::SBL );
      result.insert_or_assign( AudioFormat::CH::SBR, AudioFormat::CH::SBR );
      break;
    case AudioFormat::CHANNEL::CHANNEL_5_0_2CH:
      result.insert_or_assign( AudioFormat::CH::FL, AudioFormat::CH::FL );
      result.insert_or_assign( AudioFormat::CH::FR, AudioFormat::CH::FR );
      result.insert_or_assign( AudioFormat::CH::SL, AudioFormat::CH::SL );
      result.insert_or_assign( AudioFormat::CH::SR, AudioFormat::CH::SR );
      result.insert_or_assign( AudioFormat::CH::C, AudioFormat::CH::C );
      result.insert_or_assign( AudioFormat::CH::SBL, AudioFormat::CH::SBL );
      result.insert_or_assign( AudioFormat::CH::SBR, AudioFormat::CH::SBR );
      break;
    case AudioFormat::CHANNEL::CHANNEL_UNKNOWN:
    default:
      break;
  }

  return result;
}


int AudioFormat::getChannelsSampleByte(AudioFormat::ENCODING encoding, AudioFormat::CHANNEL channel)
{
  return getSampleByte(encoding) * getNumberOfChannels(channel);
}

AudioFormat::ENCODING AudioFormat::getEncoding(void)
{
  return mEncoding;
}

bool AudioFormat::isEncodingPcm(void)
{
  return isEncodingPcm( getEncoding() );
}

bool AudioFormat::isEncodingPcm(AudioFormat::ENCODING encoding)
{
  return encoding < ENCODING::PCM_UNKNOWN;
}

bool AudioFormat::isEncodingCompressed(void)
{
  return !isEncodingPcm();
}

bool AudioFormat::isEncodingCompressed(AudioFormat::ENCODING encoding)
{
  return !isEncodingPcm( encoding );
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
    default:
      result = "PCM_UNKNOWN";
      break;
  }
  if( isEncodingCompressed(encoding) ){
    result = "COMPRESSED_" + std::to_string(encoding - AudioFormat::ENCODING::COMPRESSED);
  }

  return result;
}

AudioFormat::ENCODING AudioFormat::getEncodingFromString(std::string encoding)
{
  if( encoding == "PCM_16BIT" ) return AudioFormat::ENCODING::PCM_16BIT;
  if( encoding == "PCM_FLOAT" ) return AudioFormat::ENCODING::PCM_FLOAT;
  if( encoding == "PCM_32BIT" ) return AudioFormat::ENCODING::PCM_32BIT;
  if( encoding == "PCM_8BIT" ) return AudioFormat::ENCODING::PCM_8BIT;
  if( encoding == "PCM_24BIT" ) return AudioFormat::ENCODING::PCM_24BIT_PACKED;
  if( encoding.starts_with("COMPRESSED_") ){
    int nCompressedEncoding = std::stoi( encoding.substr(11, encoding.size()) );
    return (AudioFormat::ENCODING)(AudioFormat::ENCODING::COMPRESSED + nCompressedEncoding);
  }
  return AudioFormat::ENCODING::PCM_UNKNOWN;
}

std::string AudioFormat::getEncodingString(void)
{
  return getEncodingString( mEncoding );
}

AudioFormat::CHANNEL AudioFormat::getChannels(void)
{
  return mChannel;
}

AudioFormat::CHANNEL AudioFormat::getChannelsFromString(std::string channels)
{
  if( channels == "1" || channels == "mono" ) return AudioFormat::CHANNEL::CHANNEL_MONO;
  if( channels == "2" || channels == "streo" ) return AudioFormat::CHANNEL::CHANNEL_STEREO;
  if( channels == "2.1" ) return AudioFormat::CHANNEL::CHANNEL_2_1CH;
  if( channels == "4" ) return AudioFormat::CHANNEL::CHANNEL_4CH;
  if( channels == "5" ) return AudioFormat::CHANNEL::CHANNEL_5CH;
  if( channels == "5.1" ) return AudioFormat::CHANNEL::CHANNEL_5_1CH;
  if( channels == "5.0.2" ) return AudioFormat::CHANNEL::CHANNEL_5_0_2CH;
  if( channels == "5.1.2" ) return AudioFormat::CHANNEL::CHANNEL_5_1_2CH;
  if( channels == "7.1" ) return AudioFormat::CHANNEL::CHANNEL_7_1CH;

  return AudioFormat::CHANNEL::CHANNEL_UNKNOWN;
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

int AudioFormat::getOffSetInSample(AudioFormat::CH ch)
{
  return getOffSetInSample(mChannel, ch);
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

int AudioFormat::getOffSetByteInSample(AudioFormat audioFormat, AudioFormat::CH ch)
{
  return audioFormat.getSampleByte() * audioFormat.getOffSetInSample(ch);
}

int AudioFormat::getOffSetByteInSample(AudioFormat::CH ch)
{
  return getSampleByte() * getOffSetInSample(mChannel, ch);
}

std::string AudioFormat::toString(void)
{
  std::string result = "encoding:" + getEncodingString() + " channel:" + std::to_string(getNumberOfChannels()) + " samplingRate:" + std::to_string(getSamplingRate());
  return result;
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

std::vector<AudioFormat> AudioBase::audioFormatOpAND(std::vector<AudioFormat>& formats1, std::vector<AudioFormat>& formats2)
{
  std::vector<AudioFormat> result;
  for( auto& aValue1 : formats1 ){
    for( auto& aValue2 : formats2 ){
      if( aValue1.equal( aValue2 ) ){
        result.push_back( aValue1 );
        break;
      }
    }
  }
  return result;
}

std::vector<AudioFormat> AudioBase::audioFormatOpOR(std::vector<AudioFormat>& formats1, std::vector<AudioFormat>& formats2)
{
  std::vector<AudioFormat> result = formats1;
  for( auto& aValue2 : formats2 ){
    bool bFound = false;
    for( auto& aValue1 : formats1 ){
      bFound |= aValue2.equal ( aValue1 );
      if( bFound ) break;
    }
    if( !bFound ){
      result.push_back( aValue2 );
    }
  }
  return result;
}

std::shared_ptr<AudioFormat> AudioFormat::getCopiedNewSharedInstance(void)
{
  return std::make_shared<AudioFormat>( getEncoding(), getSamplingRate(), getChannels() );
}

AudioFormat::CHANNEL AudioFormat::getAudioChannel(int nChannels)
{
  switch( nChannels ){
    case 1:
      return AudioFormat::CHANNEL::CHANNEL_MONO;
    case 2:
      return AudioFormat::CHANNEL::CHANNEL_STEREO;
    case 3:
      return AudioFormat::CHANNEL::CHANNEL_2_1CH;
    case 4:
      return AudioFormat::CHANNEL::CHANNEL_4CH;
    case 5:
      return AudioFormat::CHANNEL::CHANNEL_5CH;
    case 6:
      return AudioFormat::CHANNEL::CHANNEL_5_1CH;
    case 7:
      return AudioFormat::CHANNEL::CHANNEL_5_0_2CH;
    case 8:
      return AudioFormat::CHANNEL::CHANNEL_7_1CH;
    default:
      return AudioFormat::CHANNEL::CHANNEL_UNKNOWN;
  }
}


AudioBase::AudioBase()
{

}

AudioBase::~AudioBase()
{
  mAudioFormatListerners.clear();
}

void AudioBase::registerAudioFormatListener(std::shared_ptr<AudioBase::AudioFormatListener> listener)
{
  std::weak_ptr<AudioBase::AudioFormatListener> theListener(listener);
  mAudioFormatListerners.push_back( theListener );
}

void AudioBase::unregisterAudioFormatListener(std::shared_ptr<AudioBase::AudioFormatListener> listener)
{
  std::weak_ptr<AudioBase::AudioFormatListener> theListener(listener);

  const auto pos = std::find_if(mAudioFormatListerners.begin(), mAudioFormatListerners.end(), [&theListener](const std::weak_ptr<AudioBase::AudioFormatListener>& aListener) {
          return aListener.lock() == theListener.lock();
      });

  if (pos != mAudioFormatListerners.end()){
    mAudioFormatListerners.erase(pos);
  }
}

void AudioBase::notifyAudioFormatChanged(AudioFormat format)
{
  for(auto& aListener : mAudioFormatListerners ){
    std::shared_ptr<AudioBase::AudioFormatListener> theListener = aListener.lock();
    if( theListener ){
      theListener->onFormatChanged( format );
    }
  }
}

bool AudioBase::setAudioFormat(AudioFormat format)
{
  bool result = isAvailableFormat(format);
  if( result && !format.equal( mPreviousAudioFormat) ){
    setAudioFormatPrimitive(format);
    mPreviousAudioFormat = format;
    notifyAudioFormatChanged( format );
  }
  return result;
}
