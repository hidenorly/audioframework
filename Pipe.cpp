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

Pipe::Pipe():mpSink(nullptr), mpSource(nullptr), mbIsRunning(false)
{

}

Pipe::~Pipe()
{
  clearFilers();
  stop();
}

void Pipe::addFilterToHead(Filter* pFilter)
{
  mFilters.insert(mFilters.begin(), pFilter);
}

void Pipe::addFilterToTail(Filter* pFilter)
{
  mFilters.push_back(pFilter);
}

void Pipe::clearFilers(void)
{
  for( auto& pFilter : mFilters ) {
    delete pFilter;
  }
  mFilters.clear();
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

void Pipe::run(void)
{
  mMutexThreads.lock();
  if( !mbIsRunning ){
    mThreads.push_back( std::thread(_execute, this) );
    mbIsRunning = true;
  }
  mMutexThreads.unlock();
}

void Pipe::stop(void)
{
  mMutexThreads.lock();
  if( mbIsRunning ){
    mbIsRunning = false;
    for( auto& aThread : mThreads ){
      if( aThread.joinable() ){
        aThread.join();
      }
    }
    mThreads.clear();
  }
  mMutexThreads.unlock();
}

bool Pipe::isRunning(void)
{
  return mbIsRunning;
}

void Pipe::dump(void)
{
  std::cout << std::endl;
  std::cout << "Source:" << (mpSource ? mpSource->toString() : "") << std::endl;
  std::cout << "Sink:" << (mpSink ? mpSink->toString() : "") << std::endl;

  std::cout << "Filters:" << std::endl;
  for( auto& pFilter : mFilters ) {
    std::cout << pFilter << std::endl;
  }
}

void Pipe::process(void)
{
  if(mpSource && mpSink){
    while(mbIsRunning){
      // tentative code. assume same window size.
      // TODO : create different thread and connect FIFO buffer for different window size situation
      AudioFormat usingAudioFormat = getFilterAudioFormat();

      int samples = (int)( (float)usingAudioFormat.getSamplingRate() * (float)getCommonWindowSizeUsec()/1000000.0f);

      AudioBuffer inBuf(  usingAudioFormat, samples );
      AudioBuffer outBuf( usingAudioFormat, samples );

      mpSource->read( inBuf );

      for( auto& pFilter : mFilters ) {
        pFilter->process( inBuf, outBuf );
        inBuf = outBuf;
      }

      // TODO : May change as directly write to the following buffer from the last filter to avoid the copy.
      mpSink->write( outBuf );
    }
  }
}

void Pipe::_execute(Pipe* pThis)
{
  pThis->process();
}

AudioFormat Pipe::getFilterAudioFormat(void)
{
  // TODO : Prepare different format choice example. Note that this is override-able.
  AudioFormat theUsingFormat; // default AudioFormat is set

  bool bPossibleToUseTheFormat = true;
  for( auto& pFilter : mFilters ) {
    std::vector<AudioFormat> formats = pFilter->getSupportedAudioFormats();
    bool bCompatible = false;
    for( auto& aFormat : formats ){
      bCompatible |= theUsingFormat.equal(aFormat);
    }
    bPossibleToUseTheFormat &= bCompatible;
    if( !bPossibleToUseTheFormat ) break;
  }

  if( !bPossibleToUseTheFormat ){
    throw std::invalid_argument("There is no common audio format in the registered filters");
  }

  return theUsingFormat;
}

int Pipe::getCommonWindowSizeUsec(void)
{
  int result = 1;

  for( auto& pFilter : mFilters ) {
    int windowSizeUsec = pFilter->getRequiredWindowSizeUsec();
    result = std::lcm(result, windowSizeUsec);
  }

  return result;
}