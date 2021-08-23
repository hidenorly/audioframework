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

#include "Pipe.hpp"
#include "PipeMultiThread.hpp"
#include "InterPipeBridge.hpp"
#include <vector>
#include <iostream>
#include <algorithm>

PipeMultiThread::PipeMultiThread() : mpSink(nullptr), mpSource(nullptr), mSinkAttached(false), mSourceAttached(false)
{

}

PipeMultiThread::~PipeMultiThread()
{
  stop();
  clearFilters();
}

std::shared_ptr<IPipe> PipeMultiThread::getHeadPipe(bool bCreateInstance)
{
  if( bCreateInstance && mPipes.empty() ){
    mPipes.insert( mPipes.begin(), std::make_shared<Pipe>() );
  }
  return mPipes.empty() ? nullptr : mPipes.front();
}

std::shared_ptr<IPipe> PipeMultiThread::getTailPipe(bool bCreateInstance)
{
  if( bCreateInstance && mPipes.empty() ){
    mPipes.push_back( std::make_shared<Pipe>() );
  }
  return mPipes.empty() ? nullptr : mPipes.back();
}

void PipeMultiThread::createAndConnectPipesToHead(std::shared_ptr<IPipe> pCurrentPipe)
{
  if( pCurrentPipe && !mPipes.empty() ){
    std::shared_ptr<IPipe> pNewPipe = std::make_shared<Pipe>();
    AudioFormat theUsingFormat = pCurrentPipe->getFilterAudioFormat();

    std::shared_ptr<InterPipeBridge> pInterBridge = std::make_shared<InterPipeBridge>( theUsingFormat );
    mInterPipeBridges.insert( mInterPipeBridges.begin(), pInterBridge );

    std::shared_ptr<ISource> pSource = pCurrentPipe->attachSource( pInterBridge );
    pNewPipe->attachSource( pSource );
    pNewPipe->attachSink( pInterBridge );
    mPipes.insert( mPipes.begin(), pNewPipe );
  }
}

void PipeMultiThread::createAndConnectPipesToTail(std::shared_ptr<IPipe> pCurrentPipe)
{
  if( pCurrentPipe && !mPipes.empty() ){
    std::shared_ptr<IPipe> pNewPipe = std::make_shared<Pipe>();
    AudioFormat theUsingFormat = pCurrentPipe->getFilterAudioFormat();

    std::shared_ptr<InterPipeBridge> pInterBridge = std::make_shared<InterPipeBridge>(theUsingFormat);
    mInterPipeBridges.push_back( pInterBridge  );

    std::shared_ptr<ISink> pSink = pCurrentPipe->attachSink( pInterBridge );
    pNewPipe->attachSink( pSink );
    pNewPipe->attachSource( pInterBridge );
    mPipes.push_back( pNewPipe );
  }
}

void PipeMultiThread::addFilterToHead(std::shared_ptr<IFilter> pFilter)
{
  if( pFilter ){
    mMutexFilters.lock();
    std::shared_ptr<IPipe> pPipe = getHeadPipe();
    if( pPipe ){
      int theFilterWindowSize = pFilter->getRequiredWindowSizeUsec();
      int thePipeWindowSize = pPipe->getWindowSizeUsec();

      if( theFilterWindowSize != thePipeWindowSize ){
        createAndConnectPipesToHead( pPipe );
      }
    }
    getHeadPipe(true)->addFilterToHead( pFilter );
    ensureSourceSink();
    mMutexFilters.unlock();
  }
}

void PipeMultiThread::addFilterToTail(std::shared_ptr<IFilter> pFilter)
{
  if( pFilter ){
    mMutexFilters.lock();
    std::shared_ptr<IPipe> pPipe = getTailPipe();
    if( pPipe ){
      int theFilterWindowSize = pFilter->getRequiredWindowSizeUsec();
      int thePipeWindowSize = pPipe->getWindowSizeUsec();

      if( theFilterWindowSize != thePipeWindowSize ){
        createAndConnectPipesToTail( pPipe );
      }
    }
    getTailPipe(true)->addFilterToTail( pFilter );
    ensureSourceSink();
    mMutexFilters.unlock();
  }
}

std::shared_ptr<IPipe> PipeMultiThread::findPipe(std::shared_ptr<IFilter> pFilter)
{
  std::shared_ptr<IPipe> result = nullptr;

  if( pFilter ){
    for( auto& aPipe : mPipes ){
      result = aPipe->isFilterIncluded( pFilter ) ? aPipe : nullptr;
      if( result ){
        break;
      }
    }
  }

  return result;
}

bool PipeMultiThread::isFilterIncluded(std::shared_ptr<IFilter> pFilter)
{
  bool result = false;

  if( pFilter ){
    for( auto& aPipe : mPipes ){
      result = aPipe->isFilterIncluded( pFilter );
      if( result ){
        break;
      }
    }
  }

  return result;
}

bool PipeMultiThread::addFilterAfterFilter(std::shared_ptr<IFilter> pFilter, std::shared_ptr<IFilter> pPosition)
{
  bool result = false;

  if( pFilter && pPosition ){
    mMutexFilters.lock();
    std::shared_ptr<IPipe> pPipe = findPipe( pFilter );
    if( pPipe ){
      int theFilterWindowSize = pFilter->getRequiredWindowSizeUsec();
      int thePipeWindowSize = pPipe->getWindowSizeUsec();

      if( theFilterWindowSize != thePipeWindowSize ) {
        // TODO : Create different pipe and interconnect
      } else {
        result = pPipe->addFilterAfterFilter( pFilter, pPosition );
      }
    }
    mMutexFilters.unlock();
  }

  return result;
}

bool PipeMultiThread::removeFilter(std::shared_ptr<IFilter> pFilter)
{
  bool result = false;

  for( auto& pPipe : mPipes ){
    if( pPipe->isFilterIncluded( pFilter ) ){
      pPipe->removeFilter( pFilter );
      result = true;
      break;
    }
  }

  return result;
}


std::shared_ptr<ISink> PipeMultiThread::attachSink(std::shared_ptr<ISink> pSink)
{
  std::shared_ptr<ISink> pResult = mpSink;
  mpSink = pSink;

  std::shared_ptr<IPipe> pPipe = getTailPipe();
  if( pPipe ){
    std::shared_ptr<ISink> pSinkFromPipe = pPipe->attachSink( pSink );
    mSinkAttached = true;
    pResult = pSinkFromPipe ? pSinkFromPipe : pResult;
  }

  return pResult;
}

std::shared_ptr<ISink> PipeMultiThread::detachSink(void)
{
  std::shared_ptr<ISink> pResult = mpSink;
  mpSink = nullptr;

  std::shared_ptr<IPipe> pPipe = getTailPipe();
  if( pPipe ){
    std::shared_ptr<ISink> pSinkFromPipe = pPipe->detachSink();
    mSourceAttached = false;
    pResult = pSinkFromPipe ? pSinkFromPipe : pResult;
  }

  return pResult;
}

std::shared_ptr<ISink> PipeMultiThread::getSinkRef(void)
{
  std::shared_ptr<ISink> pResult = mpSink;

  std::shared_ptr<IPipe> pPipe = getTailPipe();
  if( pPipe ){
    std::shared_ptr<ISink> pSinkFromPipe = pPipe->getSinkRef();
    pResult = pSinkFromPipe ? pSinkFromPipe : pResult;
  }

  return pResult;
}

std::shared_ptr<ISource> PipeMultiThread::attachSource(std::shared_ptr<ISource> pSource)
{
  std::shared_ptr<ISource> pResult = mpSource;
  mpSource = pSource;

  std::shared_ptr<IPipe> pPipe = getHeadPipe();
  if( pPipe ){
    std::shared_ptr<ISource> pSourceFromPipe = pPipe->attachSource( pSource );
    mSourceAttached = true;
    pResult = pSourceFromPipe ? pSourceFromPipe : pResult;
  }

  return pResult;
}

std::shared_ptr<ISource> PipeMultiThread::detachSource(void)
{
  std::shared_ptr<ISource> pResult = mpSource;
  mpSource = nullptr;

  std::shared_ptr<IPipe> pPipe = getHeadPipe();
  if( pPipe ){
    std::shared_ptr<ISource> pSourceFromPipe = pPipe->detachSource();
    mSourceAttached = false;
    pResult = pSourceFromPipe ? pSourceFromPipe : pResult;
  }

  return pResult;
}

std::shared_ptr<ISource> PipeMultiThread::getSourceRef(void)
{
  std::shared_ptr<ISource> pResult = mpSource;

  std::shared_ptr<IPipe> pPipe = getHeadPipe();
  if( pPipe ){
    std::shared_ptr<ISource> pSourceFromPipe = pPipe->getSourceRef();
    pResult = pSourceFromPipe ? pSourceFromPipe : pResult;
  }

  return pResult;
}

void PipeMultiThread::run(void)
{
  for( auto& pPipe : mPipes ){
    pPipe->run();
  }
}

void PipeMultiThread::stop(void)
{
  for( auto& pPipe : mPipes ){
    for( auto& pInterPipeBridge : mInterPipeBridges ) {
      pInterPipeBridge->unlock();
    }
    pPipe->stop();
  }
}

void PipeMultiThread::stopAndFlush(void)
{
  // TODO : need to consider more
  for( auto& pPipe : mPipes ){
    pPipe->stopAndFlush();
  }
}

bool PipeMultiThread::isRunning(void)
{
  bool bRunning = false;

  for( auto& pPipe : mPipes ){
    bRunning |= pPipe->isRunning();
  }

  return bRunning;
}


void PipeMultiThread::dump(void)
{
  std::cout << std::endl << "PipeMultiThread::dump" << std::endl;
  std::cout << "Interpipe bridges:" << std::endl;
  for( auto& pInterPipeBridge : mInterPipeBridges ){
    std::cout << pInterPipeBridge << std::endl;
  }

  std::cout << "Pipes:" << std::endl;
  for( auto& pPipe : mPipes ){
    std::cout << "pipe:" << pPipe << std::endl;
    pPipe->dump();
  }
}

void PipeMultiThread::clearFilters(void)
{
  mPipes.clear();
  mInterPipeBridges.clear();
}

AudioFormat PipeMultiThread::getFilterAudioFormat(AudioFormat theUsingFormat)
{
  throw std::runtime_error( "PipeMultiThread::getFilterAudioFormat() is unsupported" );
  return AudioFormat();
}

int PipeMultiThread::getWindowSizeUsec(void)
{
  int result = 0;

  for( auto& pPipe : mPipes ){
    int nSize = pPipe->getWindowSizeUsec();
    if( nSize > result ){
      result = nSize; // return max window size
    }
  }

  return result;
}

void PipeMultiThread::ensureSourceSink(void)
{
  if( !mSourceAttached && mpSource ){
    std::shared_ptr<IPipe> pHeadPipe = getHeadPipe();
    if( pHeadPipe ){
      pHeadPipe->attachSource( mpSource );
      mSourceAttached = true;
    }
  }
  if( !mSinkAttached && mpSink ){
    std::shared_ptr<IPipe> pTailPipe = getTailPipe();
    if( pTailPipe ){
      pTailPipe->attachSink( mpSink );
      mSinkAttached = true;
    }
  }
}

int PipeMultiThread::getLatencyUSec(void)
{
  int nLatency = 0;
  for( auto& pPipe : mPipes ){
    nLatency = std::max( nLatency, pPipe->getLatencyUSec() );
  }

  return nLatency;
}

int PipeMultiThread::stateResourceConsumption(void)
{
  int nProcessingResource = 0;
  for( auto& pPipe : mPipes ){
    nProcessingResource += pPipe->stateResourceConsumption();
  }
  nProcessingResource += ( mpSink ? mpSink->stateResourceConsumption() : 0 );
  nProcessingResource += ( mpSource ? mpSource->stateResourceConsumption() : 0 );

  return nProcessingResource;
}
