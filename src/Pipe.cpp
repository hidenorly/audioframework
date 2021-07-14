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

void Pipe::addFilterToHead(std::shared_ptr<IFilter> pFilter)
{
  mMutexFilters.lock();
  mFilters.insert(mFilters.begin(), pFilter);
  mMutexFilters.unlock();
}

void Pipe::addFilterToTail(std::shared_ptr<IFilter> pFilter)
{
  mMutexFilters.lock();
  mFilters.push_back(pFilter);
  mMutexFilters.unlock();
}

bool Pipe::addFilterAfterFilter(std::shared_ptr<IFilter> pFilter, std::shared_ptr<IFilter> pPosition)
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

bool Pipe::removeFilter(std::shared_ptr<IFilter> pFilter)
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

bool Pipe::isFilterIncluded(std::shared_ptr<IFilter> pFilter)
{
  auto it = std::find( mFilters.begin(), mFilters.end(), pFilter );
  return ( it != mFilters.end() ) ? true : false;
}

void Pipe::clearFilters(void)
{
  mMutexFilters.lock();
  mFilters.clear();
  mMutexFilters.unlock();
}

std::shared_ptr<ISink> Pipe::attachSink(std::shared_ptr<ISink> pISink)
{
  std::shared_ptr<ISink> pPrevISink = mpSink;
  mMutexSink.lock();
  mpSink = pISink;
  mMutexSink.unlock();

  return pPrevISink;
}

std::shared_ptr<ISink> Pipe::detachSink(void)
{
  std::shared_ptr<ISink> pPrevISink = mpSink;
  mpSink = nullptr;

  return pPrevISink;
}

std::shared_ptr<ISource> Pipe::attachSource(std::shared_ptr<ISource> pISource)
{
  std::shared_ptr<ISource> pPrevISource = mpSource;
  mMutexSource.lock();
  mpSource = pISource;
  mMutexSource.unlock();
  return pPrevISource;
}

std::shared_ptr<ISource> Pipe::detachSource(void)
{
  std::shared_ptr<ISource> pPrevISource = mpSource;
  mpSource = nullptr;

  return pPrevISource;
}

void Pipe::dump(void)
{
  std::cout << "Source:" << (mpSource ? mpSource->toString() : "") << std::endl;
  std::cout << "Sink:" << (mpSink ? mpSink->toString() : "") << std::endl;

  std::cout << "Filters:" << std::endl;
  for( auto& pFilter : mFilters ) {
    std::cout << pFilter->toString() << std::endl;
  }
  std::cout << std::endl;
}

void Pipe::unlockToStop(void)
{
  std::shared_ptr<IUnlockable> pSource = std::dynamic_pointer_cast<IUnlockable>(mpSource);
  if( pSource ) pSource->unlock();

  std::shared_ptr<IUnlockable> pSink = std::dynamic_pointer_cast<IUnlockable>(mpSink);
  if( pSink ) pSink->unlock();
}


void Pipe::process(void)
{
  while(mbIsRunning && mpSource && mpSink){
    if( mpSource->getAudioFormat().isEncodingPcm() && mpSink->getAudioFormat().isEncodingPcm() ){
      // TODO: Should check not only filter format but also source/sink formats.
      float windowSizeUsec = getCommonWindowSizeUsec();
      AudioFormat usingAudioFormat = getFilterAudioFormat( mpSink->getAudioFormat() );
      float usingSamplingRate = usingAudioFormat.getSamplingRate();
      float perSampleDurationUsec = 1000000.0f / usingSamplingRate;
      int samples = windowSizeUsec / perSampleDurationUsec;

      AudioBuffer* pInBuf = new AudioBuffer( usingAudioFormat, samples );
      AudioBuffer* pOutBuf= new AudioBuffer( usingAudioFormat, samples );
      AudioBuffer* pSinkOut = pInBuf;

      int nFilterSize = mFilters.size();
      while( mbIsRunning && ( nFilterSize == mFilters.size() && (mpSource->getAudioFormat().isEncodingPcm() && mpSink->getAudioFormat().isEncodingPcm())) ) {
        // TODO: implement wait during muting and implement unlock for the mute wait
        mMutexSource.lock();
        mpSource->read( *pInBuf );
        mMutexSource.unlock();

        mMutexFilters.lock();
        for( auto& pFilter : mFilters ) {
          pFilter->process( *pInBuf, *pOutBuf );
          pSinkOut = pOutBuf;
          std::swap( pInBuf, pOutBuf );
        }
        mMutexFilters.unlock();

        // TODO : May change as directly write to the following buffer from the last filter to avoid the copy.
        mMutexSink.lock();
        mpSink->write( *pSinkOut );
        mMutexSink.unlock();
      }

      delete pInBuf;  pInBuf = nullptr;
      delete pOutBuf; pOutBuf = nullptr;
    } else {
      while( mbIsRunning && (mpSource->getAudioFormat().isEncodingCompressed() && mpSink->getAudioFormat().isEncodingCompressed()) ) {
        CompressAudioBuffer buf;
        mMutexSource.lock();
        mpSource->read( buf );
        mMutexSource.unlock();
        mMutexSink.lock();
        mpSink->write( buf );
        mMutexSink.unlock();
      }
    }
  }
}

AudioFormat Pipe::getFilterAudioFormat(AudioFormat theUsingFormat)
{
  // TODO : Prepare different format choice example. Note that this is override-able.

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

void Pipe::mutePrimitive(bool bEnableMute, bool bUseZero)
{
  if( mpSink ){
    mpSink->setMuteEnabled( bEnableMute, bUseZero);
  }
  if( mpSource ){
    mpSource->setMuteEnabled( bEnableMute, bUseZero);
  }
}