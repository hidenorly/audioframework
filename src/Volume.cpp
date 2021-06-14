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

#include "Volume.hpp"
#include "VolumePrimitive.hpp"
#include <vector>
#include <algorithm>

bool Volume::process( AudioBuffer* pInBuf, AudioBuffer* pOutBuf, std::vector<float> channelVolumes )
{
  bool bHandled = false;

  if( pInBuf && pOutBuf ){
    int nSamples = pInBuf->getNumberOfSamples();
    if( pOutBuf->getNumberOfSamples() != nSamples) {
      pOutBuf->resize( nSamples );
    }
    AudioFormat srcFormat = pInBuf->getAudioFormat();
    AudioFormat dstFormat = pOutBuf->getAudioFormat();
    if( srcFormat.equal(dstFormat) ){
      if( isVolumeRequired( channelVolumes ) ){
        int nChannelSamples = nSamples * dstFormat.getNumberOfChannels();

        int8_t* pRawInBuf = reinterpret_cast<int8_t*>( pInBuf->getRawBufferPointer() );
        int8_t* pRawOutBuf = reinterpret_cast<int8_t*>( pOutBuf->getRawBufferPointer() );

        switch( dstFormat.getEncoding() ){
          case AudioFormat::ENCODING::PCM_8BIT:
            bHandled = VolumePrimitive::volume( pRawInBuf, pRawOutBuf, channelVolumes, nChannelSamples );
            break;
          case AudioFormat::ENCODING::PCM_16BIT:
            bHandled = VolumePrimitive::volume( reinterpret_cast<int16_t*>(pRawInBuf), reinterpret_cast<int16_t*>(pRawOutBuf), channelVolumes, nChannelSamples );
            break;
          case AudioFormat::ENCODING::PCM_32BIT:
            bHandled = VolumePrimitive::volume( reinterpret_cast<int32_t*>(pRawInBuf), reinterpret_cast<int32_t*>(pRawOutBuf), channelVolumes, nChannelSamples );
            break;
          case AudioFormat::ENCODING::PCM_FLOAT:
            bHandled = VolumePrimitive::volume( reinterpret_cast<float*>(pRawInBuf), reinterpret_cast<float*>(pRawOutBuf), channelVolumes, nChannelSamples );
            break;
          case AudioFormat::ENCODING::PCM_24BIT_PACKED:
            bHandled = VolumePrimitive::volume24( pRawInBuf, pRawOutBuf, channelVolumes, nChannelSamples );
            break;
          case AudioFormat::ENCODING::PCM_UNKNOWN:
          default:
            bHandled = false;
            *pOutBuf = *pInBuf;
            break;
        }
      } else {
        *pOutBuf = *pInBuf;
        bHandled = true;
      }
    }
  }
  return bHandled;
}

bool Volume::process( AudioBuffer* pInBuf, AudioBuffer* pOutBuf, float volume )
{
  if( pOutBuf ){
    std::vector<float> channelVolumes;
    int nChannels = pOutBuf->getAudioFormat().getNumberOfChannels();
    for( int i=0; i<nChannels; i++ ){
      channelVolumes.push_back( volume );
    }
    return process( pInBuf, pOutBuf, channelVolumes );
  }
  return false;
}

bool Volume::process( AudioBuffer& inBuf, AudioBuffer& outBuf, float volume )
{
  return process( &inBuf, &outBuf, volume );
}

bool Volume::process( AudioBuffer* pInBuf, AudioBuffer* pOutBuf, Volume::CHANNEL_VOLUME volumes )
{
 if( pOutBuf ){
    return process( pInBuf, pOutBuf, getPerChannelVolumes(pOutBuf->getAudioFormat(), volumes) );
  }
  return false;
}

bool Volume::process( AudioBuffer& inBuf, AudioBuffer& outBuf, Volume::CHANNEL_VOLUME volumes )
{
  return process( &inBuf, &outBuf, volumes );
}

bool Volume::isVolumeRequired(std::vector<float> channelVolumes)
{
  bool result = true;

  for( auto& aVolume : channelVolumes ){
    result = (aVolume != 100.0f );
    if( result ) break;
  }

  return result;
}

bool Volume::isVolumeRequired(AudioFormat format, CHANNEL_VOLUME channelVolumes)
{
  return isVolumeRequired( getPerChannelVolumes(format, channelVolumes) );
}

std::vector<float> Volume::getPerChannelVolumes(AudioFormat format, CHANNEL_VOLUME channelVolumes)
{
  std::vector<float> channelVolumesVector;
  channelVolumesVector.resize( channelVolumes.size() );
  for( auto& [ch, volume] : channelVolumes ){
    channelVolumesVector[ format.getOffSetInSample(ch) ] = volume;
  }
  return channelVolumesVector;
}

float Volume::getVolumeMax(std::vector<float> volumes)
{
  float result = 0.0f;
  for( auto& aVolume : volumes ){
    result = std::max<float>( result, aVolume );
  }
  return result;
}