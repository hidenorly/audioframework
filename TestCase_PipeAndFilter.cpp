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
#include "Source.hpp"
#include "Sink.hpp"
#include <iostream>



class TestCase_PipeAndFilter
{
public:
  virtual void Setup()
  {

  }

  virtual void TearDown()
  {

  }


  void AddFiltersTest(void)
  {
    Filter* pFilter1 = new Filter();
    Filter* pFilter2 = new Filter();
    Filter* pFilter3 = new Filter();

    Pipe* pPipe = new Pipe();
    pPipe->addFilterToTail(pFilter1);
    pPipe->dump();

    pPipe->addFilterToTail(pFilter2);
    pPipe->dump();

    pPipe->addFilterToHead(pFilter3);
    pPipe->dump();

    pPipe->clearFilers(); // delete filter instances also.
    pPipe->dump();
    delete pPipe; pPipe = nullptr;
  }


  void attachSourceSinkToPipeTest(void)
  {
    Pipe* pPipe = new Pipe();

    pPipe->attachSink( new Sink() );
    pPipe->attachSource( new Source() );
    pPipe->dump();

    pPipe->run();
    std::cout << "Pipe is " << (pPipe->isRunning() ? "running" : "stopped") << std::endl;
    pPipe->stop();
    std::cout << "Pipe is " << (pPipe->isRunning() ? "running" : "stopped") << std::endl;

    Sink* pSink = pPipe->detachSink();
    Source* pSource = pPipe->detachSource();
    pPipe->dump();

    delete pPipe; pPipe = nullptr;
    delete pSink; pSink = nullptr;
    delete pSource; pSource = nullptr;
  }

};


// TODO: replace it with gtest
int main(void)
{
  {
    TestCase_PipeAndFilter testCase;

    testCase.Setup();

    testCase.AddFiltersTest();
    testCase.attachSourceSinkToPipeTest();

    testCase.TearDown();
  }

  return 0;
}