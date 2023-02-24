/*
  Copyright (C) 2021, 2023  hidenorly

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

#include "MixerSplitter.hpp"
#include "Mixer.hpp"
#include <algorithm>
#include <set>
#include <iostream>

bool MixerSplitter::isPipeRunningOrNotRegistered(std::shared_ptr<ISink> srcSink)
{
  std::shared_ptr<IPipe> pPipe = nullptr;
  if( mpSourcePipes.contains( srcSink ) ){
    pPipe = mpSourcePipes[ srcSink ].lock();
  }
  return ( (pPipe && pPipe->isRunning()) || !pPipe );
}

/*
  @desc return true if add/removeSink, allocate/releaseSinkAdaptor, map/unmap/conditionalMap, source(=SinkAdaptor)'s format changed.
  TODO: improve the format change by using AudioFormat's format change listener
*/
bool MixerSplitter::isSituationChanged(void)
{
  bool bIsChanged = mbOnChanged;
  mbOnChanged = false;
  if( bIsChanged ) return bIsChanged;
  for( auto& pSource : mpSources ){
    if( !mpSourceAudioFormats.contains( pSource ) || !mpSourceAudioFormats[pSource]->equal( pSource->getAudioFormat() )){
      bIsChanged = true;
      mpSourceAudioFormats.insert_or_assign( pSource, pSource->getAudioFormat().getCopiedNewSharedInstance() );
      break;
    }
  }
  return bIsChanged;
}


void MixerSplitter::process(void)
{
  while( mbIsRunning && !mpSinks.empty() && !mpSources.empty() ){
    if( isSituationChanged() ){
      mMutexSourceSink.lock();
      std::map<std::shared_ptr<ISink>, std::vector<std::shared_ptr<ISink>>> mapper;
      for(auto& pConditionMapper : mSourceSinkMapper){
        if( !mapper.contains( pConditionMapper->sink ) ){
          std::vector<std::shared_ptr<ISink>> emptyArray;
          mapper.insert_or_assign( pConditionMapper->sink, emptyArray );
        }
        std::shared_ptr<IPipe> pPipe = mpSourcePipes[ pConditionMapper->source ].lock();
        if( ( (pPipe && pPipe->isRunning()) || !pPipe ) && pConditionMapper->condition->canHandle( pConditionMapper->source->getAudioFormat() ) ){
          mapper[ pConditionMapper->sink ].push_back( pConditionMapper->source );
        }
      }

      for( auto& [pSink, pSources] : mapper ){
        // TODO: Improve only 1 stream mix(=no mix) case by bypassing PipeMixer (=Direct write to Sink) by wrapper class of InterPipeBridge
        // ensure PipeMixer
        if( !mpMixers.contains(pSink) ){
          mpMixers.insert_or_assign( pSink, std::make_shared<PipeMixer>( pSink->getAudioFormat(), pSink ) );
        }
        // setup PipeMixer
        std::shared_ptr<PipeMixer> pPipeMixer = mpMixers[pSink];
        pPipeMixer->attachSink( pSink );
        std::vector<std::shared_ptr<ISink>> sources = pPipeMixer->getSinkAdaptors();
        for( auto& pSinkAdaptor : pSources ){
          pPipeMixer->attachSinkAdaptor( std::dynamic_pointer_cast<InterPipeBridge>(pSinkAdaptor), mpSourcePipes[ pSinkAdaptor ].lock() );
          std::erase( sources, pSinkAdaptor );
        }
        // remove unused sink adaptors(=Source for the mixer) from pPipeMixer
        for( auto& pSinkAdaptor : sources ){
          pPipeMixer->releaseSinkAdaptor( pSinkAdaptor );
        }

        pPipeMixer->run();
      }
      mMutexSourceSink.unlock();
    } else {
      std::this_thread::sleep_for(std::chrono::microseconds(1000));
    }
  }
  for( auto& [pSink, pPipeMixer] : mpMixers ){
    pPipeMixer->stop();
  }
}

void MixerSplitter::unlockToStop(void)
{
  for( auto& pSink : mpSources ){
    std::shared_ptr<IUnlockable> pLocakable = std::dynamic_pointer_cast<IUnlockable>(pSink);
    if( pLocakable ){
      pLocakable->unlock();
    }
  }
  for( auto& pSink : mpSinks ){
    std::shared_ptr<IUnlockable> pLocakable = std::dynamic_pointer_cast<IUnlockable>(pSink);
    if( pLocakable ){
      pLocakable->unlock();
    }
  }
}

MixerSplitter::MixerSplitter():ThreadBase(),mbOnChanged(false)
{

}

MixerSplitter::~MixerSplitter()
{
  stop();
  mSourceSinkMapper.clear();
  mpSources.clear();
  mpSourceAudioFormats.clear();
  mpSourcePipes.clear();
  mpSinks.clear();
  mpMixers.clear();
}

std::vector<std::shared_ptr<ISink>> MixerSplitter::getAllOfSinks(void)
{
  return mpSinks;
}

void MixerSplitter::attachSink(std::shared_ptr<ISink> pSink)
{
  mMutexSourceSink.lock();
  mpSinks.push_back( pSink );
  mMutexSourceSink.unlock();
  mbOnChanged = true;
}

bool MixerSplitter::detachSink(std::shared_ptr<ISink> pSink)
{
  bool result = false;
  mMutexSourceSink.lock();
  if( isSinkAvailableLocked(pSink) ){
    std::shared_ptr<IUnlockable> pLockable = std::dynamic_pointer_cast<IUnlockable>(pSink);
    if( pLockable ){
      pLockable->unlock();
    }
    if( mpMixers.contains( pSink ) ){
      mpMixers[pSink]->stop();
      mpMixers.erase(pSink);
    }
    int nCurrentSize = mpSinks.size();
    std::erase( mpSinks, pSink );
    result = (mpSinks.size() == nCurrentSize);
    bool bFound = true;
    while(bFound){
      std::shared_ptr<SourceSinkConditionMapper> mapper = getSourceSinkMapperLocked(nullptr, pSink);
      bFound = (nullptr != mapper);
      if( mapper ){
        std::erase( mSourceSinkMapper, mapper );
      }
    }
  }
  mMutexSourceSink.unlock();
  mbOnChanged = true;

  return result;
}

std::vector<std::shared_ptr<ISink>> MixerSplitter::getAllOfSinkAdaptors(void)
{
  return mpSources;
}

std::shared_ptr<ISink> MixerSplitter::allocateSinkAdaptor(AudioFormat format, std::shared_ptr<IPipe> pPipe)
{
  std::shared_ptr<InterPipeBridge> pInterPipeBridge = std::make_shared<InterPipeBridge>( format );
  mMutexSourceSink.lock();
  mpSources.push_back( pInterPipeBridge );
  mpSourcePipes.insert_or_assign( pInterPipeBridge, pPipe );
  mpSourceAudioFormats.insert_or_assign( pInterPipeBridge, format.getCopiedNewSharedInstance() );
  mMutexSourceSink.unlock();
  mbOnChanged = true;
  return pInterPipeBridge;
}

bool MixerSplitter::releaseSinkAdaptor(std::shared_ptr<ISink> pSink)
{
  bool result = false;
  mMutexSourceSink.lock();
  if( isSourceAvailableLocked(pSink) ){
    std::shared_ptr<IUnlockable> pLockable = std::dynamic_pointer_cast<IUnlockable>(pSink);
    if( pLockable ){
      pLockable->unlock();
    }
    int nCurrentSize = mpSources.size();
    std::erase( mpSources, pSink );
    mpSourcePipes.erase( pSink );
    mpSourceAudioFormats.erase( pSink );
    result = (mpSources.size() == nCurrentSize);
    removeMapperLocked(pSink);
  }
  mMutexSourceSink.unlock();
  mbOnChanged = true;

  return result;
}

bool MixerSplitter::isSinkAvailableLocked(std::shared_ptr<ISink> pSink)
{
  return std::find(mpSinks.begin(), mpSinks.end(), pSink) != mpSinks.end();
}

bool MixerSplitter::isSourceAvailableLocked(std::shared_ptr<ISink> pSink)
{
  return std::find(mpSources.begin(), mpSources.end(), pSink) != mpSources.end();
}

std::shared_ptr<MixerSplitter::SourceSinkConditionMapper> MixerSplitter::getSourceSinkMapperLocked(std::shared_ptr<ISink> pSource, std::shared_ptr<ISink> pSink)
{
  std::shared_ptr<SourceSinkConditionMapper> result = nullptr;
  if( pSource || pSink ){
    for( auto& aMapper : mSourceSinkMapper ){
      if( ( pSource && ( aMapper->source == pSource ) ) && ( pSink && ( aMapper->sink == pSink ) ) ){
        result = aMapper;
        break;
      }
    }
  }
  return result;
}

std::shared_ptr<MixerSplitter::SourceSinkMapper> MixerSplitter::getSourceSinkMapperLocked(std::shared_ptr<ISink> pSource)
{
  std::shared_ptr<SourceSinkMapper> result = nullptr;
  if( pSource ){
    for( auto& aMapper : mSourceSinkMapper ){
      if( aMapper->source == pSource ){
        if( aMapper->condition && aMapper->condition->canHandle( pSource->getAudioFormat() ) ){
          result = aMapper;
          break;
        }
      }
    }
  }
  return result;
}


bool MixerSplitter::conditionalMap(std::shared_ptr<ISink> srcSink, std::shared_ptr<ISink> dstSink, std::shared_ptr<MixerSplitter::MapCondition> condition)
{
  mMutexSourceSink.lock();
  bool result = isSourceAvailableLocked(srcSink) && isSinkAvailableLocked(dstSink);
  if( result ){
    mSourceSinkMapper.push_back( std::make_shared<SourceSinkConditionMapper>(srcSink, dstSink, condition) );
  }
  mMutexSourceSink.unlock();
  mbOnChanged = true;
  return result;
}

bool MixerSplitter::map(std::shared_ptr<ISink> srcSink, std::shared_ptr<ISink> dstSink)
{
  bool result = conditionalMap( srcSink, dstSink, std::make_shared<MapAnyCondition>() );
  mbOnChanged = true;
  return result;
}

bool MixerSplitter::removeMapperLocked(std::shared_ptr<ISink> srcSink)
{
  bool result = false;

  std::set<std::shared_ptr<SourceSinkConditionMapper>> mappers;
  for( auto& aMapper : mSourceSinkMapper ){
    if( aMapper->source == srcSink ){
      mappers.insert( aMapper );
    }
  }
  for( auto& aMapper : mappers ){
    std::erase( mSourceSinkMapper, aMapper );
    result = true;
  }

  return result;
}

bool MixerSplitter::unmap(std::shared_ptr<ISink> srcSink)
{
  bool result = false;
  mMutexSourceSink.lock();
  result = removeMapperLocked( srcSink );
  mMutexSourceSink.unlock();
  mbOnChanged = true;
  return result;
}

void MixerSplitter::dump(void)
{
  std::cout << "MixerSplitter::list of sinks" << std::endl;

  for(auto& pSink : mpSinks ){
    std::cout << "Sink:" << pSink->toString() << ":" << pSink << " latency: " << pSink->getLatencyUSec() << std::endl;
    pSink->dump();
  }
}

