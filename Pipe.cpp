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
#include <iostream>
#include <string>

Pipe::Pipe():mpSink(nullptr), mpSource(nullptr), mbIsRunning(false)
{

}

Pipe::~Pipe()
{
  clearFilers();
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
  for( Filter* pFilter : mFilters ) {
    delete pFilter;
  }
  mFilters.clear();
}

Sink* Pipe::attachSink(Sink* pSink)
{
  Sink* pPrevSink = mpSink;
  mpSink = pSink;

  return pPrevSink;
}

Sink* Pipe::detachSink(void)
{
  Sink* pPrevSink = mpSink;
  mpSink = nullptr;

  return pPrevSink;
}

Source* Pipe::attachSource(Source* pSource)
{
  Source* pPrevSource = mpSource;
  mpSource = pSource;
  return pPrevSource;
}

Source* Pipe::detachSource(void)
{
  Source* pPrevSource = mpSource;
  mpSource = nullptr;

  return pPrevSource;
}

void Pipe::run(void)
{
  if( !mbIsRunning ){
    // TODO : run thread
    mbIsRunning = true;
  }
}

void Pipe::stop(void)
{
  if( mbIsRunning ){
    // TODO : stop thread
    mbIsRunning = false;
  }
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
  for( Filter* pFilter : mFilters ) {
    std::cout << pFilter << std::endl;
  }
}
