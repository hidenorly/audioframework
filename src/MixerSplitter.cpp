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
  return ( pPipe && pPipe->isRunning() || !pPipe );
}


void MixerSplitter::mixPrimitiveLocked(std::vector<std::shared_ptr<ISink>> pSources, std::shared_ptr<ISink> pSink)
{
  if( pSink->getAudioFormat().isEncodingPcm() ){
    int nSamples = 256;
    AudioFormat outFormat = pSink->getAudioFormat();
    AudioBuffer outBuf( outFormat, nSamples );
    std::vector<AudioBuffer*> buffers;
    int nCurrentPipeSize = pSources.size();
    for(int i=0; i<nCurrentPipeSize; i++){
      buffers.push_back( new AudioBuffer(outFormat, nSamples) );
    }

    for(int i=0; mbIsRunning && i<pSources.size(); i++){
      bool bZeroData = true;
      if( isPipeRunningOrNotRegistered( pSources[i] ) ){
        std::shared_ptr<InterPipeBridge> pSource = std::dynamic_pointer_cast<InterPipeBridge>(pSources[i]);
        if( pSource && pSource->getAudioFormat().isEncodingPcm() ){
          pSource->read( *buffers[i] );
          bZeroData = false;
        }
      }
      if( bZeroData ) {
        ByteBuffer zeroBuffer( buffers[i]->getRawBuffer().size(), 0 );
        buffers[i]->setRawBuffer(zeroBuffer);
      }
    }

    if( mbIsRunning ){
      Mixer::process( buffers, &outBuf );
    }
    if( mbIsRunning ){
      pSink->write( outBuf );
    }
    for( auto& pBuffer : buffers ){
      delete pBuffer;
    }
    buffers.clear();
  } else {
    CompressAudioBuffer buf;
    for(auto& aSource : pSources){
      std::shared_ptr<InterPipeBridge> pSource = std::dynamic_pointer_cast<InterPipeBridge>(aSource);
      if( isPipeRunningOrNotRegistered( aSource ) && pSource ){
        pSource->read( buf );
        break;
      }
    }
    pSink->write( buf );
  }
}

void MixerSplitter::process(void)
{
  while( mbIsRunning && !mpSinks.empty() && !mpSources.empty() ){
    mMutexSourceSink.lock();

    std::map<std::shared_ptr<ISink>, std::vector<std::shared_ptr<ISink>>> mapper;
    std::set<std::shared_ptr<ISink>> pSinks;
    for(auto& pConditionMapper : mSourceSinkMapper){
      if( !mapper.contains( pConditionMapper->sink ) ){
        std::vector<std::shared_ptr<ISink>> emptyArray;
        mapper.insert_or_assign( pConditionMapper->sink, emptyArray );
        pSinks.insert( pConditionMapper->sink );
      }
      std::shared_ptr<IPipe> pPipe = mpSourcePipes[ pConditionMapper->source ].lock();
      if( pPipe && pPipe->isRunning() && pConditionMapper->condition->canHandle( pConditionMapper->sink->getAudioFormat() ) ){
        mapper[ pConditionMapper->sink ].push_back( pConditionMapper->source );
      }
    }

    // TODO: following mixPrimitiveLocked() should be executed in parallell by thread.
    for( auto& pSink : pSinks ){ // for( auto& pSink, pSources : mapper ){ cannot enumerate...
        mixPrimitiveLocked(mapper[pSink], pSink );
    }

    mMutexSourceSink.unlock();
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

MixerSplitter::MixerSplitter():ThreadBase()
{

}

MixerSplitter::~MixerSplitter()
{
  stop();
  mSourceSinkMapper.clear();
  mpSources.clear();
  mpSourcePipes.clear();
  mpSinks.clear();
}

std::vector<std::shared_ptr<ISink>> MixerSplitter::getAllOfSinks(void)
{
  return mpSinks;
}

void MixerSplitter::addSink(std::shared_ptr<ISink> pSink)
{
  mMutexSourceSink.lock();
  mpSinks.push_back( pSink );
  mMutexSourceSink.unlock();
}

bool MixerSplitter::removeSink(std::shared_ptr<ISink> pSink)
{
  bool result = false;
  mMutexSourceSink.lock();
  if( isSinkAvailableLocked(pSink) ){
    std::shared_ptr<IUnlockable> pLockable = std::dynamic_pointer_cast<IUnlockable>(pSink);
    if( pLockable ){
      pLockable->unlock();
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
  mMutexSourceSink.unlock();
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
    result = (mpSources.size() == nCurrentSize);
    removeMapperLocked(pSink);
  }
  mMutexSourceSink.unlock();

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
  bool result = isSourceAvailableLocked(srcSink) & isSinkAvailableLocked(dstSink);
  if( result ){
    mSourceSinkMapper.push_back( std::make_shared<SourceSinkConditionMapper>(srcSink, dstSink, condition) );
  }
  mMutexSourceSink.unlock();
  return result;
}

bool MixerSplitter::map(std::shared_ptr<ISink> srcSink, std::shared_ptr<ISink> dstSink)
{
  return conditionalMap( srcSink, dstSink, std::make_shared<MapAnyCondition>() );
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

