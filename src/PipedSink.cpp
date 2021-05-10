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

#include "PipedSink.hpp"
#include "PipeManager.hpp"

PipedSink::PipedSink() : mpSink(nullptr)
{
  mpInterPipeBridge = new InterPipeBridge();
  mpPipe = new PipeManager();
  mpPipe->attachSource ( mpInterPipeBridge );
}

PipedSink::~PipedSink()
{
  stop();
  clearFilters();
  delete mpPipe; mpPipe = nullptr;
  delete mpInterPipeBridge; mpInterPipeBridge = nullptr;
}

ISink* PipedSink::attachSink(ISink* pSink)
{
  bool bIsRunning = isRunning();
  if( bIsRunning ){
    stop();
  }

  ISink* prevSink = mpSink;
  mpSink = pSink;

  if( mpPipe ){
    ISink* prevPipeSink = mpPipe->attachSink( pSink );
    assert( !prevPipeSink || (prevPipeSink == prevSink) );
  }

  if( bIsRunning ){
    run();
  }

  return prevSink;
}

ISink* PipedSink::detachSink(void)
{
  if( isRunning() ){
    stop();
  }
  ISink* prevSink = mpSink;
  mpSink = nullptr;

  if( mpPipe ){
    ISink* prevPipeSink = mpPipe->detachSink();
    assert( !prevPipeSink || (prevPipeSink == prevSink) );
  }

  return prevSink;
}

void PipedSink::write(AudioBuffer& buf)
{
  if( mpSink && mpInterPipeBridge ){
    mpInterPipeBridge->write( buf );
  }
}

void PipedSink::dump(void)
{
  if( mpSink ){
    mpSink->dump();
  }
}

bool PipedSink::setAudioFormat(AudioFormat audioFormat)
{
  bool result = false;
  if( mpSink ){
    bool bIsRunning = isRunning();
    if( bIsRunning ){
      stop();
    }
    delete mpInterPipeBridge; mpInterPipeBridge = nullptr;
    mpInterPipeBridge = new InterPipeBridge( audioFormat );
    if( mpPipe ){
      ISource* pSource = mpPipe->attachSource( mpInterPipeBridge );
      delete pSource; pSource = nullptr;
    }
    result = mpSink->setAudioFormat( audioFormat );
    if( bIsRunning ){
      run();
    }
  }
  return result;
}

AudioFormat PipedSink::getAudioFormat(void)
{
  if( mpSink ){
    return mpSink->getAudioFormat();
  }
  return AudioFormat();
}

void PipedSink::addFilterToHead(Filter* pFilter)
{
  if( mpPipe ){
    mpPipe->addFilterToHead(pFilter);
  }
}

void PipedSink::addFilterToTail(Filter* pFilter)
{
  if( mpPipe ){
    mpPipe->addFilterToTail(pFilter);
  }
}

void PipedSink::run(void)
{
  if( mpPipe && mpSink && mpInterPipeBridge ){
    mpPipe->run();
  }
}

void PipedSink::stop(void)
{
  if( mpPipe ){
    mpPipe->stop();
  }
}

bool PipedSink::isRunning(void)
{
  if( mpPipe ){
    return mpPipe->isRunning();
  }
  return false;
}

void PipedSink::clearFilters(void)
{
  if( mpPipe ){
    return mpPipe->clearFilters();
  }
}
