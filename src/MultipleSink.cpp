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


MultipleSink::MultipleSink(AudioFormat audioFormat):ISink(), mFormat(audioFormat), mMaxLatency(0)
{

}

MultipleSink::~MultipleSink()
{
  clearSinks();
}


void MultipleSink::attachSink(ISink* pSink, AudioFormat::ChannelMapper& map)
{
  if( pSink ){
    mpSinks.push_back( pSink );
    mChannelMaps.insert_or_assign( pSink, map );
  }
}

bool MultipleSink::detachSink(ISink* pSink, bool bDisposeSink)
{
  bool result = false;

  if( mChannelMaps.contains( pSink ) ){
    std::erase( mpSinks, pSink );
    mChannelMaps.erase( pSink );
    result = true;
    if( bDisposeSink ){
      delete pSink;
    }
  }

  return result;
}

void MultipleSink::clearSinks(bool bDisposeSinks)
{
  if( bDisposeSinks ){
    for(auto& pSink : mpSinks ){
      delete pSink;
    }
  }
  mpSinks.clear();
  mChannelMaps.clear();
}

void MultipleSink::ensureDelayFilters(bool bForceRecreate)
{
  bool isDelayFiltersEmpty = mpDelayFilters.empty();
  if( bForceRecreate || isDelayFiltersEmpty ){
    if( !isDelayFiltersEmpty ){
      for( auto& [pSink, pDelayFilter] : mpDelayFilters ){
        delete pDelayFilter;
      }
      mpDelayFilters.clear();
    }
    mMaxLatency = getLatencyUSec();
    for(auto& pSink : mpSinks ){
      mpDelayFilters.insert_or_assign( pSink, new DelayFilter( pSink->getAudioFormat(), mMaxLatency-pSink->getLatencyUSec() ) );
    }
  }
}


void MultipleSink::writePrimitive(IAudioBuffer& buf)
{
  AudioBuffer* pBuf = dynamic_cast<AudioBuffer*>(&buf);
  for(auto& pSink : mpSinks ){
    if( pBuf ){
      AudioFormat::ChannelMapper mapper = mChannelMaps[ pSink ];
      AudioBuffer selectedChannelData = pBuf->getSelectedChannelData( pSink->getAudioFormat(), mapper );
      ensureDelayFilters();
      AudioBuffer delayedOut( pSink->getAudioFormat(), pBuf->getNumberOfSamples() );
      DelayFilter* pDelayFilter = mpDelayFilters[ pSink ];
      pDelayFilter->process( selectedChannelData, delayedOut );
      pSink->write( delayedOut );
    } else {
      pSink->write( buf );
    }
  }
}

void MultipleSink::dump(void)
{
  std::cout << "MultipleSink::list of sinks" << std::endl;

  for(auto& pSink : mpSinks ){
    std::cout << "Sink:" << pSink << std::endl;
    AudioFormat::ChannelMapper mapper = mChannelMaps[ pSink ];
    for( const auto& [dstCh, srcCh] : mapper ){
      std::cout << "SrcCh:" << srcCh << " -> DstCh:" << dstCh << std::endl;
    }
    pSink->dump();
  }
}

bool MultipleSink::setAudioFormat(AudioFormat audioFormat)
{
  mFormat = audioFormat;
  return true;
}

AudioFormat MultipleSink::getAudioFormat(void)
{
  return mFormat;
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

bool MultipleSink::setVolume(float volumePercentage)
{
  bool bResult = ISink::setVolume(volumePercentage);

  for(auto& pSink : mpSinks ){
    bResult &= pSink->setVolume(volumePercentage);
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

