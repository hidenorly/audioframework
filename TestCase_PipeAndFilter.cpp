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

#include <gtest/gtest.h>

#include "TestCase_PipeAndFilter.hpp"

#include "Pipe.hpp"
#include "Filter.hpp"
#include "Source.hpp"
#include "Sink.hpp"
#include "AudioFormat.hpp"
#include "FilterExample.hpp"

#include <iostream>


TestCase_PipeAndFilter::TestCase_PipeAndFilter()
{
}

TestCase_PipeAndFilter::~TestCase_PipeAndFilter()
{
}

void TestCase_PipeAndFilter::SetUp()
{
}

void TestCase_PipeAndFilter::TearDown()
{
}


TEST_F(TestCase_PipeAndFilter, AddFiltersTest)
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

  EXPECT_TRUE(true);
}


TEST_F(TestCase_PipeAndFilter, attachSourceSinkToPipeTest)
{
  Pipe* pPipe = new Pipe();

  Sink* pSink = new Sink();
  pSink->setAudioFormat( AudioFormat(
    AudioFormat::ENCODING::PCM_16BIT, 
    AudioFormat::SAMPLING_RATE::SAMPLING_RATE_48_KHZ,
    AudioFormat::CHANNEL::CHANNEL_STEREO
    )
  );
  pSink->setPresentation( Sink::PRESENTATION::SPEAKER_STEREO );
  EXPECT_EQ( nullptr, pPipe->attachSink( pSink ) );

  EXPECT_EQ( nullptr, pPipe->attachSource( new Source() ) );

  pPipe->addFilterToTail( new FilterIncrement() );
  pPipe->addFilterToTail( new Filter() );
  pPipe->addFilterToTail( new Filter() );
  pPipe->dump();

  pPipe->run();
  EXPECT_TRUE(pPipe->isRunning());
  pPipe->stop();
  EXPECT_FALSE(pPipe->isRunning());

  pSink = pPipe->detachSink();
  EXPECT_NE(nullptr, pSink);
  pSink->dump();
  Source* pSource = pPipe->detachSource();
  EXPECT_NE(nullptr, pSource);
  pPipe->dump();

  pPipe->clearFilers(); // delete filter instances also.

  delete pPipe; pPipe = nullptr;
  delete pSink; pSink = nullptr;
  delete pSource; pSource = nullptr;
}

int main(int argc, char **argv)
{
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
