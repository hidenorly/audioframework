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

IPipe* PipeMultiThread::getHeadPipe(bool bCreateInstance)
{
  IPipe* result = nullptr;
  if( bCreateInstance && mPipes.empty() ){
    mPipes.insert( mPipes.begin(), new Pipe() );
  }
  return mPipes.empty() ? nullptr : mPipes.front();
}

IPipe* PipeMultiThread::getTailPipe(bool bCreateInstance)
{
  if( bCreateInstance && mPipes.empty() ){
    mPipes.push_back( new Pipe() );
  }
  return mPipes.empty() ? nullptr : mPipes.back();
}

void PipeMultiThread::createAndConnectPipesToHead(IPipe* pCurrentPipe)
{
  if( pCurrentPipe && !mPipes.empty() ){
    IPipe* pNewPipe = new Pipe();
    AudioFormat theUsingFormat = pCurrentPipe->getFilterAudioFormat();

    InterPipeBridge* pInterBridge = new InterPipeBridge( theUsingFormat );
    mInterPipeBridges.insert( mInterPipeBridges.begin(), pInterBridge );

    ISource* pSource = pCurrentPipe->attachSource( (ISource*)pInterBridge );
    pNewPipe->attachSource( pSource );
    pNewPipe->attachSink( (ISink*)pInterBridge );
    mPipes.insert( mPipes.begin(), pNewPipe );
  }
}

void PipeMultiThread::createAndConnectPipesToTail(IPipe* pCurrentPipe)
{
  if( pCurrentPipe && !mPipes.empty() ){
    IPipe* pNewPipe = new Pipe();
    AudioFormat theUsingFormat = pCurrentPipe->getFilterAudioFormat();

    InterPipeBridge* pInterBridge = new InterPipeBridge(theUsingFormat);
    mInterPipeBridges.push_back( pInterBridge  );

    ISink* pSink = pCurrentPipe->attachSink( (ISink*)pInterBridge );
    pNewPipe->attachSink( pSink );
    pNewPipe->attachSource( (ISource*)pInterBridge );
    mPipes.push_back( pNewPipe );
  }
}

void PipeMultiThread::addFilterToHead(std::shared_ptr<IFilter> pFilter)
{
  if( pFilter ){
    mMutexFilters.lock();
    IPipe* pPipe = getHeadPipe();
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
    IPipe* pPipe = getTailPipe();
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

IPipe* PipeMultiThread::findPipe(std::shared_ptr<IFilter> pFilter)
{
  IPipe* result = nullptr;

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
    IPipe* pPipe = findPipe( pFilter );
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


ISink* PipeMultiThread::attachSink(ISink* pSink)
{
  ISink* pResult = mpSink;
  mpSink = pSink;

  IPipe* pPipe = getTailPipe();
  if( pPipe ){
    ISink* pSinkFromPipe = pPipe->attachSink( pSink );
    mSinkAttached = true;
    pResult = pSinkFromPipe ? pSinkFromPipe : pResult;
  }

  return pResult;
}

ISink* PipeMultiThread::detachSink(void)
{
  ISink* pResult = mpSink;
  mpSink = nullptr;

  IPipe* pPipe = getTailPipe();
  if( pPipe ){
    ISink* pSinkFromPipe = pPipe->detachSink();
    mSourceAttached = false;
    pResult = pSinkFromPipe ? pSinkFromPipe : pResult;
  }

  return pResult;
}

ISource* PipeMultiThread::attachSource(ISource* pSource)
{
  ISource* pResult = mpSource;
  mpSource = pSource;

  IPipe* pPipe = getHeadPipe();
  if( pPipe ){
    ISource* pSourceFromPipe = pPipe->attachSource( pSource );
    mSourceAttached = true;
    pResult = pSourceFromPipe ? pSourceFromPipe : pResult;
  }

  return pResult;
}

ISource* PipeMultiThread::detachSource(void)
{
  ISource* pResult = mpSource;
  mpSource = nullptr;

  IPipe* pPipe = getHeadPipe();
  if( pPipe ){
    ISource* pSourceFromPipe = pPipe->detachSource();
    mSourceAttached = false;
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
  for( auto& pPipe : mPipes ){
    pPipe->clearFilters();
    delete pPipe;
  }
  mPipes.clear();

  for( auto& pInterPipeBridge : mInterPipeBridges ){
    delete pInterPipeBridge;
  }
  mInterPipeBridges.clear();
}

AudioFormat PipeMultiThread::getFilterAudioFormat(void)
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
    IPipe* pHeadPipe = getHeadPipe();
    if( pHeadPipe ){
      pHeadPipe->attachSource( mpSource );
      mSourceAttached = true;
    }
  }
  if( !mSinkAttached && mpSink ){
    IPipe* pTailPipe = getTailPipe();
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
