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
#include "FifoBuffer.hpp"
#include "InterPipeBridge.hpp"
#include "PipeManager.hpp"
#include "MultipleSink.hpp"

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


TEST_F(TestCase_PipeAndFilter, testAddFilters)
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


TEST_F(TestCase_PipeAndFilter, testAttachSourceSinkToPipe)
{
  IPipe* pPipe = new Pipe();

  ISink* pSink = new Sink();
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
  pPipe->addFilterToTail( new FilterIncrement() );
  pPipe->dump();

  pPipe->run();
  EXPECT_TRUE(pPipe->isRunning());

  std::this_thread::sleep_for(std::chrono::microseconds(10000));

  pPipe->stop();
  EXPECT_FALSE(pPipe->isRunning());

  pSink = pPipe->detachSink();
  EXPECT_NE(nullptr, pSink);
  pSink->dump();
  ISource* pSource = pPipe->detachSource();
  EXPECT_NE(nullptr, pSource);
  pPipe->dump();

  pPipe->clearFilers(); // delete filter instances also.

  delete pPipe; pPipe = nullptr;
  delete pSink; pSink = nullptr;
  delete pSource; pSource = nullptr;
}

TEST_F(TestCase_PipeAndFilter, testFifoBuffer)
{
  AudioFormat defaultFormat;
  FifoBuffer fifoBuf( defaultFormat );
  int nSize = 256;
  AudioBuffer readBuf( defaultFormat, nSize );
  AudioBuffer writeBuf( defaultFormat, nSize );

  EXPECT_TRUE( fifoBuf.write( writeBuf ) );
  EXPECT_EQ( fifoBuf.getBufferedSamples(), nSize );

  EXPECT_TRUE( fifoBuf.write( writeBuf ) );
  EXPECT_EQ( fifoBuf.getBufferedSamples(), nSize*2 );

  EXPECT_TRUE( fifoBuf.read( readBuf ) );
  EXPECT_EQ( fifoBuf.getBufferedSamples(), nSize );
  Util::dumpBuffer("readBuf:", readBuf);

  EXPECT_TRUE( fifoBuf.read( readBuf ) );
  EXPECT_EQ( fifoBuf.getBufferedSamples(), 0 );
  Util::dumpBuffer("readBuf:", readBuf);

  std::atomic<bool> bResult = false;
  std::thread thx([&]{ bResult = fifoBuf.read( readBuf );});
  std::this_thread::sleep_for(std::chrono::microseconds(100000));
  EXPECT_TRUE( fifoBuf.write( writeBuf ) );
  thx.join();
  EXPECT_TRUE( bResult );
}

TEST_F(TestCase_PipeAndFilter, testInterPipeBridge)
{
  AudioFormat theUsingFormat = AudioFormat();
  Pipe* pPipe1 = new Pipe();
  Pipe* pPipe2 = new Pipe();
  InterPipeBridge interPipe( theUsingFormat );

  // config pipe1
  EXPECT_EQ( nullptr, pPipe1->attachSource( new Source() ) );
  pPipe1->attachSink( dynamic_cast<ISink*>(&interPipe) );
  pPipe1->addFilterToTail( new FilterIncrement() );
  pPipe1->addFilterToTail( new Filter() );

  // config pipe2
  EXPECT_EQ( nullptr, pPipe2->attachSource( dynamic_cast<ISource*>(&interPipe) ) );
  ISink* pSink = new Sink();
  pSink->setAudioFormat( AudioFormat(
    AudioFormat::ENCODING::PCM_16BIT,
    AudioFormat::SAMPLING_RATE::SAMPLING_RATE_48_KHZ,
    AudioFormat::CHANNEL::CHANNEL_STEREO
    )
  );
  pSink->setPresentation( Sink::PRESENTATION::SPEAKER_STEREO );
  EXPECT_EQ( nullptr, pPipe2->attachSink( pSink ) );
  pPipe2->addFilterToTail( new FilterIncrement() );
  pPipe2->addFilterToTail( new Filter() );

  std::cout << "run" << std::endl;
  // run pipe1 & 2
  pPipe1->run();
  EXPECT_TRUE(pPipe1->isRunning());
  pPipe2->run();
  EXPECT_TRUE(pPipe2->isRunning());
  std::cout << "running" << std::endl;

  std::this_thread::sleep_for(std::chrono::microseconds(10000));

  std::cout << "stop" << std::endl;
  interPipe.unlock();
  // stop pipe1&2
  pPipe1->stop();
  EXPECT_FALSE(pPipe1->isRunning());
  interPipe.unlock();
  pPipe2->stop();
  EXPECT_FALSE(pPipe2->isRunning());
  std::cout << "stopped" << std::endl;

  // clean up
  ISource* pSource = pPipe1->detachSource();
  EXPECT_NE(nullptr, pSource);
  pSink = pPipe2->detachSink();
  EXPECT_NE(nullptr, pSink);

  // check sink
  pSink->dump();

  // clean up
  pPipe1->clearFilers(); // delete filter instances also.
  pPipe2->clearFilers(); // delete filter instances also.

  delete pPipe2; pPipe2 = nullptr;
  delete pPipe1; pPipe1 = nullptr;

  delete pSink; pSink = nullptr;
  delete pSource; pSource = nullptr;
}

TEST_F(TestCase_PipeAndFilter, testPipeManager)
{
  IPipe* pPipe = new PipeManager();

  ISink* pSink = new Sink();
  pSink->setAudioFormat( AudioFormat(
    AudioFormat::ENCODING::PCM_16BIT,
    AudioFormat::SAMPLING_RATE::SAMPLING_RATE_48_KHZ,
    AudioFormat::CHANNEL::CHANNEL_STEREO
    )
  );
  pSink->setPresentation( Sink::PRESENTATION::SPEAKER_STEREO );
  EXPECT_EQ( nullptr, pPipe->attachSink( pSink ) );

  EXPECT_EQ( nullptr, pPipe->attachSource( new Source() ) );

  pPipe->addFilterToTail( new FilterIncrement(IFilter::DEFAULT_WINDOW_SIZE_USEC * 2) );
  pPipe->addFilterToTail( new Filter() );
  pPipe->addFilterToTail( new Filter() );
  pPipe->addFilterToTail( new FilterIncrement() );

  std::cout << "run" << std::endl;
  pPipe->run();
  EXPECT_TRUE(pPipe->isRunning());
  std::cout << "running" << std::endl;

  std::this_thread::sleep_for(std::chrono::microseconds(10000));

  std::cout << "stop" << std::endl;
  pPipe->stop();
  EXPECT_FALSE(pPipe->isRunning());
  std::cout << "stopped" << std::endl;

  pSink = pPipe->detachSink();
  EXPECT_NE(nullptr, pSink);
  pSink->dump();
  ISource* pSource = pPipe->detachSource();
  EXPECT_NE(nullptr, pSource);
  pPipe->dump();

  pPipe->clearFilers(); // delete filter instances also.

  delete pPipe; pPipe = nullptr;
  delete pSink; pSink = nullptr;
  delete pSource; pSource = nullptr;
}

TEST_F(TestCase_PipeAndFilter, testMultipleSink)
{
  MultipleSink* pMultiSink = new MultipleSink();

  ISink* pSink1 = new Sink();
  AudioFormat::ChannelMapper chMap1;
  chMap1.insert( std::make_pair(AudioFormat::CH::L, AudioFormat::CH::L) ); // dst, src
  chMap1.insert( std::make_pair(AudioFormat::CH::R, AudioFormat::CH::L) ); // dst, src
  pMultiSink->addSink( pSink1, chMap1 );
  pMultiSink->dump();

  ISink* pSink2 = new Sink();
  AudioFormat::ChannelMapper chMap2;
  chMap2.insert( std::make_pair(AudioFormat::CH::L, AudioFormat::CH::R) ); // dst, src
  chMap2.insert( std::make_pair(AudioFormat::CH::R, AudioFormat::CH::R) ); // dst, src

  pMultiSink->addSink( pSink2, chMap2 );
  pMultiSink->dump();

  AudioBuffer buf( AudioFormat(), 256 );
  Source source;
  source.read( buf );
  pMultiSink->write( buf );
  pMultiSink->dump();
}


int main(int argc, char **argv)
{
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
