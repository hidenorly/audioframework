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
#include "PipeManager.hpp"
#include "InterPipeBridge.hpp"
#include <vector>
#include <iostream>

PipeManager::PipeManager() : mpSink(nullptr), mpSource(nullptr), mSinkAttached(false), mSourceAttached(false)
{

}

PipeManager::~PipeManager()
{
  clearFilers();
  stop();
}

IPipe* PipeManager::getHeadPipe(bool bCreateInstance)
{
  IPipe* result = nullptr;
  if( bCreateInstance && mPipes.empty() ){
    mPipes.insert(mPipes.begin(), new Pipe() );
  }
  return mPipes.empty() ? nullptr : mPipes.front();
}

IPipe* PipeManager::getTailPipe(bool bCreateInstance)
{
  if( bCreateInstance && mPipes.empty() ){
    mPipes.push_back( new Pipe() );
  }
  return mPipes.empty() ? nullptr : mPipes.back();
}

void PipeManager::createAndConnectPipesToHead(IPipe* pCurrentPipe)
{
  if( pCurrentPipe && !mPipes.empty() ){
    IPipe* pNewPipe = new Pipe();
    AudioFormat theUsingFormat = pCurrentPipe->getFilterAudioFormat();

    InterPipeBridge* pInterBridge = new InterPipeBridge(theUsingFormat);
    mInterPipeBridges.insert(mInterPipeBridges.begin(), pInterBridge );

    ISource* pSource = pCurrentPipe->attachSource( (ISource*)pInterBridge );
    pNewPipe->attachSource( pSource );
    pNewPipe->attachSink( (ISink*)pInterBridge );
    mPipes.insert(mPipes.begin(), pNewPipe );
  }
}

void PipeManager::createAndConnectPipesToTail(IPipe* pCurrentPipe)
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

void PipeManager::addFilterToHead(Filter* pFilter)
{
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
}

void PipeManager::addFilterToTail(Filter* pFilter)
{
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
}

ISink* PipeManager::attachSink(ISink* pSink)
{
  ISink* pResult = mpSink;
  mpSink = pSink;

  IPipe* pPipe = getTailPipe();
  if( pPipe ){
    ISink* pSinkFromPipe = pPipe->attachSink(pSink);
    mSinkAttached = true;
    pResult = pSinkFromPipe ? pSinkFromPipe : pResult;
  }

  return pResult;
}

ISink* PipeManager::detachSink(void)
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

ISource* PipeManager::attachSource(ISource* pSource)
{
  ISource* pResult = mpSource;
  mpSource = pSource;

  IPipe* pPipe = getHeadPipe();
  if( pPipe ){
    ISource* pSourceFromPipe = pPipe->attachSource(pSource);
    mSourceAttached = true;
    pResult = pSourceFromPipe ? pSourceFromPipe : pResult;
  }

  return pResult;
}

ISource* PipeManager::detachSource(void)
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

void PipeManager::run(void)
{
  for( auto& pPipe : mPipes ){
    pPipe->run();
  }
}

void PipeManager::stop(void)
{
  for( auto& pPipe : mPipes ){
    for( auto& pInterPipeBridge : mInterPipeBridges ) {
      pInterPipeBridge->unlock();
    }
    pPipe->stop();
  }
}

bool PipeManager::isRunning(void)
{
  bool bRunning = false;

  for( auto& pPipe : mPipes ){
    bRunning |= pPipe->isRunning();
  }

  return bRunning;
}


void PipeManager::dump(void)
{
  std::cout << std::endl << "PipeManager::dump" << std::endl;
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

void PipeManager::clearFilers(void)
{
  for( auto& pPipe : mPipes ){
    pPipe->clearFilers();
    delete pPipe;
  }
  mPipes.clear();

  for( auto& pInterPipeBridge : mInterPipeBridges ){
    delete pInterPipeBridge;
  }
  mInterPipeBridges.clear();
}

AudioFormat PipeManager::getFilterAudioFormat(void)
{
  throw std::runtime_error( "PipeManager::getFilterAudioFormat() is unsupported" );
  return AudioFormat();
}

int PipeManager::getWindowSizeUsec(void)
{
  int result = 0;

  for(auto& pPipe : mPipes ){
    int nSize = pPipe->getWindowSizeUsec();
    if( nSize > result ){
      result = nSize; // return max window size
    }
  }

  return result;
}

void PipeManager::ensureSourceSink(void)
{
  if( !mSourceAttached && mpSource ){
    IPipe* pHeadPipe = getHeadPipe();
    if( pHeadPipe ){
      pHeadPipe->attachSource(mpSource);
      mSourceAttached = true;
    }
  }
  if( !mSinkAttached && mpSink ){
    IPipe* pTailPipe = getTailPipe();
    if( pTailPipe ){
      pTailPipe->attachSink(mpSink);
      mSinkAttached = true;
    }
  }
}


