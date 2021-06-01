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
#include "Filter.hpp"
#include "AudioFormat.hpp"
#include "Buffer.hpp"
#include "Util.hpp"
#include <iostream>
#include <string>
#include <numeric>
#include <stdexcept>
#include <utility>
#include <algorithm>

Pipe::Pipe():IPipe(), mpSink(nullptr), mpSource(nullptr)
{

}

Pipe::~Pipe()
{
  clearFilters();
  stop();
}

void Pipe::addFilterToHead(IFilter* pFilter)
{
  mMutexFilters.lock();
  mFilters.insert(mFilters.begin(), pFilter);
  mMutexFilters.unlock();
}

void Pipe::addFilterToTail(IFilter* pFilter)
{
  mMutexFilters.lock();
  mFilters.push_back(pFilter);
  mMutexFilters.unlock();
}

bool Pipe::addFilterAfterFilter(IFilter* pFilter, IFilter* pPosition)
{
  bool result = false;

  if( pPosition ){
    mMutexFilters.lock();
    auto it = std::find( mFilters.begin(), mFilters.end(), pPosition );
    if( it != mFilters.end() ){
      mFilters.insert( it+1, pFilter );
      result = true;
    }
    mMutexFilters.unlock();
  }

  return result;
}

bool Pipe::removeFilter(IFilter* pFilter)
{
  bool result = false;

  if( isFilterIncluded(pFilter) ){
    mMutexFilters.lock();
    std::erase( mFilters, pFilter );
    result = true;
    mMutexFilters.unlock();
  }

  return result;
}

bool Pipe::isFilterIncluded(IFilter* pFilter)
{
  auto it = std::find( mFilters.begin(), mFilters.end(), pFilter );
  return ( it != mFilters.end() ) ? true : false;
}

void Pipe::clearFilters(void)
{
  mMutexFilters.lock();
  for( auto& pFilter : mFilters ) {
    delete pFilter;
  }
  mFilters.clear();
  mMutexFilters.unlock();
}

ISink* Pipe::attachSink(ISink* pISink)
{
  ISink* pPrevISink = mpSink;
  mpSink = pISink;

  return pPrevISink;
}

ISink* Pipe::detachSink(void)
{
  ISink* pPrevISink = mpSink;
  mpSink = nullptr;

  return pPrevISink;
}

ISource* Pipe::attachSource(ISource* pISource)
{
  ISource* pPrevISource = mpSource;
  mpSource = pISource;
  return pPrevISource;
}

ISource* Pipe::detachSource(void)
{
  ISource* pPrevISource = mpSource;
  mpSource = nullptr;

  return pPrevISource;
}

void Pipe::dump(void)
{
  std::cout << "Source:" << (mpSource ? mpSource->toString() : "") << std::endl;
  std::cout << "Sink:" << (mpSink ? mpSink->toString() : "") << std::endl;

  std::cout << "Filters:" << std::endl;
  for( auto& pFilter : mFilters ) {
    std::cout << pFilter << std::endl;
  }
  std::cout << std::endl;
}

void Pipe::unlockToStop(void)
{
  IUnlockable* pSource = dynamic_cast<IUnlockable*>(mpSource);
  if( pSource ) pSource->unlock();

  IUnlockable* pSink = dynamic_cast<IUnlockable*>(mpSink);
  if( pSink ) pSink->unlock();
}


void Pipe::process(void)
{
  while(mbIsRunning && mpSource && mpSink){
    float windowSizeUsec = getCommonWindowSizeUsec();
    AudioFormat usingAudioFormat = getFilterAudioFormat();
    float usingSamplingRate = usingAudioFormat.getSamplingRate();
    float perSampleDurationUsec = 1000000.0f / usingSamplingRate;
    int samples = windowSizeUsec / perSampleDurationUsec;

    AudioBuffer* pInBuf = new AudioBuffer( usingAudioFormat, samples );
    AudioBuffer* pOutBuf= new AudioBuffer( usingAudioFormat, samples );
    AudioBuffer* pSinkOut = pInBuf;

    int nFilterSize = mFilters.size();
    while( mbIsRunning && ( nFilterSize == mFilters.size() ) ) {
      mpSource->read( *pInBuf );

      mMutexFilters.lock();
      for( auto& pFilter : mFilters ) {
        pFilter->process( *pInBuf, *pOutBuf );
        pSinkOut = pOutBuf;
        std::swap( pInBuf, pOutBuf );
      }
      mMutexFilters.unlock();

      // TODO : May change as directly write to the following buffer from the last filter to avoid the copy.
      mpSink->write( *pSinkOut );
    }

    delete pInBuf;  pInBuf = nullptr;
    delete pOutBuf; pOutBuf = nullptr;
  }
}

AudioFormat Pipe::getFilterAudioFormat(void)
{
  // TODO : Prepare different format choice example. Note that this is override-able.
  AudioFormat theUsingFormat; // default AudioFormat is set

  bool bPossibleToUseTheFormat = true;
  mMutexFilters.lock();
  for( auto& pFilter : mFilters ) {
    std::vector<AudioFormat> formats = pFilter->getSupportedAudioFormats();
    bool bCompatible = false;
    for( auto& aFormat : formats ){
      bCompatible |= theUsingFormat.equal(aFormat);
    }
    bPossibleToUseTheFormat &= bCompatible;
    if( !bPossibleToUseTheFormat ) break;
  }
  mMutexFilters.unlock();

  if( !bPossibleToUseTheFormat ){
    throw std::invalid_argument("There is no common audio format in the registered filters");
  }

  return theUsingFormat;
}

int Pipe::getCommonWindowSizeUsec(void)
{
  int result = 1;

  mMutexFilters.lock();
  for( auto& pFilter : mFilters ) {
    int windowSizeUsec = pFilter->getRequiredWindowSizeUsec();
    result = windowSizeUsec ? std::lcm(result, windowSizeUsec) : result;
  }
  mMutexFilters.unlock();

  return result;
}

int Pipe::getWindowSizeUsec(void)
{
  return getCommonWindowSizeUsec();
}

int Pipe::getLatencyUSec(void)
{
  int nProcessingTimeUsec = 0;
  mMutexFilters.lock();
  for( auto& pFilter : mFilters ) {
    nProcessingTimeUsec += pFilter->getExpectedProcessingUSec();
  }
  mMutexFilters.unlock();

  return getCommonWindowSizeUsec() + nProcessingTimeUsec;
}

int Pipe::stateResourceConsumption(void)
{
  int nProcessingResource = 0;
  mMutexFilters.lock();
  for( auto& pFilter : mFilters ) {
    nProcessingResource += pFilter->stateResourceConsumption();
  }
  mMutexFilters.unlock();
  nProcessingResource += ( mpSink ? mpSink->stateResourceConsumption() : 0 );
  nProcessingResource += ( mpSource ? mpSource->stateResourceConsumption() : 0 );

  return nProcessingResource;
}

