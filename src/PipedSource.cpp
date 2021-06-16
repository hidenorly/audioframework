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

#include "PipedSource.hpp"
#include "PipeMultiThread.hpp"
#include <cassert>

PipedSource::PipedSource() : ISource(), mpSource(nullptr)
{
  mpInterPipeBridge = new InterPipeBridge();
  mpPipe = new PipeMultiThread();
  mpPipe->attachSink ( mpInterPipeBridge );
}

PipedSource::~PipedSource()
{
  stop();
  clearFilters();
  delete mpPipe; mpPipe = nullptr;
  delete mpInterPipeBridge; mpInterPipeBridge = nullptr;
}

ISource* PipedSource::attachSource(ISource* pSource)
{
  bool bIsRunning = isRunning();
  if( bIsRunning ){
    stop();
  }

  ISource* prevSource = mpSource;
  mpSource = pSource;

  if( mpPipe ){
    ISource* prevPipeSource = mpPipe->attachSource( pSource );
    assert( !prevPipeSource || (prevPipeSource == prevSource) );
  }

  if( bIsRunning ){
    run();
  }

  return prevSource;
}

ISource* PipedSource::detachSource(void)
{
  if( isRunning() ){
    stop();
  }
  ISource* prevSource = mpSource;
  mpSource = nullptr;

  if( mpPipe ){
    ISource* prevPipeSource = mpPipe->detachSource();
    assert( !prevPipeSource || (prevPipeSource == prevSource) );
  }

  return prevSource;
}

void PipedSource::readPrimitive(IAudioBuffer& buf)
{
  if( mpSource && mpInterPipeBridge ){
    mpInterPipeBridge->read( buf );
  }
}

AudioFormat PipedSource::getAudioFormat(void)
{
  if( mpSource ){
    return mpSource->getAudioFormat();
  }
  return AudioFormat();
}

void PipedSource::addFilterToHead(std::shared_ptr<IFilter> pFilter)
{
  if( mpPipe ){
    mpPipe->addFilterToHead(pFilter);
  }
}

void PipedSource::addFilterToTail(std::shared_ptr<IFilter> pFilter)
{
  if( mpPipe ){
    mpPipe->addFilterToTail(pFilter);
  }
}

void PipedSource::run(void)
{
  if( mpPipe && mpSource && mpInterPipeBridge ){
    mpPipe->run();
  }
}

void PipedSource::stop(void)
{
  if( mpPipe ){
    mpPipe->stop();
  }
}

bool PipedSource::isRunning(void)
{
  if( mpPipe ){
    return mpPipe->isRunning();
  }
  return false;
}

void PipedSource::clearFilters(void)
{
  if( mpPipe ){
    return mpPipe->clearFilters();
  }
}

int PipedSource::stateResourceConsumption(void)
{
  int nProcessingResource = 0;
  nProcessingResource += ( mpSource ? mpSource->stateResourceConsumption() : 0 );
  nProcessingResource += ( mpPipe ? mpPipe->stateResourceConsumption() : 0 );

  return nProcessingResource;
}