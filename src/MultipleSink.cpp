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

#include "MultipleSink.hpp"
#include <algorithm>
#include <iostream>


MultipleSink::MultipleSink(AudioFormat audioFormat, bool bSupportedFormatsOpOR):ISink(), mFormat(audioFormat), mMaxLatency(0), mbSupportedFormatsOpOR(bSupportedFormatsOpOR)
{

}

MultipleSink::~MultipleSink()
{
  clearSinks();
}


void MultipleSink::attachSink(std::shared_ptr<ISink> pSink, AudioFormat::ChannelMapper& map)
{
  if( pSink ){
    mpSinks.push_back( pSink );
    mChannelMaps.insert_or_assign( pSink, map );
  }
}

bool MultipleSink::detachSink(std::shared_ptr<ISink> pSink)
{
  bool result = false;

  if( mChannelMaps.contains( pSink ) ){
    std::erase( mpSinks, pSink );
    mChannelMaps.erase( pSink );
    result = true;
    pSink = nullptr;
  }

  return result;
}

void MultipleSink::clearSinks(void)
{
  mpSinks.clear();
  mChannelMaps.clear();
}

void MultipleSink::ensureDelayFilters(bool bForceRecreate)
{
  bool isDelayFiltersEmpty = mpDelayFilters.empty();
  if( bForceRecreate || isDelayFiltersEmpty ){
    if( !isDelayFiltersEmpty ){
      mpDelayFilters.clear();
    }
    mMaxLatency = getLatencyUSec();
    for(auto& pSink : mpSinks ){
      mpDelayFilters.insert_or_assign( pSink, std::make_shared<DelayFilter>( pSink->getAudioFormat(), mMaxLatency-pSink->getLatencyUSec() ) );
    }
  }
}


void MultipleSink::writePrimitive(IAudioBuffer& buf)
{
  AudioBuffer* pBuf = dynamic_cast<AudioBuffer*>(&buf);
  for(auto& pSink : mpSinks ){
    if( pBuf ){
      AudioFormat::ChannelMapper mapper = mChannelMaps[ pSink ];
      AudioFormat sinkFormat = pSink->getAudioFormat();
      if( sinkFormat.getNumberOfChannels() >= mapper.size() ){
        AudioBuffer selectedChannelData = pBuf->getSelectedChannelData( sinkFormat, mapper );
        ensureDelayFilters();
        AudioBuffer delayedOut( sinkFormat, pBuf->getNumberOfSamples() );
        std::shared_ptr<DelayFilter> pDelayFilter = mpDelayFilters[ pSink ];
        pDelayFilter->process( selectedChannelData, delayedOut );
        pSink->write( delayedOut );
      }
    } else {
      pSink->write( buf );
    }
  }
}

void MultipleSink::dump(void)
{
  std::cout << "MultipleSink::list of sinks (MaxLatency = " << getLatencyUSec() << ")" << std::endl;

  for(auto& pSink : mpSinks ){
    std::cout << "Sink:" << pSink->toString() << ":" << pSink << " latency: " << pSink->getLatencyUSec() << std::endl;
    AudioFormat::ChannelMapper mapper = mChannelMaps[ pSink ];
    for( const auto& [dstCh, srcCh] : mapper ){
      std::cout << "SrcCh:" << srcCh << " -> DstCh:" << dstCh << std::endl;
    }
    pSink->dump();
  }
}

void MultipleSink::setAudioFormatSupportOrModeEnabled(bool bSupportedFormatsOpOR)
{
  mbSupportedFormatsOpOR = bSupportedFormatsOpOR;
}

bool MultipleSink::getAudioFormatSupportOrModeEnabled(void)
{
  return mbSupportedFormatsOpOR;
}

bool MultipleSink::setAudioFormat(AudioFormat audioFormat)
{
  return setAudioFormat(audioFormat, false);
}

bool MultipleSink::setAudioFormat(AudioFormat audioFormat, bool bForce)
{
  bool bResult = isAvailableFormat( audioFormat ) | bForce;
  if( bResult ){
    mFormat = audioFormat;
  }
  return bResult;
}

AudioFormat MultipleSink::getAudioFormat(void)
{
  return mFormat;
}

std::vector<AudioFormat> MultipleSink::getSupportedAudioFormats(void)
{
  std::vector<AudioFormat> result;
  if( mpSinks.size()>=1 ){
    result = mpSinks[0]->getSupportedAudioFormats();
    for( int i=1; i<mpSinks.size(); i++ ){
      std::vector<AudioFormat> theFormats = mpSinks[i]->getSupportedAudioFormats();
      result = mbSupportedFormatsOpOR ? audioFormatOpOR( result, theFormats ) : audioFormatOpAND( result, theFormats );
    }
  }
  return result;
}


int MultipleSink::getLatencyUSec(void)
{
  int nLatencyUsec = 0;

  for(auto& pSink : mpSinks ){
    int theLatencyUsec = pSink->getLatencyUSec();
    nLatencyUsec = std::max( theLatencyUsec, nLatencyUsec );
  }

  return nLatencyUsec;
}

float MultipleSink::getVolume(void)
{
  float result = 0.0f;

  for(auto& pSink : mpSinks ){
    result = std::max<float>( result, pSink->getVolume() );
  }

  return result;
}


bool MultipleSink::setVolume(float volumePercentage)
{
  bool bResult = ISink::setVolume(volumePercentage);

  for(auto& pSink : mpSinks ){
    bResult &= pSink->setVolume(volumePercentage);
  }

  return bResult;
}

std::vector<float> MultipleSink::getPerSinkChannelVolumes(std::shared_ptr<ISink> pSink, Volume::CHANNEL_VOLUME perChannelVolumes)
{
  std::vector<float> result;
  if( pSink && mChannelMaps.contains(pSink) ){
    AudioFormat::ChannelMapper mapper = mChannelMaps[pSink];
    result.resize( mapper.size() );
    AudioFormat format = pSink->getAudioFormat();
    for( const auto& [dstCh, srcCh] : mapper ){
      if( perChannelVolumes.contains(srcCh) ){
        result[ format.getOffSetInSample(dstCh) ] = perChannelVolumes[ srcCh ];
      } else {
        throw std::invalid_argument( "Should align Channel Volume with Sink's mapper" );
      }
    }
  }
  return result;
}


bool MultipleSink::setVolume(Volume::CHANNEL_VOLUME perChannelVolumes)
{
  bool bResult = true;

  for(auto& pSink : mpSinks ){
    bResult &= pSink->setVolume( getPerSinkChannelVolumes(pSink, perChannelVolumes) );
  }

  return bResult;
}


int MultipleSink::stateResourceConsumption(void)
{
  int nProcessingResource = 0;
  for(auto& pSink : mpSinks ){
    nProcessingResource += pSink->stateResourceConsumption();
  }

  return nProcessingResource;
}

