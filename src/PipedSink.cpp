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
#include "PipeMultiThread.hpp"
#include <cassert>

PipedSink::PipedSink( std::shared_ptr<ISink> pSink ) : ISink(), mpSink(pSink)
{
  mpInterPipeBridge = std::make_shared<InterPipeBridge>();
  mpPipe = std::make_shared<PipeMultiThread>();
  mpPipe->attachSource ( mpInterPipeBridge );
}

PipedSink::~PipedSink()
{
  stop();
  clearFilters();
  mpPipe.reset();
  mpInterPipeBridge.reset();
}

std::shared_ptr<ISink> PipedSink::attachSink(std::shared_ptr<ISink> pSink)
{
  bool bIsRunning = isRunning();
  if( bIsRunning ){
    stop();
  }

  std::shared_ptr<ISink> prevSink = mpSink;
  mpSink = pSink;

  if( mpPipe ){
    std::shared_ptr<ISink> prevPipeSink = mpPipe->attachSink( pSink );
    assert( !prevPipeSink || (prevPipeSink == prevSink) );
  }

  if( bIsRunning ){
    run();
  }

  return prevSink;
}

std::shared_ptr<ISink> PipedSink::detachSink(void)
{
  if( isRunning() ){
    stop();
  }
  std::shared_ptr<ISink> prevSink = mpSink;
  mpSink = nullptr;

  if( mpPipe ){
    std::shared_ptr<ISink> prevPipeSink = mpPipe->detachSink();
    assert( !prevPipeSink || (prevPipeSink == prevSink) );
  }

  return prevSink;
}

void PipedSink::writePrimitive(IAudioBuffer& buf)
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

void PipedSink::setAudioFormatPrimitive(AudioFormat audioFormat)
{
  if( mpSink ){
    bool bIsRunning = isRunning();
    if( bIsRunning ){
      stop();
    }
    mpInterPipeBridge = std::make_shared<InterPipeBridge>( audioFormat );
    if( mpPipe ){
      mpPipe->attachSource( mpInterPipeBridge );
    }
    mpSink->setAudioFormat( audioFormat );
    if( bIsRunning ){
      run();
    }
  }
}

AudioFormat PipedSink::getAudioFormat(void)
{
  if( mpSink ){
    return mpSink->getAudioFormat();
  }
  return AudioFormat();
}

void PipedSink::addFilterToHead(std::shared_ptr<IFilter> pFilter)
{
  if( mpPipe ){
    mpPipe->addFilterToHead(pFilter);
  }
}

void PipedSink::addFilterToTail(std::shared_ptr<IFilter> pFilter)
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

int PipedSink::stateResourceConsumption(void)
{
  int nProcessingResource = 0;
  nProcessingResource += ( mpSink ? mpSink->stateResourceConsumption() : 0 );
  nProcessingResource += ( mpPipe ? mpPipe->stateResourceConsumption() : 0 );

  return nProcessingResource;
}