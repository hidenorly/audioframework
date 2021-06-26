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
#include "PipeMultiThread.hpp"
#include "MultipleSink.hpp"
#include "Stream.hpp"
#include "StreamSink.hpp"
#include "StreamSource.hpp"
#include "PipeMixer.hpp"
#include "PipedSink.hpp"
#include "PipedSource.hpp"
#include "Media.hpp"
#include "Decoder.hpp"
#include "Encoder.hpp"
#include "EncodedSink.hpp"
#include "Player.hpp"
#include "ParameterManager.hpp"
#include "StringTokenizer.hpp"
#include "PlugInManager.hpp"
#include "DelayFilter.hpp"
#include "Testability.hpp"
#include "Util.hpp"
#include "ResourceManager.hpp"
#include "Strategy.hpp"
#include "StreamManager.hpp"
#include "PowerManager.hpp"
#include "PowerManagerPrimitive.hpp"
#include "AccousticEchoCancelledSource.hpp"
#include "ReferenceSoundSinkSource.hpp"

#include <iostream>
#include <filesystem>
#include <chrono>
#include <memory>

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
  std::shared_ptr<IFilter> pFilter1( std::make_shared<Filter>() );
  std::shared_ptr<IFilter> pFilter2( std::make_shared<Filter>() );
  std::shared_ptr<IFilter> pFilter3( std::make_shared<Filter>() );
  std::shared_ptr<IFilter> pFilter4( std::make_shared<Filter>() );

  std::unique_ptr<IPipe> pPipe = std::make_unique<Pipe>();
  pPipe->addFilterToTail(pFilter1);
  pPipe->dump();

  pPipe->addFilterToTail(pFilter2);
  pPipe->dump();

  pPipe->addFilterToHead(pFilter3);
  pPipe->dump();

  pPipe->addFilterAfterFilter(pFilter4, pFilter3);
  pPipe->dump();

  pPipe->clearFilters();
  pPipe->dump();

  EXPECT_TRUE(true);
}

TEST_F(TestCase_PipeAndFilter, testAttachSourceSinkToPipe)
{
  // Signal flow : Source -> Pipe(->FilterIncrement->Filter->Filter->FilterIncrement) -> Sink
  std::unique_ptr<IPipe> pPipe = std::make_unique<Pipe>();

  std::shared_ptr<ISink> pSink = std::make_shared<Sink>();
  pSink->setAudioFormat( AudioFormat(
    AudioFormat::ENCODING::PCM_16BIT,
    AudioFormat::SAMPLING_RATE::SAMPLING_RATE_48_KHZ,
    AudioFormat::CHANNEL::CHANNEL_STEREO
    )
  );
  pSink->setPresentation( Sink::PRESENTATION::SPEAKER_STEREO );
  EXPECT_EQ( nullptr, pPipe->attachSink( pSink ) );

  EXPECT_EQ( nullptr, pPipe->attachSource( std::make_shared<Source>() ) );

  pPipe->addFilterToTail( std::make_shared<FilterIncrement>() );
  pPipe->addFilterToTail( std::make_shared<Filter>() );
  pPipe->addFilterToTail( std::make_shared<Filter>() );
  pPipe->addFilterToTail( std::make_shared<FilterIncrement>() );
  pPipe->dump();

  pPipe->run();
  EXPECT_TRUE(pPipe->isRunning());

  std::this_thread::sleep_for(std::chrono::microseconds(1000));

  pPipe->stop();
  EXPECT_FALSE(pPipe->isRunning());

  pSink = pPipe->detachSink();
  EXPECT_NE(nullptr, pSink);
  pSink->dump();
  std::shared_ptr<ISource> pSource = pPipe->detachSource();
  EXPECT_NE(nullptr, pSource);
  pPipe->dump();

  pPipe->clearFilters();
}

TEST_F(TestCase_PipeAndFilter, testInterPipeBridge)
{
  // Signal flow
  //   Source -> Pipe1(->FilterIncrement->Filter->) -> <InterPipeBridge> -> Pipe2(->FilterIncrement->Filter->) -> Sink
  AudioFormat theUsingFormat = AudioFormat();
  std::unique_ptr<IPipe> pPipe1 = std::make_unique<Pipe>();
  std::unique_ptr<IPipe> pPipe2 = std::make_unique<Pipe>();
  std::shared_ptr<InterPipeBridge> interPipe( std::make_shared<InterPipeBridge>(theUsingFormat) );

  // config pipe1
  EXPECT_EQ( nullptr, pPipe1->attachSource( std::make_shared<Source>() ) );
  pPipe1->attachSink( std::dynamic_pointer_cast<ISink>(interPipe) );
  pPipe1->addFilterToTail( std::make_shared<FilterIncrement>() );
  pPipe1->addFilterToTail( std::make_shared<Filter>() );

  // config pipe2
  EXPECT_EQ( nullptr, pPipe2->attachSource( std::dynamic_pointer_cast<ISource>(interPipe) ) );
  std::shared_ptr<ISink> pSink = std::make_shared<Sink>();
  pSink->setAudioFormat( AudioFormat(
    AudioFormat::ENCODING::PCM_16BIT,
    AudioFormat::SAMPLING_RATE::SAMPLING_RATE_48_KHZ,
    AudioFormat::CHANNEL::CHANNEL_STEREO
    )
  );
  pSink->setPresentation( Sink::PRESENTATION::SPEAKER_STEREO );
  EXPECT_EQ( nullptr, pPipe2->attachSink( pSink ) );
  pPipe2->addFilterToTail( std::make_shared<FilterIncrement>() );
  pPipe2->addFilterToTail( std::make_shared<Filter>() );

  std::cout << "run" << std::endl;
  // run pipe1 & 2
  pPipe1->run();
  EXPECT_TRUE(pPipe1->isRunning());
  pPipe2->run();
  EXPECT_TRUE(pPipe2->isRunning());
  std::cout << "running" << std::endl;

  std::this_thread::sleep_for(std::chrono::microseconds(1000));

  std::cout << "stop" << std::endl;
  interPipe->unlock();
  // stop pipe1&2
  pPipe1->stop();
  EXPECT_FALSE(pPipe1->isRunning());
  interPipe->unlock();
  pPipe2->stop();
  EXPECT_FALSE(pPipe2->isRunning());
  std::cout << "stopped" << std::endl;

  // clean up
  std::shared_ptr<ISource> pSource = pPipe1->detachSource();
  EXPECT_NE(nullptr, pSource);
  pSink = pPipe2->detachSink();
  EXPECT_NE(nullptr, pSink);

  // check sink
  pSink->dump();

  // clean up
  pPipe1->clearFilters();
  pPipe2->clearFilters();
}

TEST_F(TestCase_PipeAndFilter, testPipeMultiThread)
{
  // Signal flow
  //   Source -> Pipe(->FilterIncrement->Filter->Filter->FilterIncrement) -> Sink

  std::unique_ptr<IPipe> pPipe = std::make_unique<PipeMultiThread>();

  std::shared_ptr<ISink> pSink = std::make_shared<Sink>();
  pSink->setAudioFormat( AudioFormat(
    AudioFormat::ENCODING::PCM_16BIT,
    AudioFormat::SAMPLING_RATE::SAMPLING_RATE_48_KHZ,
    AudioFormat::CHANNEL::CHANNEL_STEREO
    )
  );
  pSink->setPresentation( Sink::PRESENTATION::SPEAKER_STEREO );
  EXPECT_EQ( nullptr, pPipe->attachSink( pSink ) );

  EXPECT_EQ( nullptr, pPipe->attachSource( std::make_shared<Source>() ) );

  pPipe->addFilterToTail( std::make_shared<FilterIncrement>(IFilter::DEFAULT_WINDOW_SIZE_USEC * 2) );
  pPipe->addFilterToTail( std::make_shared<Filter>() );
  pPipe->addFilterToTail( std::make_shared<Filter>() );
  pPipe->addFilterToTail( std::make_shared<FilterIncrement>() );

  std::cout << "run" << std::endl;
  pPipe->run();
  EXPECT_TRUE(pPipe->isRunning());
  std::cout << "running" << std::endl;

  std::this_thread::sleep_for(std::chrono::microseconds(1000));

  std::cout << "stop" << std::endl;
  pPipe->stop();
  EXPECT_FALSE(pPipe->isRunning());
  std::cout << "stopped" << std::endl;

  pSink = pPipe->detachSink();
  EXPECT_NE(nullptr, pSink);
  pSink->dump();
  std::shared_ptr<ISource> pSource = pPipe->detachSource();
  EXPECT_NE(nullptr, pSource);
  pPipe->dump();

  pPipe->clearFilters();
}

TEST_F(TestCase_PipeAndFilter, testMultipleSink)
{
  class TestSink : public Sink
  {
    int mTestLatency;
  public:
    TestSink(int latencyUsec): mTestLatency(latencyUsec){};
    virtual ~TestSink(){};
    virtual int getLatencyUSec(void){ return mTestLatency; };
  };

  std::shared_ptr<MultipleSink> pMultiSink = std::make_shared<MultipleSink>();

  std::shared_ptr<ISink> pSink1 = std::make_shared<TestSink>( 5*1000 );
  AudioFormat::ChannelMapper chMap1;
  chMap1.insert( std::make_pair(AudioFormat::CH::L, AudioFormat::CH::L) ); // dst, src
  chMap1.insert( std::make_pair(AudioFormat::CH::R, AudioFormat::CH::L) ); // dst, src
  pMultiSink->attachSink( pSink1, chMap1 );

  std::shared_ptr<ISink> pSink2 = std::make_shared<TestSink>( 10*1000 );
  AudioFormat::ChannelMapper chMap2;
  chMap2.insert( std::make_pair(AudioFormat::CH::L, AudioFormat::CH::R) ); // dst, src
  chMap2.insert( std::make_pair(AudioFormat::CH::R, AudioFormat::CH::R) ); // dst, src

  pMultiSink->attachSink( pSink2, chMap2 );

  AudioBuffer buf( AudioFormat(), 256 );
  Source source;
  source.read( buf );
  pMultiSink->write( buf );

  std::cout << "multi sink's latency : " << pMultiSink->getLatencyUSec() << std::endl;
  pMultiSink->dump();

  pMultiSink->clearSinks();
}

TEST_F(TestCase_PipeAndFilter, testMultipleSink_Same)
{
  std::shared_ptr<MultipleSink> pMultiSink = std::make_shared<MultipleSink>();

  std::shared_ptr<ISink> pSink1 = std::make_shared<Sink>();
  AudioFormat::ChannelMapper chMap1 = pSink1->getAudioFormat().getSameChannelMapper();
  pMultiSink->attachSink( pSink1, chMap1 );

  std::shared_ptr<ISink> pSink2 = std::make_shared<Sink>();
  AudioFormat::ChannelMapper chMap2 = pSink2->getAudioFormat().getSameChannelMapper();
  pMultiSink->attachSink( pSink2, chMap2 );

  AudioBuffer buf( AudioFormat(), 256 );
  Source source;
  source.read( buf );
  pMultiSink->write( buf );

  std::cout << "multi sink's latency : " << pMultiSink->getLatencyUSec() << std::endl;
  pMultiSink->dump();

  pMultiSink->clearSinks();
}

TEST_F(TestCase_PipeAndFilter, testMultipleSink_Format)
{
  class TestSink : public Sink
  {
  protected:
    std::vector<AudioFormat> mFormats;
  public:
    TestSink(std::vector<AudioFormat> formats) : mFormats(formats){}
    virtual std::vector<AudioFormat> getSupportedAudioFormats(void)
    {
      return mFormats;
    }
  };

  std::shared_ptr<MultipleSink> pMultiSink = std::make_shared<MultipleSink>();

  std::vector<AudioFormat> formats;
  formats.push_back( AudioFormat() );
  formats.push_back( AudioFormat(AudioFormat::ENCODING::PCM_16BIT, AudioFormat::SAMPLING_RATE::SAMPLING_RATE_96_KHZ) );
  std::shared_ptr<ISink> pSink1 = std::make_shared<TestSink>( formats );
  AudioFormat::ChannelMapper chMap1 = pSink1->getAudioFormat().getSameChannelMapper();
  pMultiSink->attachSink( pSink1, chMap1 );

  formats.push_back( AudioFormat(AudioFormat::ENCODING::PCM_16BIT, AudioFormat::SAMPLING_RATE::SAMPLING_RATE_192_KHZ) );
  std::shared_ptr<ISink> pSink2 = std::make_shared<TestSink>( formats );
  AudioFormat::ChannelMapper chMap2 = pSink2->getAudioFormat().getSameChannelMapper();
  pMultiSink->attachSink( pSink2, chMap2 );

  auto&& sinkFormats = pMultiSink->getSupportedAudioFormats();
  bool bResult1 = true;
  bool bResult2 = true;
  for( auto& aFormat : sinkFormats ){
    std::cout << aFormat.toString() << std::endl;
    bResult1 &= pSink1->isAvailableFormat( aFormat );
    bResult2 &= pSink2->isAvailableFormat( aFormat );
  }
  EXPECT_TRUE( bResult1 );
  EXPECT_TRUE( bResult2 );
}

TEST_F(TestCase_PipeAndFilter, testMultipleSink_FormatOR)
{
  class TestSink : public Sink
  {
  protected:
    std::vector<AudioFormat> mFormats;
  public:
    TestSink(std::vector<AudioFormat> formats) : mFormats(formats){}
    virtual std::vector<AudioFormat> getSupportedAudioFormats(void)
    {
      return mFormats;
    }
  };

  std::shared_ptr<MultipleSink> pMultiSink = std::make_shared<MultipleSink>(AudioFormat(), true);
  EXPECT_TRUE( pMultiSink->getAudioFormatSupportOrModeEnabled() );

  std::vector<AudioFormat> formats;
  formats.push_back( AudioFormat() );
  formats.push_back( AudioFormat(AudioFormat::ENCODING::PCM_16BIT, AudioFormat::SAMPLING_RATE::SAMPLING_RATE_96_KHZ) );
  std::shared_ptr<ISink> pSink1 = std::make_shared<TestSink>( formats );
  AudioFormat::ChannelMapper chMap1 = pSink1->getAudioFormat().getSameChannelMapper();
  pMultiSink->attachSink( pSink1, chMap1 );

  formats.push_back( AudioFormat(AudioFormat::ENCODING::PCM_16BIT, AudioFormat::SAMPLING_RATE::SAMPLING_RATE_192_KHZ) );
  std::shared_ptr<ISink> pSink2 = std::make_shared<TestSink>( formats );
  AudioFormat::ChannelMapper chMap2 = pSink2->getAudioFormat().getSameChannelMapper();
  pMultiSink->attachSink( pSink2, chMap2 );

  std::cout << "result of OR:" << std::endl;
  auto&& sinkFormats = pMultiSink->getSupportedAudioFormats();
  bool bResult1 = true;
  bool bResult2 = true;
  for( auto& aFormat : sinkFormats ){
    std::cout << aFormat.toString() << std::endl;
    bResult1 &= pSink1->isAvailableFormat( aFormat );
    bResult2 &= pSink2->isAvailableFormat( aFormat );
  }
  EXPECT_FALSE( bResult1 );
  EXPECT_TRUE( bResult2 );

  std::cout << "result of AND:" << std::endl;
  pMultiSink->setAudioFormatSupportOrModeEnabled(false);
  bResult1 = true;
  bResult2 = true;
  sinkFormats = pMultiSink->getSupportedAudioFormats();
  for( auto& aFormat : sinkFormats ){
    std::cout << aFormat.toString() << std::endl;
    bResult1 &= pSink1->isAvailableFormat( aFormat );
    bResult2 &= pSink2->isAvailableFormat( aFormat );
  }
  EXPECT_TRUE( bResult1 );
  EXPECT_TRUE( bResult2 );
}


TEST_F(TestCase_PipeAndFilter, testStreamSink)
{
  IStream* pStream = new FileStream("test.bin");
  StreamSink* pSink = new StreamSink(AudioFormat(), pStream);
  AudioBuffer audioBuf(AudioFormat(), 256);
  ByteBuffer buf = audioBuf.getRawBuffer();
  for(int i=0; i<buf.size(); i++){
      buf[i] = i % 256;
  }
  audioBuf.setRawBuffer( buf );
  pSink->write( audioBuf );

  pSink->close();
}

TEST_F(TestCase_PipeAndFilter, testStreamSink_DifferentFormat)
{
  IStream* pStream = new FileStream("test_32b96k.bin");
  StreamSink* pSink = new StreamSink(AudioFormat(AudioFormat::ENCODING::PCM_32BIT, AudioFormat::SAMPLING_RATE::SAMPLING_RATE_96_KHZ, AudioFormat::CHANNEL::CHANNEL_STEREO), pStream);
  AudioBuffer audioBuf(AudioFormat(), 256);
  ByteBuffer buf = audioBuf.getRawBuffer();
  for(int i=0; i<buf.size(); i++){
      buf[i] = i % 256;
  }
  audioBuf.setRawBuffer( buf );
  pSink->write( audioBuf );

  pSink->close();
}

TEST_F(TestCase_PipeAndFilter, testStreamSource)
{
  IStream* pStream = new FileStream("test.bin");
  StreamSource* pSource = new StreamSource(AudioFormat(), pStream);
  AudioBuffer audioBuf(AudioFormat(), 256);
  pSource->read( audioBuf );

  Util::dumpBuffer( audioBuf );

  pSource->close();
}


TEST_F(TestCase_PipeAndFilter, testPipeMixer)
{
  // Signal flow
  //  Source1 -> Pipe1(->FilterIncrement->) -> |PipeMixer | -> Sink
  //  Source2 -> Pipe2(->FilterIncrement->) -> |(mix here)|

  std::unique_ptr<IPipe> pStream1 = std::make_unique<Pipe>();
  std::unique_ptr<IPipe> pStream2 = std::make_unique<Pipe>();

  std::unique_ptr<PipeMixer> pPipeMixer = std::make_unique<PipeMixer>();

  std::shared_ptr<ISource> pSource1 = std::make_shared<Source>();
  pStream1->attachSource( pSource1 );
  pStream1->attachSink( pPipeMixer->allocateSinkAdaptor() );
  pStream1->addFilterToTail( std::make_shared<FilterIncrement>() );

  std::shared_ptr<ISource> pSource2 = std::make_shared<Source>();
  pStream2->attachSource( pSource2 );
  pStream2->attachSink( pPipeMixer->allocateSinkAdaptor() );
  pStream2->addFilterToTail( std::make_shared<FilterIncrement>() );

  std::shared_ptr<ISink> pSink = std::make_shared<Sink>();
  pPipeMixer->attachSink( pSink );

  std::cout << "start" << std::endl;
  pStream1->run();
  pStream2->run();
  pPipeMixer->run();
  std::cout << "started" << std::endl;

  std::this_thread::sleep_for(std::chrono::microseconds(1000));

  std::cout << "stop" << std::endl;
  pPipeMixer->stop();
  pStream1->stop();
  pStream2->stop();
  EXPECT_FALSE(pPipeMixer->isRunning());
  std::cout << "stopped" << std::endl;

  // finalize stream1, the source and the sink
  std::shared_ptr<ISink> pSink1 = pStream1->detachSink();
  EXPECT_NE(nullptr, pSink1);
  pPipeMixer->releaseSinkAdaptor( pSink1 );
  pSource1 = pStream1->detachSource();
  EXPECT_NE(nullptr, pSource1);
  pStream1->clearFilters();

  // finalize stream2, the source and the sink
  std::shared_ptr<ISink> pSink2 = pStream2->detachSink();
  EXPECT_NE(nullptr, pSink2);
  pPipeMixer->releaseSinkAdaptor( pSink2 );
  pSource2 = pStream2->detachSource();
  EXPECT_NE(nullptr, pSource2);
  pStream2->clearFilters();

  // finalize pipemixer
  pSink = pPipeMixer->detachSink();
  EXPECT_NE(nullptr, pSink);
  pSink->dump();
}

TEST_F(TestCase_PipeAndFilter, testPipedSink)
{
  // Signal flow : Source -> Pipe(->FilterIncrement->) -> PipedSink(->FilterIncrement->) -> ActualSink

  std::shared_ptr<ISource> pSource = std::make_shared<Source>();
  std::shared_ptr<ISink> pActualSink = std::make_shared<Sink>();
  pActualSink->setVolume(50.0f);

  std::shared_ptr<PipedSink> pPipedSink = std::make_shared<PipedSink>();
  pPipedSink->attachSink( pActualSink );
  pPipedSink->addFilterToTail( std::make_shared<FilterIncrement>() );

  std::unique_ptr<IPipe> pPipe = std::make_unique<Pipe>();
  pPipe->addFilterToTail( std::make_shared<FilterIncrement>() );
  pPipe->attachSource( pSource );
  pPipe->attachSink( pPipedSink );

  pPipe->run();
  pPipedSink->run();

  std::this_thread::sleep_for(std::chrono::microseconds(500));

  std::cout << "source latency: " << std::dec << pSource->getLatencyUSec() << std::endl;
  std::cout << "pipe latency: " << pPipe->getLatencyUSec() << std::endl;
  std::cout << "actual sink latency: " << pActualSink->getLatencyUSec() << std::endl;
  std::cout << "piped sink latency: " << pPipedSink->getLatencyUSec() << std::endl;

  std::cout << "source PTS: " << pSource->getSourcePts();
  std::cout << " / actual sink PTS: " << pActualSink->getSinkPts() << std::endl;
  std::this_thread::sleep_for(std::chrono::microseconds(500));
  std::cout << "source PTS: " << pSource->getSourcePts();
  std::cout << " / actual sink PTS: " << pActualSink->getSinkPts() << std::endl;

  pPipedSink->stop();
  pPipe->stop();
  pPipedSink->dump();

  std::shared_ptr<ISink> pActualSink2 = pPipedSink->detachSink();
  EXPECT_EQ( pActualSink, pActualSink2 );
  pPipedSink->clearFilters();

  std::shared_ptr<ISink> pPipedSink2 = pPipe->detachSink();
  EXPECT_EQ( pPipedSink, pPipedSink2 );
  std::shared_ptr<ISource> pSource2 = pPipe->detachSource();
  EXPECT_EQ( pSource, pSource2 );
  pPipe->clearFilters();
}

TEST_F(TestCase_PipeAndFilter, testPipedSource)
{
  // Signal flow : Actual Source -> PipedSource(->FilterIncrement->) -> Pipe(->FilterIncrement->) -> Sink
  std::shared_ptr<ISink> pSink = std::make_shared<Sink>();

  std::shared_ptr<ISource> pActualSource = std::make_shared<Source>();
  std::shared_ptr<PipedSource> pPipedSource = std::make_shared<PipedSource>();
  pPipedSource->attachSource( pActualSource );
  pPipedSource->addFilterToTail( std::make_shared<FilterIncrement>() );

  std::unique_ptr<IPipe> pPipe = std::make_unique<Pipe>();
  pPipe->addFilterToTail( std::make_shared<FilterIncrement>() );
  pPipe->attachSource( pPipedSource );
  pPipe->attachSink( pSink );

  pPipedSource->run();
  pPipe->run();

  std::this_thread::sleep_for(std::chrono::microseconds(500));

  pPipe->stop();
  pPipedSource->stop();
  pSink->dump();

  std::shared_ptr<ISource> pDetachedActualSource = pPipedSource->detachSource();
  EXPECT_EQ( pDetachedActualSource, pActualSource );
  pPipedSource->clearFilters();

  std::shared_ptr<ISource> pDetachedSource = pPipe->detachSource();
  EXPECT_EQ( pDetachedSource, pPipedSource );

  std::shared_ptr<ISink> pDetachedSink = pPipe->detachSink();
  EXPECT_EQ( pDetachedSink, pSink );

  pPipe->clearFilters();
}

TEST_F(TestCase_PipeAndFilter, testDecoder)
{
  // Signal flow : Source -> Decoder -> <SourceAdaptor> -> Pipe(->Filter->) -> Sink

  std::vector<MediaParam> params;
  MediaParam param1("testKey1", "abc"); params.push_back(param1);
  MediaParam param2("testKey2", 3840);  params.push_back(param2);
  MediaParam param3("testKey3", true);  params.push_back(param3);
  std::shared_ptr<IDecoder> pDecoder = std::make_shared<NullDecoder>();
  pDecoder->configure(params);

  std::shared_ptr<ISource> pSource = std::make_shared<Source>();
  pDecoder->attachSource( pSource );
  std::shared_ptr<ISource> pSourceAdaptor = pDecoder->allocateSourceAdaptor();
  std::shared_ptr<ISink> pSink = std::make_shared<Sink>();

  std::unique_ptr<IPipe> pPipe = std::make_unique<Pipe>();
  pPipe->attachSource( pSourceAdaptor );
  pPipe->attachSink( pSink );
  pPipe->addFilterToTail( std::make_shared<FilterIncrement>() );

  pDecoder->run();
  pPipe->run();

  std::this_thread::sleep_for(std::chrono::microseconds(1000));

  std::cout << "source PTS: " << pSource->getSourcePts();
  std::cout << " / sink PTS: " << pSink->getSinkPts() << std::endl;
  std::this_thread::sleep_for(std::chrono::microseconds(1000));
  std::cout << "source PTS: " << pSource->getSourcePts();
  std::cout << " / sink PTS: " << pSink->getSinkPts() << std::endl;

  std::cout << "Pipe::stop" << std::endl;
  pPipe->stop();
  std::cout << "Decoder::stop" << std::endl;
  pDecoder->stop();
  std::cout << "Decoder::stopped" << std::endl;

  pSink->dump();

  std::shared_ptr<ISource> pDetachedSource = pPipe->attachSource( pSourceAdaptor );
  EXPECT_NE( pDetachedSource, nullptr );
  EXPECT_EQ( pDetachedSource, pSourceAdaptor );
  pDecoder->releaseSourceAdaptor( pDetachedSource ); pSourceAdaptor = pDetachedSource = nullptr;

  std::shared_ptr<ISink> pDetachedSink = pPipe->detachSink();
  EXPECT_NE( pDetachedSink, nullptr );
  EXPECT_EQ( pDetachedSink, pSink );
  pSink = nullptr; pDetachedSink = nullptr;

  pPipe->clearFilters();

  std::shared_ptr<ISource> pDetachedDecoderSource = pDecoder->detachSource();
  EXPECT_NE( pDetachedDecoderSource, nullptr );
  EXPECT_EQ( pDetachedDecoderSource, pSource );
}

TEST_F(TestCase_PipeAndFilter, testPlayer)
{
  // Signal flow : Source -> Player(decoder) -> <SourceAdaptor> -> Pipe(->Filter->) -> Sink
  // player(source, decoder) --via source adaptor -> pipe(filter) -> sink
  std::shared_ptr<ISource> pSource = std::make_shared<Source>();
  std::shared_ptr<ISink> pSink = std::make_shared<Sink>();

  std::shared_ptr<IDecoder> pDecoder = std::make_shared<NullDecoder>();
  std::unique_ptr<IPlayer> pPlayer = std::make_unique<Player>();
  std::shared_ptr<ISource> pSourceAdaptor = pPlayer->prepare( pSource, pDecoder ); // handover the source and decoder
  EXPECT_EQ( pPlayer->isReady(), true );

  std::unique_ptr<IPipe> pPipe = std::make_unique<Pipe>();
  pPipe->attachSource( pSourceAdaptor );
  pPipe->attachSink( pSink );
  pPipe->addFilterToTail( std::make_shared<FilterIncrement>() );

  pPlayer->play();
  pPipe->run();
  std::cout << "source PTS: " << pSource->getSourcePts();
  std::cout << " / sink PTS: " << pSink->getSinkPts() << std::endl;
  std::this_thread::sleep_for(std::chrono::microseconds(1000));
  std::cout << "source PTS: " << pSource->getSourcePts();
  std::cout << " / sink PTS: " << pSink->getSinkPts() << std::endl;

  std::cout << "pause" << std::endl;
  pPlayer->pause();
  std::this_thread::sleep_for(std::chrono::microseconds(1000));
  std::cout << "resume" << std::endl;
  pPlayer->resume();
  std::this_thread::sleep_for(std::chrono::microseconds(1000));

  std::cout << "stop" << std::endl;
  pPipe->stop();
  pPlayer->stop();
  std::cout << "stopped" << std::endl;

  pSink->dump();

  // detach source (=player's source adaptor) from pipe
  std::shared_ptr<ISource> pDetachedSourceAdaptor = pPipe->detachSource();
  EXPECT_NE( pDetachedSourceAdaptor, nullptr );
  EXPECT_EQ( pDetachedSourceAdaptor, pSourceAdaptor );

  // release the Detached Source Adaptor & detach the decoder(=player)'s source from player. note that player terminate will delete the decoder instance
  std::shared_ptr<ISource> pDetachedPlayerSource = pPlayer->terminate( pDetachedSourceAdaptor );
  EXPECT_NE( pDetachedPlayerSource, nullptr );
  EXPECT_EQ( pDetachedPlayerSource, pSource );

  // detach sink from pipe
  std::shared_ptr<ISink> pDetachedSink = pPipe->detachSink();
  EXPECT_NE( pDetachedSink, nullptr );
  EXPECT_EQ( pDetachedSink, pSink );

  // clear (delete) the filters
  pPipe->clearFilters();
}

TEST_F(TestCase_PipeAndFilter, testEncoder)
{
  // Signal flow : Source -> Pipe(->Filter->) -> <SinkAdaptor> -> Encoder -> EncodedSink
  std::vector<MediaParam> params;
  MediaParam param1("testKey1", "abc"); params.push_back(param1);
  MediaParam param2("testKey2", 3840);  params.push_back(param2);
  MediaParam param3("testKey3", true);  params.push_back(param3);
  std::unique_ptr<IEncoder> pEncoder = std::make_unique<NullEncoder>();
  pEncoder->configure(params);

  std::shared_ptr<ISource> pSource = std::make_shared<Source>();
  std::shared_ptr<ISink> pSink = std::make_shared<EncodedSink>();
  std::unique_ptr<IPipe> pPipe = std::make_unique<Pipe>();
  pPipe->attachSource( pSource );
  pPipe->addFilterToTail( std::make_shared<FilterIncrement>() );
  std::shared_ptr<ISink> pSinkAdaptor = pEncoder->allocateSinkAdaptor();
  pPipe->attachSink( pSinkAdaptor );
  pEncoder->attachSink( pSink );

  pEncoder->run();
  pPipe->run();

  std::this_thread::sleep_for(std::chrono::microseconds(1000));

  std::cout << "source PTS: " << pSource->getSourcePts();
  std::cout << " / sink PTS: " << pSink->getSinkPts() << std::endl;
  std::this_thread::sleep_for(std::chrono::microseconds(1000));
  std::cout << "source PTS: " << pSource->getSourcePts();
  std::cout << " / sink PTS: " << pSink->getSinkPts() << std::endl;

  std::cout << "Encoder::stop" << std::endl;
  pEncoder->stop();
  std::cout << "Encoder::stopped" << std::endl;
  std::cout << "Pipe::stop" << std::endl;
  pPipe->stop();
  std::cout << "Pipe::stopped" << std::endl;

  pSink->dump();

  // detach sink from encoder
  std::shared_ptr<ISink> pDetachedSink = pEncoder->detachSink();
  EXPECT_NE( pDetachedSink, nullptr );
  EXPECT_EQ( pDetachedSink, pSink );
  pDetachedSink = pSink = nullptr;

  // detach sink adaptor from pipe (=release encoder's sink adaptor)
  std::shared_ptr<ISink> pDetachedSinkEnc = pPipe->detachSink();
  EXPECT_NE( pDetachedSinkEnc, nullptr );
  EXPECT_EQ( pDetachedSinkEnc, pSinkAdaptor );
  pEncoder->releaseSinkAdaptor( pSinkAdaptor ); pSinkAdaptor = pDetachedSinkEnc = nullptr;

  // detach source from pipe
  std::shared_ptr<ISource> pDetachedSource = pPipe->detachSource();
  EXPECT_NE( pDetachedSource, nullptr );
  EXPECT_EQ( pDetachedSource, pSource );
  pDetachedSource = pSource = nullptr;

  // clear filter in the pipe
  pPipe->clearFilters();
}


TEST_F(TestCase_PipeAndFilter, testDelayFilter)
{
  // Signal flow : Source -> Pipe(->Delay->PerChannelDelayFilter->) -> Sink
  std::shared_ptr<ISource> pSource = std::make_shared<Source>();
  std::shared_ptr<ISink> pSink = std::make_shared<Sink>();
  std::unique_ptr<IPipe> pPipe = std::make_unique<Pipe>();

  pPipe->attachSource(pSource);
  PerChannelDelayFilter::ChannelDelay channelDelay;
  channelDelay[AudioFormat::CH::L] = 0;
  channelDelay[AudioFormat::CH::R] = 20*120; // 20us = 1 sample delay @ 48KHz

  pPipe->addFilterToTail( std::make_shared<DelayFilter>( AudioFormat(), 20*120 ) );
  pPipe->addFilterToTail( std::make_shared<PerChannelDelayFilter>( AudioFormat(), channelDelay ) );
  pPipe->attachSink(pSink);

  pPipe->run();
  std::this_thread::sleep_for(std::chrono::microseconds(1000));

  pPipe->stop();

  EXPECT_EQ( pSink, pPipe->detachSink());
  EXPECT_EQ( pSource, pPipe->detachSource());
  pSink->dump();
  pPipe->clearFilters();
}


TEST_F(TestCase_PipeAndFilter, testDynamicSignalFlow_AddNewPipeToPipeMixer)
{
  // Signal flow: adding filterduring pipe is running
  //   step1: Source -> Pipe(->FilterIncrement->) -> SinkAdaptor -> [PipeMixer ] -> Sink
  //
  //   step2: Source -> Pipe(->FilterIncrement->) -> SinkAdaptor -> [PipeMixer ] -> Sink
  //          Source -> Pipe(->FilterIncrement->) -> SinkAdaptor -> [(mix here)]
  std::unique_ptr<PipeMixer> pPipeMixer = std::make_unique<PipeMixer>();
  std::shared_ptr<ISink> pSink = std::make_shared<Sink>();
  pPipeMixer->attachSink( pSink );

  std::unique_ptr<IPipe> pStream1 = std::make_unique<Pipe>();
  std::shared_ptr<ISource> pSource1 = std::make_shared<Source>();
  pStream1->attachSource( pSource1 );
  pStream1->attachSink( pPipeMixer->allocateSinkAdaptor() );
  pStream1->addFilterToTail( std::make_shared<FilterIncrement>() );

  std::cout << "start:mixer & stream1" << std::endl;
  pStream1->run();
  pPipeMixer->run();
  std::cout << "started" << std::endl;
  std::this_thread::sleep_for(std::chrono::microseconds(1000));

  std::cout << "add:stream2 during stream1 is running" << std::endl;
  std::unique_ptr<IPipe> pStream2 = std::make_unique<Pipe>();
  std::shared_ptr<ISource> pSource2 = std::make_shared<Source>();
  pStream2->attachSource( pSource2 );
  std::shared_ptr<ISink> pSinkAdaptor2 = pPipeMixer->allocateSinkAdaptor();
  pStream2->attachSink( pSinkAdaptor2 );
  std::cout << "added:stream2 during stream1 is running" << std::endl;
  pStream2->addFilterToTail( std::make_shared<FilterIncrement>() );
  std::cout << "start:stream2 during stream1 is running" << std::endl;
  pStream2->run();
  std::cout << "started:stream2 during stream1 is running" << std::endl;
  std::this_thread::sleep_for(std::chrono::microseconds(1000));

  // SHOULD DETACH SINK ADAPTOR FROM PIPEMIXER FIRST WITHOUT DELETE WHILE THE PIPE IS RUNNING.
  std::cout << "release detached sink adaptor of stream2 during stream1 is running" << std::endl;
  pPipeMixer->releaseSinkAdaptor( pSinkAdaptor2 );
  std::cout << "released detached sink adaptor of stream2 during stream1 is running" << std::endl;

  std::cout << "stop:stream2 during stream1 is running" << std::endl;
  pStream2->stop();
  std::cout << "stopped:stream2 during stream1 is running" << std::endl;
  // finalize stream2, the source and the sink

  std::cout << "detach:sink adaptor from stream2 during stream1 is running" << std::endl;
  std::shared_ptr<ISink> pSink2 = pStream2->detachSink();
  EXPECT_EQ(pSinkAdaptor2, pSink2);
  pSink2 = nullptr; pSinkAdaptor2 = nullptr;

  std::cout << "detach:source of stream2 during stream1 is running" << std::endl;
  pSource2 = pStream2->detachSource();
  EXPECT_NE(nullptr, pSource2);
  std::cout << "detached:source of stream2 during stream1 is running" << std::endl;
  pSource2 = nullptr;
  std::cout << "clear:filters of stream2 during stream1 is running" << std::endl;
  pStream2->clearFilters();
  std::cout << "cleared:filters of stream2 during stream1 is running" << std::endl;
  std::cout << "wait is running" << std::endl;
  std::this_thread::sleep_for(std::chrono::microseconds(1000));
  std::cout << "waited during stream1 is running" << std::endl;

  std::cout << "stop all" << std::endl;

  pPipeMixer->stop();
  pStream1->stop();
  EXPECT_FALSE(pPipeMixer->isRunning());
  std::cout << "stopped" << std::endl;

  // finalize stream1, the source and the sink
  std::shared_ptr<ISink> pSink1 = pStream1->detachSink();
  EXPECT_NE(nullptr, pSink1);
  pPipeMixer->releaseSinkAdaptor( pSink1 );
  pSource1 = pStream1->detachSource();
  EXPECT_NE(nullptr, pSource1);
  pSource1 = nullptr;
  pStream1->clearFilters();

  // finalize pipemixer
  pSink = pPipeMixer->detachSink();
  EXPECT_NE(nullptr, pSink);
  pSink->dump();
}

TEST_F(TestCase_PipeAndFilter, testDynamicSignalFlow_AddNewFilter)
{
  // Signal flow: adding filterduring pipe is running
  //   step1: Source -> Pipe(->FilterIncrement->) -> Sink
  //   step2: Source -> Pipe(->FilterIncrement->FilterIncrement->) -> Sink
  //   step3: Source -> Pipe(->FilterIncrement->) -> Sink
  std::unique_ptr<IPipe> pStream = std::make_unique<Pipe>();
  std::shared_ptr<ISource> pSource = std::make_shared<Source>();
  pStream->attachSource( pSource );
  std::shared_ptr<ISink> pSink = std::make_shared<Sink>();
  pStream->attachSink( pSink );
  pStream->addFilterToTail( std::make_shared<FilterIncrement>() );

  std::cout << "start" << std::endl;
  pStream->run();
  std::cout << "started" << std::endl;
  std::this_thread::sleep_for(std::chrono::microseconds(1000));

  std::cout << "add:filter during stream is running" << std::endl;
  std::shared_ptr<IFilter> pFilter = std::make_shared<FilterIncrement>();
  pStream->addFilterToTail( pFilter );
  std::cout << "added:filter during stream is running" << std::endl;
  std::this_thread::sleep_for(std::chrono::microseconds(1000));
  std::cout << "remove:filter during stream is running" << std::endl;
  pStream->removeFilter( pFilter );
  std::cout << "removed:filter during stream is running" << std::endl;
  std::this_thread::sleep_for(std::chrono::microseconds(1000));

  std::cout << "stop" << std::endl;
  pStream->stop();
  std::cout << "stopped" << std::endl;

  std::shared_ptr<ISink> pDetachedSink = pStream->detachSink();
  EXPECT_EQ(pSink, pDetachedSink);
  pSink->dump();
  pSink = pDetachedSink = nullptr;

  std::shared_ptr<ISource> pDetachedSource = pStream->detachSource();
  EXPECT_EQ(pSource, pDetachedSource);
  pSource = pDetachedSource = nullptr;

  pStream->clearFilters();
}

TEST_F(TestCase_PipeAndFilter, testDynamicSignalFlow_AddNewFilter_PipeMultiThread)
{
  // Signal flow: adding filterduring pipe is running
  //   step1: Source -> PipeMultiThread(->FilterIncrement->) -> Sink
  //   step2: Source -> PipeMultiThread(->FilterIncrement->FilterIncrement->) -> Sink
  //   step3: Source -> PipeMultiThread(->FilterIncrement->) -> Sink
  std::unique_ptr<IPipe> pStream = std::make_unique<PipeMultiThread>();
  std::shared_ptr<ISource> pSource = std::make_shared<Source>();
  pStream->attachSource( pSource );
  std::shared_ptr<ISink> pSink = std::make_shared<Sink>();
  pStream->attachSink( pSink );
  pStream->addFilterToTail( std::make_shared<FilterIncrement>() );

  std::cout << "start" << std::endl;
  pStream->run();
  std::cout << "started" << std::endl;
  std::this_thread::sleep_for(std::chrono::microseconds(1000));

  std::cout << "add:filter during stream is running" << std::endl;
  std::shared_ptr<IFilter> pFilter = std::make_shared<FilterIncrement>();
  pStream->addFilterToTail( pFilter );
  std::cout << "added:filter during stream is running" << std::endl;
  std::this_thread::sleep_for(std::chrono::microseconds(1000));
  std::cout << "remove:filter during stream is running" << std::endl;
  pStream->removeFilter( pFilter );
  std::cout << "removed:filter during stream is running" << std::endl;
  std::this_thread::sleep_for(std::chrono::microseconds(1000));

  std::cout << "stop" << std::endl;
  pStream->stop();
  std::cout << "stopped" << std::endl;

  std::shared_ptr<ISink> pDetachedSink = pStream->detachSink();
  EXPECT_EQ(pSink, pDetachedSink);
  pSink->dump();

  std::shared_ptr<ISource> pDetachedSource = pStream->detachSource();
  EXPECT_EQ(pSource, pDetachedSource);

  pStream->clearFilters();
}

TEST_F(TestCase_PipeAndFilter, testDynamicSignalFlow_AddNewSinkToPipe)
{
  // Signal flow: switch sink during pipe is running
  //   before: Source -> Pipe(->FilterIncrement->) -> Sink1
  //   after:  Source -> Pipe(->FilterIncrement->) -> Sink2
  std::unique_ptr<IPipe> pPipe = std::make_unique<Pipe>();

  std::shared_ptr<ISource> pSource = std::make_shared<Source>();
  pPipe->attachSource( pSource );

  std::shared_ptr<ISink> pSink1 = std::make_shared<Sink>();
  pPipe->attachSink( pSink1 );

  pPipe->addFilterToTail( std::make_shared<FilterIncrement>() );

  std::cout << "start" << std::endl;
  pPipe->run();
  std::this_thread::sleep_for(std::chrono::microseconds(1000));
  std::cout << "started" << std::endl;

  std::shared_ptr<ISink> pSink2 = std::make_shared<Sink>();
  std::cout << "attach new Sink to pipe during the pipe is running" << std::endl;
  std::shared_ptr<ISink> pDetachedSink1 = pPipe->attachSink( pSink2 );
  std::cout << "attached new Sink to pipe during the pipe is running" << std::endl;
  EXPECT_EQ( pDetachedSink1, pSink1 );
  pDetachedSink1->dump();
  std::this_thread::sleep_for(std::chrono::microseconds(1000));

  std::cout << "stop" << std::endl;
  pPipe->stop();
  std::cout << "stopped" << std::endl;

  std::shared_ptr<ISink> pDetachedSink2 = pPipe->detachSink();
  EXPECT_EQ( pDetachedSink2, pSink2 );
  pDetachedSink2->dump();

  std::shared_ptr<ISource> pDetachedSource = pPipe->detachSource();
  EXPECT_EQ( pDetachedSource, pSource );
}

TEST_F(TestCase_PipeAndFilter, testDynamicSignalFlow_AddNewSinkToPipe_PipeMultiThread)
{
  // Signal flow: switch sink during pipe is running
  //   before: Source -> PipeMultiThread(->FilterIncrement->) -> Sink1
  //   after:  Source -> PipeMultiThread(->FilterIncrement->) -> Sink2
  std::unique_ptr<IPipe> pPipe = std::make_unique<PipeMultiThread>();

  std::shared_ptr<ISource> pSource = std::make_shared<Source>();
  pPipe->attachSource( pSource );

  std::shared_ptr<ISink> pSink1 = std::make_shared<Sink>();
  pPipe->attachSink( pSink1 );

  pPipe->addFilterToTail( std::make_shared<FilterIncrement>() );

  std::cout << "start" << std::endl;
  pPipe->run();
  std::this_thread::sleep_for(std::chrono::microseconds(1000));
  std::cout << "started" << std::endl;

  std::shared_ptr<ISink> pSink2 = std::make_shared<Sink>();
  std::cout << "attach new Sink to pipe during the pipe is running" << std::endl;
  std::shared_ptr<ISink> pDetachedSink1 = pPipe->attachSink( pSink2 );
  std::cout << "attached new Sink to pipe during the pipe is running" << std::endl;
  EXPECT_EQ( pDetachedSink1, pSink1 );
  pDetachedSink1->dump();
  std::this_thread::sleep_for(std::chrono::microseconds(1000));

  std::cout << "stop" << std::endl;
  pPipe->stop();
  std::cout << "stopped" << std::endl;

  std::shared_ptr<ISink> pDetachedSink2 = pPipe->detachSink();
  EXPECT_EQ( pDetachedSink2, pSink2 );
  pDetachedSink2->dump();

  std::shared_ptr<ISource> pDetachedSource = pPipe->detachSource();
  EXPECT_EQ( pDetachedSource, pSource );
}


TEST_F(TestCase_PipeAndFilter, testDynamicSignalFlow_AddNewSourceToPipe)
{
  // Signal flow
  //   before: Source1 -> Pipe(->FilterIncrement->) -> Sink
  //   after:  Source2 -> Pipe(->FilterIncrement->) -> Sink
  std::unique_ptr<IPipe> pPipe = std::make_unique<Pipe>();

  std::shared_ptr<ISource> pSource1 = std::make_shared<Source>();
  pPipe->attachSource( pSource1 );

  std::shared_ptr<ISink> pSink = std::make_shared<Sink>();
  pPipe->attachSink( pSink );

  pPipe->addFilterToTail( std::make_shared<FilterIncrement>() );

  std::cout << "start" << std::endl;
  pPipe->run();
  std::this_thread::sleep_for(std::chrono::microseconds(1000));
  std::cout << "started" << std::endl;

  std::shared_ptr<ISource> pSource2 = std::make_shared<Source>();
  std::cout << "attach new Source to pipe during the pipe is running" << std::endl;
  std::shared_ptr<ISource> pDetachedSource1 = pPipe->attachSource( pSource2 );
  std::cout << "attached new Source to pipe during the pipe is running" << std::endl;
  EXPECT_EQ( pDetachedSource1, pSource1 );
  std::this_thread::sleep_for(std::chrono::microseconds(1000));

  std::cout << "stop" << std::endl;
  pPipe->stop();
  std::cout << "stopped" << std::endl;

  std::shared_ptr<ISink> pDetachedSink = pPipe->detachSink();
  EXPECT_EQ( pDetachedSink, pSink );
  pDetachedSink->dump();

  std::shared_ptr<ISource> pDetachedSource2 = pPipe->detachSource();
  EXPECT_EQ( pDetachedSource2, pSource2 );
}

TEST_F(TestCase_PipeAndFilter, testDynamicSignalFlow_AddNewSourceToPipe_PipeMultiThread)
{
  // Signal flow
  //   before: Source1 -> PipeMultiThread(->FilterIncrement->) -> Sink
  //   after:  Source2 -> PipeMultiThread(->FilterIncrement->) -> Sink
  std::unique_ptr<IPipe> pPipe = std::make_unique<PipeMultiThread>();

  std::shared_ptr<ISource> pSource1 = std::make_shared<Source>();
  pPipe->attachSource( pSource1 );

  std::shared_ptr<ISink> pSink = std::make_shared<Sink>();
  pPipe->attachSink( pSink );

  pPipe->addFilterToTail( std::make_shared<FilterIncrement>() );

  std::cout << "start" << std::endl;
  pPipe->run();
  std::this_thread::sleep_for(std::chrono::microseconds(1000));
  std::cout << "started" << std::endl;

  std::shared_ptr<ISource> pSource2 = std::make_shared<Source>();
  std::cout << "attach new Source to pipe during the pipe is running" << std::endl;
  std::shared_ptr<ISource> pDetachedSource1 = pPipe->attachSource( pSource2 );
  std::cout << "attached new Source to pipe during the pipe is running" << std::endl;
  EXPECT_EQ( pDetachedSource1, pSource1 );
  std::this_thread::sleep_for(std::chrono::microseconds(1000));

  std::cout << "stop" << std::endl;
  pPipe->stop();
  std::cout << "stopped" << std::endl;

  std::shared_ptr<ISink> pDetachedSink = pPipe->detachSink();
  EXPECT_EQ( pDetachedSink, pSink );
  pDetachedSink->dump();

  std::shared_ptr<ISource> pDetachedSource2 = pPipe->detachSource();
  EXPECT_EQ( pDetachedSource2, pSource2 );
}

TEST_F(TestCase_PipeAndFilter, testSinkMute)
{
  // Signal flow
  //   Source -> Pipe(->FilterIncrement->) -> Sink
  //   setMuteEnabled(true,true)----------------^  # 2nd arg (=bUseZero)=true means output zero
  std::unique_ptr<IPipe> pPipe = std::make_unique<Pipe>();
  std::shared_ptr<ISource> pSource = std::make_shared<Source>();
  pPipe->attachSource( pSource );
  std::shared_ptr<ISink> pSink = std::make_shared<Sink>();
  pPipe->attachSink( pSink );
  pPipe->addFilterToTail( std::make_shared<FilterIncrement>() );
  pSink->setMuteEnabled( true, true );
  pPipe->run();
  std::this_thread::sleep_for(std::chrono::microseconds(1000));
  pSink->setMuteEnabled( false );
  std::this_thread::sleep_for(std::chrono::microseconds(1000));
  pPipe->stop();
  pSink->dump();
  pPipe->clearFilters();
}

TEST_F(TestCase_PipeAndFilter, testSourceMute)
{
  // Signal flow
  //   Source -> Pipe(->FilterIncrement->) -> Sink
  //      ^----- setMuteEnabled(true,true) # 2nd arg (=bUseZero)=true means output zero
  std::unique_ptr<IPipe> pPipe = std::make_unique<Pipe>();
  std::shared_ptr<ISource> pSource = std::make_shared<Source>();
  pPipe->attachSource( pSource );
  std::shared_ptr<ISink> pSink = std::make_shared<Sink>();
  pPipe->attachSink( pSink );
  pPipe->addFilterToTail( std::make_shared<FilterIncrement>() );
  pSource->setMuteEnabled( true, true );
  pPipe->run();
  std::this_thread::sleep_for(std::chrono::microseconds(1000));
  pSource->setMuteEnabled( false );
  std::this_thread::sleep_for(std::chrono::microseconds(1000));
  pPipe->stop();
  pSink->dump();
  pPipe->clearFilters();
}

TEST_F(TestCase_PipeAndFilter, testPipeMute)
{
  // Signal flow
  //   Source -> Pipe(->FilterIncrement->) -> Sink
  //      ^----- setMuteEnabled(true,true) # 2nd arg (=bUseZero)=true means output zero
  std::unique_ptr<IPipe> pPipe = std::make_unique<Pipe>();
  std::shared_ptr<ISource> pSource = std::make_shared<Source>();
  std::shared_ptr<ISink> pSink = std::make_shared<Sink>();
  pPipe->attachSource( pSource );
  pPipe->attachSink( pSink );
  pPipe->addFilterToTail( std::make_shared<FilterIncrement>() );
  pPipe->setMuteEnabled( true, true );
  EXPECT_TRUE( pPipe->getMuteEnabled() );
  EXPECT_TRUE( pSink->getMuteEnabled() );
  EXPECT_TRUE( pSource->getMuteEnabled() );
  pPipe->run();
  std::this_thread::sleep_for(std::chrono::microseconds(1000));
  pPipe->setMuteEnabled( false );
  EXPECT_FALSE( pPipe->getMuteEnabled() );
  EXPECT_FALSE( pSink->getMuteEnabled() );
  EXPECT_FALSE( pSource->getMuteEnabled() );
  std::this_thread::sleep_for(std::chrono::microseconds(1000));
  pPipe->stop();
  pSink->dump();
  pPipe->clearFilters();
}

class TestSink : public Sink
{
  int mTestLatency;
public:
  TestSink(int latencyUsec): Sink(), mTestLatency(latencyUsec){};
  virtual ~TestSink(){};
  virtual int getLatencyUSec(void){ return mTestLatency; };
};

class TestSource : public Source
{
  int mTestLatency;
public:
  TestSource(int latencyUsec): Source(), mTestLatency(latencyUsec){};
  virtual ~TestSource(){};
  virtual int getLatencyUSec(void){ return mTestLatency; };
};

TEST_F(TestCase_PipeAndFilter, testAecSource)
{
  std::unique_ptr<IPipe> pPipe = std::make_unique<Pipe>();
  std::shared_ptr<ISource> pSource = std::make_shared<TestSource>( 5*1000 );
  std::shared_ptr<ISink> pActualSink = std::make_shared<TestSink>( 5*1000 ); // 5 msec latency
  std::shared_ptr<ISource> pReferenceSource = std::make_shared<ReferenceSoundSinkSource>( pActualSink );
  std::shared_ptr<ISource> pAecSource = std::make_shared<AccousticEchoCancelledSource>( pSource, pReferenceSource );
  std::shared_ptr<ISink> pSink = std::make_shared<Sink>();
  pPipe->attachSource( pAecSource );
  pPipe->addFilterToTail( std::make_shared<Filter>() );
  pPipe->attachSink( pSink );
  pPipe->run();
  std::this_thread::sleep_for(std::chrono::microseconds(1000));
  pPipe->stop();
  pSink->dump();
}

TEST_F(TestCase_PipeAndFilter, testAecSourceDelayOnly)
{
  std::unique_ptr<IPipe> pPipe = std::make_unique<Pipe>();
  std::shared_ptr<ISource> pSource = std::make_shared<TestSource>( 5*1000 );
  std::shared_ptr<ISink> pActualSink = std::make_shared<TestSink>( 5*1000 ); // 5 msec latency
  std::shared_ptr<ISource> pReferenceSource = std::make_shared<ReferenceSoundSinkSource>( pActualSink );
  std::shared_ptr<ISource> pAecSource = std::make_shared<AccousticEchoCancelledSource>( pSource, pReferenceSource, true );
  std::shared_ptr<ISink> pSink = std::make_shared<Sink>();
  pPipe->attachSource( pAecSource );
  pPipe->addFilterToTail( std::make_shared<Filter>() );
  pPipe->attachSink( pSink );
  pPipe->run();
  std::this_thread::sleep_for(std::chrono::microseconds(1000));
  pPipe->stop();
  pSink->dump();
}


TEST_F(TestCase_PipeAndFilter, testDynamicSignalFlow_AddNewSinkToReferenceSoundSink)
{
  std::unique_ptr<IPipe> pPipe = std::make_unique<Pipe>();

  std::shared_ptr<ISink> pGlobalSink1 = std::make_shared<Sink>();
  std::shared_ptr<ReferenceSoundSinkSource> pReferenceSource = std::make_shared<ReferenceSoundSinkSource>( pGlobalSink1 );
  std::shared_ptr<ISource> pRawMicSource = std::make_shared<TestSource>( 5*1000 );
  std::shared_ptr<AccousticEchoCancelledSource> pAecedMicSource = std::make_shared<AccousticEchoCancelledSource>( pRawMicSource, pReferenceSource );

  pPipe->attachSource( pAecedMicSource );

  pPipe->addFilterToTail( std::make_shared<Filter>() );

  std::shared_ptr<ISink> pMicSink = std::make_shared<Sink>();
  pPipe->attachSink( pMicSink );

  pPipe->run();
  std::this_thread::sleep_for(std::chrono::microseconds(1000));

  std::shared_ptr<ISink> pGlobalSink2 = std::make_shared<TestSink>(1*1000);
  std::shared_ptr<ISink> pDetachedSinkFromRefSound1 = pReferenceSource->attachSink(pGlobalSink2);
  EXPECT_EQ( pDetachedSinkFromRefSound1, pGlobalSink1 );
  pAecedMicSource->adjustDelay();
  std::this_thread::sleep_for(std::chrono::microseconds(5000));

  pPipe->stop();
  pMicSink->dump();

  std::shared_ptr<ISink> pDetachedMicSink = pPipe->detachSink();
  EXPECT_EQ( pDetachedMicSink, pMicSink );

  std::shared_ptr<ISource> pDetachedMicSource = pPipe->detachSource();
  EXPECT_EQ( pDetachedMicSource, pAecedMicSource );

  std::shared_ptr<ISink> pDetachedSinkFromRefSound2 = pReferenceSource->detachSink();
  EXPECT_EQ( pDetachedSinkFromRefSound2, pGlobalSink2 );
}

TEST_F(TestCase_PipeAndFilter, testPerChannelVolumeWithSink)
{
  std::shared_ptr<ISource> pSource = std::make_shared<Source>();
  std::shared_ptr<ISink> pSink = std::make_shared<Sink>();
  std::unique_ptr<IPipe> pPipe = std::make_unique<Pipe>();
  pPipe->attachSource( pSource );
  pPipe->attachSink( pSink );
  pPipe->addFilterToTail( std::make_shared<Filter>() );
  Volume::CHANNEL_VOLUME perChannelVolume;
  perChannelVolume.insert_or_assign( AudioFormat::CH::L, 100.0f );
  perChannelVolume.insert_or_assign( AudioFormat::CH::R, 0.0f );
  pSink->setVolume( perChannelVolume );
  pPipe->run();
  std::this_thread::sleep_for(std::chrono::microseconds(1000));
  pPipe->stop();
  pSink->dump();
  pPipe->clearFilters();
}

TEST_F(TestCase_PipeAndFilter, testPerChannelVolumeWithMultiSink)
{
  class TestSink : public Sink
  {
    int mTestLatency;
  public:
    TestSink(int latencyUsec): mTestLatency(latencyUsec){};
    virtual ~TestSink(){};
    virtual int getLatencyUSec(void){ return mTestLatency; };
  };

  std::shared_ptr<ISource> pSource = std::make_shared<Source>();
  std::shared_ptr<MultipleSink> pMultiSink = std::make_shared<MultipleSink>();

  std::shared_ptr<ISink> pSink1 = std::make_shared<TestSink>( 10*1000 );
  AudioFormat::ChannelMapper chMap1;
  chMap1.insert( std::make_pair(AudioFormat::CH::L, AudioFormat::CH::L) ); // dst, src
  chMap1.insert( std::make_pair(AudioFormat::CH::R, AudioFormat::CH::L) ); // dst, src
  pMultiSink->attachSink( pSink1, chMap1 );

  std::shared_ptr<ISink> pSink2 = std::make_shared<TestSink>( 10*1000 );
  AudioFormat::ChannelMapper chMap2;
  chMap2.insert( std::make_pair(AudioFormat::CH::L, AudioFormat::CH::R) ); // dst, src
  chMap2.insert( std::make_pair(AudioFormat::CH::R, AudioFormat::CH::R) ); // dst, src
  pMultiSink->attachSink( pSink2, chMap2 );

  std::unique_ptr<IPipe> pPipe = std::make_unique<Pipe>();
  pPipe->attachSource( pSource );
  pPipe->attachSink( pMultiSink );
  pPipe->addFilterToTail( std::make_shared<Filter>() );
  Volume::CHANNEL_VOLUME perChannelVolume;
  perChannelVolume.insert_or_assign( AudioFormat::CH::L, 100.0f );
  perChannelVolume.insert_or_assign( AudioFormat::CH::R, 0.0f );
  pMultiSink->setVolume( perChannelVolume );
  EXPECT_EQ( pMultiSink->getVolume(), 100.0f );
  pPipe->run();
  std::this_thread::sleep_for(std::chrono::microseconds(1000));
  pPipe->stop();
  pMultiSink->dump();
  pMultiSink->clearSinks();
  pPipe->clearFilters();
}

class CompressedSource : public Source
{
protected:
  AudioFormat mFormat;
public:
  CompressedSource():Source(){};
  virtual ~CompressedSource(){};
  virtual void setAudioFormat(AudioFormat format){ mFormat=format; };
  virtual AudioFormat getAudioFormat(void){ return mFormat; };
  virtual void readPrimitive(IAudioBuffer& buf){
    ByteBuffer esRawBuf( 256, 0 );
    buf.setRawBuffer( esRawBuf );
    buf.setAudioFormat( mFormat );
  }
  virtual std::string toString(void){return "CompressedSource";};
};

class CompressedSink : public Sink
{
protected:
  std::vector<AudioFormat> mAudioFormats;
  AudioFormat mFormat;

public:
  CompressedSink(AudioFormat::ENCODING encodingStartPoint = AudioFormat::ENCODING::COMPRESSED):Sink(){
    for(int anEncoding = encodingStartPoint; anEncoding < AudioFormat::ENCODING::COMPRESSED_UNKNOWN; anEncoding++){
      mAudioFormats.push_back( AudioFormat((AudioFormat::ENCODING)anEncoding) );
    }
  };
  virtual ~CompressedSink(){};
  virtual bool setAudioFormat(AudioFormat format){
    mFormat=format;
    Sink::setAudioFormat(format);
    return true;
  };
  virtual AudioFormat getAudioFormat(void){ return mFormat; };
  std::vector<AudioFormat> getSupportedAudioFormats(void){ return mAudioFormats; };
  virtual std::string toString(void){return "CompressedSink";};
};

TEST_F(TestCase_PipeAndFilter, testEncodedSink)
{
  // Signal flow : CompressedSource -> Pipe -> EncodedSink -> CompressedSink
  std::shared_ptr<IPipe> pPipe = std::make_shared<Pipe>();
  std::shared_ptr<CompressedSource> pSource = std::make_shared<CompressedSource>();
  std::shared_ptr<EncodedSink> pSink = std::make_shared<EncodedSink>();
  pSink->attachSink( std::make_shared<CompressedSink>( AudioFormat::ENCODING::COMPRESSED_AC3 ) );
  pSink->setTranscodeEnabled(true);
  pPipe->attachSource( pSource );
  pPipe->attachSink( pSink );

  // Sink can support case
  std::cout << "Sink supportable case : " << 
    AudioFormat::getEncodingString(AudioFormat::ENCODING::COMPRESSED_AC3) << "->" <<
    AudioFormat::getEncodingString(AudioFormat::ENCODING::COMPRESSED_AC3) << 
    std::endl;
  pSource->setAudioFormat( AudioFormat::ENCODING::COMPRESSED_AC3 );
  pSink->setAudioFormat( AudioFormat::ENCODING::COMPRESSED_AC3 );
  pPipe->run();
  std::this_thread::sleep_for(std::chrono::microseconds(1000));
  pPipe->stop();

  // Decoder only case
  std::cout << "Decoder only case : " << 
    AudioFormat::getEncodingString(AudioFormat::ENCODING::COMPRESSED_AAC) << "->" <<
    AudioFormat::getEncodingString(AudioFormat::ENCODING::PCM_16BIT) << 
    std::endl;
  pSource->setAudioFormat( AudioFormat::ENCODING::COMPRESSED_AAC );
  pSink->setAudioFormat( AudioFormat::ENCODING::PCM_16BIT );
  pPipe->run();
  std::this_thread::sleep_for(std::chrono::microseconds(1000));
  pPipe->stop();

  // Encoder only case
  std::cout << "Encoder only case : " << 
    AudioFormat::getEncodingString(AudioFormat::ENCODING::PCM_16BIT) << "->" <<
    AudioFormat::getEncodingString(AudioFormat::ENCODING::COMPRESSED_AC3) << 
    std::endl;
  pSource->setAudioFormat( AudioFormat::ENCODING::PCM_16BIT );
  pSink->setAudioFormat( AudioFormat::ENCODING::COMPRESSED_AC3 );
  pPipe->run();
  std::this_thread::sleep_for(std::chrono::microseconds(1000));
  pPipe->stop();

  // transcoder case
  std::cout << "Transcoder case : " << 
    AudioFormat::getEncodingString(AudioFormat::ENCODING::COMPRESSED_AAC) << "->" <<
    AudioFormat::getEncodingString(AudioFormat::ENCODING::COMPRESSED_AC3) << 
    std::endl;
  pSource->setAudioFormat( AudioFormat::ENCODING::COMPRESSED_AAC );
  pSink->setAudioFormat( AudioFormat::ENCODING::COMPRESSED_AC3 );
  pPipe->run();
  std::this_thread::sleep_for(std::chrono::microseconds(1000));
  pPipe->stop();

  // PCM format conversion case
  std::cout << "format conversion case : " << 
    AudioFormat::getEncodingString(AudioFormat::ENCODING::PCM_16BIT) << "->" <<
    AudioFormat::getEncodingString(AudioFormat::ENCODING::PCM_32BIT) << 
    std::endl;
  pSource->setAudioFormat( AudioFormat::ENCODING::PCM_16BIT );
  pSink->setAudioFormat( AudioFormat::ENCODING::PCM_32BIT );
  pPipe->run();
  std::this_thread::sleep_for(std::chrono::microseconds(1000));
  pPipe->stop();
}

class VirtualizerA : public Filter
{
public:
  static inline std::string applyConditionKey = "virtualizer.virtualizerA.applyCondition";
  static void ensureDefaultAssumption(void){
    std::vector<AudioFormat::ENCODING> encodingsA = {
      AudioFormat::ENCODING::COMPRESSED_AC3,
      AudioFormat::ENCODING::COMPRESSED_E_AC3,
      AudioFormat::ENCODING::COMPRESSED_AC4,
      AudioFormat::ENCODING::COMPRESSED_DOLBY_TRUEHD,
      AudioFormat::ENCODING::COMPRESSED_MAT,
    };
    std::string applyConditionA;
    for(auto& anEncoding : encodingsA ){
      applyConditionA = applyConditionA + std::to_string((int)anEncoding) + ",";
    }
    ParameterManager* pParams = ParameterManager::getManager();
    pParams->setParameter(applyConditionKey, applyConditionA);
  };
  VirtualizerA():Filter(){
    ensureDefaultAssumption();
  }
  virtual ~VirtualizerA(){};
  virtual void process(AudioBuffer& inBuf, AudioBuffer& outBuf)
  {
    std::cout << "virtualizer A" << std::endl;
    ByteBuffer buf( inBuf.getRawBuffer().size(), 'A');
    outBuf.setRawBuffer(buf);
  }
  virtual std::string toString(void){ return "VirtualizerA"; };
};

class VirtualizerB : public Filter
{
public:
  static inline std::string applyConditionKey = "virtualizer.virtualizerB.applyCondition";
  static void ensureDefaultAssumption(void){
    std::vector<AudioFormat::ENCODING> encodingsB = {
      AudioFormat::ENCODING::COMPRESSED_DTS,
      AudioFormat::ENCODING::COMPRESSED_DTS_HD,
    };
    std::string applyConditionB;
    for(auto& anEncoding : encodingsB ){
      applyConditionB = applyConditionB + std::to_string((int)anEncoding) + ",";
    }
    ParameterManager* pParams = ParameterManager::getManager();
    pParams->setParameter(applyConditionKey, applyConditionB);
  };
  VirtualizerB():Filter(){
    ensureDefaultAssumption();
  };
  virtual ~VirtualizerB(){};
  virtual void process(AudioBuffer& inBuf, AudioBuffer& outBuf)
  {
    std::cout << "virtualizer B" << std::endl;
    ByteBuffer buf( inBuf.getRawBuffer().size(), 'B');
    outBuf.setRawBuffer(buf);
  }
  virtual std::string toString(void){ return "VirtualizerA"; };
};

TEST_F(TestCase_PipeAndFilter, testPipeSetupByCondition)
{
  ParameterManager* pParams = ParameterManager::getManager();

  class TunnelPlaybackContext : public StrategyContext
  {
  public:
    std::shared_ptr<IPipe> pPipe;
    std::shared_ptr<ISource> pSource;
    std::shared_ptr<ISink> pSink;
    std::shared_ptr<IPlayer> pPlayer;

  public:
    TunnelPlaybackContext():StrategyContext(){};
    TunnelPlaybackContext(std::shared_ptr<IPipe> pPipe, std::shared_ptr<ISource> pSource, std::shared_ptr<ISink> pSink):StrategyContext(), pPipe(pPipe), pSource(pSource), pSink(pSink){};
    virtual ~TunnelPlaybackContext(){
      pPipe.reset();
      pSource.reset();
      pSink.reset();
      pPlayer.reset();
    };
  };

  class TunnelPlaybackStrategy : public IStrategy
  {
  protected:
    std::shared_ptr<IMediaCodec> getCodec(AudioFormat format){
      return IMediaCodec::createByFormat(format, true);
    }
    bool shouldHandleFormat(AudioFormat format, std::string encodings)
    {
      std::cout << "shouldHandleFormat(" << (int)format.getEncoding() << " , " << encodings << " )" << std::endl;
      // TODO: should expand not only encodings but also the other conditions such as channels
      StringTokenizer tok(encodings, ",");
      AudioFormat::ENCODING theEncoding = format.getEncoding();
      while( tok.hasNext() ){
        if( (int)theEncoding == std::stoi(tok.getNext()) ){
          return true;
        }
      }
      return false;
    }
    std::shared_ptr<IFilter> getVirtualizer(AudioFormat format, std::shared_ptr<StrategyContext> context){
      ParameterManager* pParams = ParameterManager::getManager();
      if( shouldHandleFormat(format, pParams->getParameter(VirtualizerA::applyConditionKey) ) ){
        std::cout << "Create instance of Virtualizer A" << std::endl;
        return std::make_shared<VirtualizerA>();
      } else if( shouldHandleFormat(format, pParams->getParameter(VirtualizerB::applyConditionKey) ) ){
        std::cout << "Create instance of Virtualizer B" << std::endl;
        return std::make_shared<VirtualizerB>();
      }
      return nullptr;
    }
    std::vector<std::shared_ptr<IFilter>> getFilters(AudioFormat format, std::shared_ptr<StrategyContext> context){
      std::vector<std::shared_ptr<IFilter>> filters;
      std::shared_ptr<IFilter> pFilter = getVirtualizer(format, context);
      if( pFilter ){
        filters.push_back( pFilter );
      }

      return filters;
    }
  public:
    TunnelPlaybackStrategy():IStrategy(){};
    virtual ~TunnelPlaybackStrategy(){};
    virtual bool canHandle(std::shared_ptr<StrategyContext> context){
      return true;
    }
    virtual bool execute(std::shared_ptr<StrategyContext> pContext){
      std::shared_ptr<TunnelPlaybackContext> context = std::dynamic_pointer_cast<TunnelPlaybackContext>(pContext);
      if( context ){
        ParameterManager* pParams = ParameterManager::getManager();
        // ensure pipe
        if( !context->pPipe ){
          std::cout << "create Pipe instance" << std::endl;
          context->pPipe = std::make_shared<Pipe>();
        } else {
          context->pPipe->stop();
          context->pPipe->clearFilters();
        }
        // setup encoder sink if necessary
        bool bTranscoder = pParams->getParameterBool("sink.transcoder");
        if( bTranscoder && context->pSink && !std::dynamic_pointer_cast<EncodedSink>(context->pSink) ){
          std::cout << "create EncodedSink instance" << std::endl;
          context->pSink = std::make_shared<EncodedSink>(context->pSink, bTranscoder);
        }
        // setup source : player if necessary
        // should be sink.passthrough = false if SpeakerSink, HeadphoneSink and BluetoothAudioSink
        if( context->pSource ){
          AudioFormat srcFormat = context->pSource->getAudioFormat();
          if( srcFormat.isEncodingCompressed() && !pParams->getParameterBool("sink.passthrough") ){
            std::cout << "create Player instance" << std::endl;
            context->pPlayer = std::make_shared<Player>();
            context->pPipe->attachSource(
              context->pPlayer->prepare(
                context->pSource,
                getCodec( context->pSource->getAudioFormat() )
              )
            );
          }
          if( srcFormat.isEncodingPcm() || context->pPlayer ){
            std::cout << "create Filter instance" << std::endl;
            std::vector<std::shared_ptr<IFilter>> pFilters = getFilters(srcFormat, context);
            for( auto& aFilter : pFilters ){
              context->pPipe->addFilterToTail( aFilter );
            }
          }
          context->pPipe->attachSource( context->pSource );
        }

        // setup sink
        context->pPipe->attachSink(context->pSink);

        return context->pPipe != nullptr;
      } else {
        return false;
      }
    }
  };

  VirtualizerA::ensureDefaultAssumption();
  VirtualizerB::ensureDefaultAssumption();

  std::shared_ptr<CompressedSource> pSource = std::make_shared<CompressedSource>();
  pSource->setAudioFormat( AudioFormat::ENCODING::COMPRESSED_AC3 );
  std::shared_ptr<CompressedSink> pSink = std::make_shared<CompressedSink>(AudioFormat::ENCODING::COMPRESSED_AC3);
  pSink->setAudioFormat(AudioFormat::ENCODING::COMPRESSED_AC3);

  std::shared_ptr<TunnelPlaybackContext> pContext = std::make_shared<TunnelPlaybackContext>();
  pContext->pSource = pSource;
  pContext->pSink = pSink;
  std::shared_ptr<TunnelPlaybackStrategy> pStrategy = std::make_shared<TunnelPlaybackStrategy>();

  // no player, no filter, EncodedSink with transcode
  pParams->setParameterBool("sink.transcoder", true);
  pParams->setParameterBool("sink.passthrough", true);
  EXPECT_TRUE( pStrategy->execute(pContext) );
  pContext->pPipe->dump();
  pContext->pPipe->run();
  std::this_thread::sleep_for(std::chrono::microseconds(1000));
  pContext->pPipe->stop();
  pContext->pSink->dump();

  // player, virtualizerA filter, Sink
  pSink->setAudioFormat(AudioFormat::ENCODING::PCM_16BIT);
  pParams->setParameterBool("sink.transcoder", false);
  pParams->setParameterBool("sink.passthrough", false);
  EXPECT_TRUE( pStrategy->execute(pContext) );
  pContext->pPipe->dump();
  pContext->pPipe->run();
  std::this_thread::sleep_for(std::chrono::microseconds(1000));
  pContext->pPipe->stop();
  pContext->pSink->dump();

  // player, virtualizerB filter, Sink
  pSink->setAudioFormat(AudioFormat::ENCODING::PCM_16BIT);
  pSource->setAudioFormat(AudioFormat::ENCODING::COMPRESSED_DTS);
  pParams->setParameterBool("sink.transcoder", false);
  pParams->setParameterBool("sink.passthrough", false);
  EXPECT_TRUE( pStrategy->execute(pContext) );
  pContext->pPipe->dump();
  pContext->pPipe->run();
  std::this_thread::sleep_for(std::chrono::microseconds(1000));
  pContext->pPipe->stop();
  pContext->pSink->dump();
}


int main(int argc, char **argv)
{
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
