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
  std::this_thread::sleep_for(std::chrono::microseconds(1000));
  EXPECT_TRUE( fifoBuf.write( writeBuf ) );
  thx.join();
  EXPECT_TRUE( bResult );
}

TEST_F(TestCase_PipeAndFilter, testInterPipeBridge)
{
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

TEST_F(TestCase_PipeAndFilter, testMultipleSink2)
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
  std::shared_ptr<ISource> pSource = std::make_shared<Source>();
  std::shared_ptr<ISink> pActualSink = std::make_shared<Sink>();
  pActualSink->setVolume(50.0);

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


TEST_F(TestCase_PipeAndFilter, testParameterManager)
{
  ParameterManager* pParams = ParameterManager::getManager();

  ParameterManager::CALLBACK callbackW = [](std::string key, std::string value){
    std::cout << "callback(param*)): [" << key << "] = " << value << std::endl;
  };
  int callbackIdW = pParams->registerCallback("param*", callbackW);

  pParams->setParameter("paramA", "ABC");
  EXPECT_TRUE( pParams->getParameter("paramA", "HOGE") == "ABC" );
  pParams->setParameterBool("paramB", true);
  EXPECT_TRUE( pParams->getParameterBool("paramB", false) == true );
  pParams->setParameterInt("paramC", 1);
  EXPECT_TRUE( pParams->getParameterInt("paramC", 0) == 1 );
  EXPECT_TRUE( pParams->getParameterInt("paramD", -1) == -1 );

  std::vector<std::string> keys = {"paramA", "paramB", "paramC"};
  std::vector<ParameterManager::Param> params = pParams->getParameters(keys);
  EXPECT_EQ( params.size(), 3 );

  pParams->setParameterInt("ro.paramD", 1);
  EXPECT_TRUE( pParams->getParameterInt("ro.paramD", 0) == 1 );
  pParams->setParameterInt("ro.paramD", 2);
  EXPECT_TRUE( pParams->getParameterInt("ro.paramD", 0) == 1 );

  ParameterManager::CALLBACK callback2 = [](std::string key, std::string value){
    std::cout << "callback(exact match): [" << key << "] = " << value << std::endl;
  };
  int callbackId1 = pParams->registerCallback("paramC", callback2);
  int callbackId2 = pParams->registerCallback("ro.paramD", callback2);
  pParams->setParameterInt("paramC", 1);
  pParams->setParameterInt("paramC", 2);
  pParams->setParameterInt("paramC", 3);
  pParams->setParameterInt("ro.paramD", 3);

  pParams->unregisterCallback(callbackIdW);
  pParams->unregisterCallback(callbackId1);
  pParams->unregisterCallback(callbackId2);
  pParams->unregisterCallback(10000);
  std::cout << "unregistered all notifier" << std::endl;
  pParams->setParameterInt("paramC", 4);

  // dump all
  std::cout << "getParameters()" << std::endl;
  std::vector<ParameterManager::Param> paramsAll = pParams->getParameters();
  for(auto& aParam : paramsAll){
    std::cout << aParam.key << " = " << aParam.value << std::endl;
  }
  std::cout << std::endl;

  std::cout << "getParameters(\"param*\")" << std::endl;
  std::vector<ParameterManager::Param> paramsWilds = pParams->getParameters("param*");
  for(auto& aParam : paramsWilds){
    std::cout << aParam.key << " = " << aParam.value << std::endl;
  }
  std::cout << std::endl;

  const std::string paramFilePath = "TestProperties";

  if( std::filesystem::exists( paramFilePath) ){
    std::filesystem::remove( paramFilePath );
  }

  std::cout << "store to stream" << std::endl;
  FileStream* pFileStream = new FileStream( paramFilePath );
  pParams->storeToStream( pFileStream );
  pFileStream->close();

  std::cout << "reset all of params" << std::endl;
  pParams->resetAllOfParams();

  std::cout << "restore from stream" << std::endl;
  pFileStream = new FileStream( paramFilePath );
  pParams->restoreFromStream( pFileStream );
  pFileStream->close();

  std::cout << "reset all of params" << std::endl;
  pParams->resetAllOfParams();

  // non-override restore. This helps to implement default params and user params load. Load current user config value(override=true) and Load the default(preset) value (override=false).
  pParams->setParameter("paramA", "XXX");
  std::cout << "restore from stream" << std::endl;
  pFileStream = new FileStream( paramFilePath );
  pParams->restoreFromStream( pFileStream, false ); // no override
  pFileStream->close();
  EXPECT_EQ( pParams->getParameter("paramA"), "XXX");

  paramsAll = pParams->getParameters();
  for(auto& aParam : paramsAll){
    std::cout << aParam.key << " = " << aParam.value << std::endl;
  }
}

TEST_F(TestCase_PipeAndFilter, testStringTokenizer)
{
  std::string target1 = "data1:data2:data3:data4";
  StringTokenizer tok1(target1, ":");

  std::cout << "#1 getNext()" << std::endl;
  EXPECT_TRUE( tok1.hasNext() );
  EXPECT_EQ( tok1.getNext(), "data1" );

  std::cout << "#2 getNext()" << std::endl;
  EXPECT_TRUE( tok1.hasNext() );
  EXPECT_EQ( tok1.getNext(), "data2" );

  std::cout << "#3 getNext()" << std::endl;
  EXPECT_TRUE( tok1.hasNext() );
  EXPECT_EQ( tok1.getNext(), "data3" );

  std::cout << "#4 getNext()" << std::endl;
  EXPECT_TRUE( tok1.hasNext() );
  EXPECT_EQ( tok1.getNext(), "data4" );

  std::cout << "#5 getNext()" << std::endl;
  EXPECT_FALSE( tok1.hasNext() );

  std::string target2 = "data1:data2:data3:data4";
  StringTokenizer tok2(target1, ",");
  EXPECT_TRUE( tok2.hasNext() );
  EXPECT_EQ( tok2.getNext(), target2 );

  std::string target3 = "data1 : data2 : data3 : data4";
  StringTokenizer tok3(target3, " : ");

  std::cout << "#1 getNext()" << std::endl;
  EXPECT_TRUE( tok3.hasNext() );
  EXPECT_EQ( tok3.getNext(), "data1" );

  std::cout << "#2 getNext()" << std::endl;
  EXPECT_TRUE( tok3.hasNext() );
  EXPECT_EQ( tok3.getNext(), "data2" );

  std::cout << "#3 getNext()" << std::endl;
  EXPECT_TRUE( tok3.hasNext() );
  EXPECT_EQ( tok3.getNext(), "data3" );

  std::cout << "#4 getNext()" << std::endl;
  EXPECT_TRUE( tok3.hasNext() );
  EXPECT_EQ( tok3.getNext(), "data4" );

  std::cout << "#5 getNext()" << std::endl;
  EXPECT_FALSE( tok3.hasNext() );
}


TEST_F(TestCase_PipeAndFilter, testPlugInManager)
{
  IPlugInManager* pPlugInManager = new IPlugInManager();
  pPlugInManager->initialize();

  std::vector<std::string> plugInIds = pPlugInManager->getPlugInIds();
  for(auto& aPlugInId : plugInIds){
    EXPECT_TRUE( pPlugInManager->hasPlugIn( aPlugInId ) );
    EXPECT_NE( nullptr, pPlugInManager->getPlugIn( aPlugInId ) );
  }
  EXPECT_FALSE( pPlugInManager->hasPlugIn( "hogehogehoge" ) );

  pPlugInManager->terminate();
}

TEST_F(TestCase_PipeAndFilter, testFilterPlugInManager)
{
  FilterManager::setPlugInPath("lib/");
  FilterManager* pManager = FilterManager::getInstance();
  pManager->initialize();

  std::vector<std::string> plugInIds = pManager->getPlugInIds();
  for(auto& aPlugInId : plugInIds){
    EXPECT_TRUE( pManager->hasPlugIn( aPlugInId ) );
    EXPECT_NE( nullptr, pManager->getPlugIn( aPlugInId ) );
    std::shared_ptr<IFilter> pFilter = FilterManager::newInstanceById( aPlugInId );
    EXPECT_NE( nullptr, pFilter );
  }
  EXPECT_FALSE( FilterManager::newInstanceById( "hogehogehoge" ) );

  pManager->terminate();
}

TEST_F(TestCase_PipeAndFilter, testSourcePlugInManager)
{
  SourceManager::setPlugInPath("lib/");
  SourceManager* pManager = SourceManager::getInstance();
  pManager->initialize();

  std::vector<std::string> plugInIds = pManager->getPlugInIds();
  for(auto& aPlugInId : plugInIds){
    EXPECT_TRUE( pManager->hasPlugIn( aPlugInId ) );
    EXPECT_NE( nullptr, pManager->getPlugIn( aPlugInId ) );
    std::shared_ptr<ISource> pSource = SourceManager::newInstanceById( aPlugInId );
    EXPECT_NE( nullptr, pSource );
  }
  EXPECT_FALSE( SourceManager::newInstanceById( "hogehogehoge" ) );

  pManager->terminate();
}

TEST_F(TestCase_PipeAndFilter, testSinkPlugInManager)
{
  SinkManager::setPlugInPath("lib/");
  SinkManager* pManager = SinkManager::getInstance();
  pManager->initialize();

  std::vector<std::string> plugInIds = pManager->getPlugInIds();
  for(auto& aPlugInId : plugInIds){
    EXPECT_TRUE( pManager->hasPlugIn( aPlugInId ) );
    EXPECT_NE( nullptr, pManager->getPlugIn( aPlugInId ) );
    std::shared_ptr<ISink> pSink = SinkManager::newInstanceById( aPlugInId );
    EXPECT_NE( nullptr, pSink );
  }
  EXPECT_FALSE( SinkManager::newInstanceById( "hogehogehoge" ) );

  pManager->terminate();
}


TEST_F(TestCase_PipeAndFilter, testDelayFilter)
{
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


TEST_F(TestCase_PipeAndFilter, testSinkCapture)
{
  std::shared_ptr<ISource> pSource = std::make_shared<Source>();
  std::shared_ptr<ISink> pSink = std::dynamic_pointer_cast<ISink>( std::make_shared<SinkCapture>( std::make_shared<Sink>() ) );
  std::shared_ptr<ICapture> pCapture = std::dynamic_pointer_cast<ICapture>(pSink);
  std::unique_ptr<IPipe> pPipe = std::make_unique<Pipe>();

  pPipe->attachSource( pSource );
  pPipe->addFilterToTail( std::make_shared<FilterIncrement>() );
  pPipe->attachSink( pSink );

  pPipe->run();
  std::this_thread::sleep_for(std::chrono::microseconds(1000));
  AudioBuffer captureBuf( AudioFormat(), 240 );
  std::cout << "captureRead" << std::endl;
  pCapture->captureRead( captureBuf );
  pPipe->stop();
  Util::dumpBuffer( "SinkCapture result", captureBuf );

  EXPECT_EQ( pSink, pPipe->detachSink());
  EXPECT_EQ( pSource, pPipe->detachSource());
  std::cout << "Sink dump:" << std::endl;
  pSink->dump();
  pPipe->clearFilters();
  pCapture->unlock();
}

TEST_F(TestCase_PipeAndFilter, testSinkInjector)
{
  std::shared_ptr<ISource> pSource = std::make_shared<Source>();
  std::shared_ptr<ISink> pSink = std::dynamic_pointer_cast<ISink>( std::make_shared<SinkInjector>( std::make_shared<Sink>() ) );
  std::shared_ptr<IInjector> pInjector = std::dynamic_pointer_cast<IInjector>(pSink);
  std::unique_ptr<IPipe> pPipe = std::make_unique<Pipe>();

  pPipe->attachSource( pSource );
  pPipe->addFilterToTail( std::make_shared<Filter>() );
  pPipe->attachSink( pSink );

  // 1st step: non injected
  EXPECT_FALSE( pInjector->getInjectorEnabled() );
  pPipe->run();
  std::this_thread::sleep_for(std::chrono::microseconds(100));
  pPipe->stop();
  std::cout << "Sink dump(non injected):" << std::endl;
  pSink->dump();

  // 2nd step: injected
  AudioBuffer injectBuf( AudioFormat(), 240 );
  ByteBuffer rawInjectBuf = injectBuf.getRawBuffer();
  for(int i=0; i<rawInjectBuf.size(); i=i+2){
    rawInjectBuf[i] = i % 16;
    rawInjectBuf[i+1] = 0;
  }
  injectBuf.setRawBuffer( rawInjectBuf );
  Util::dumpBuffer("injectBuf", injectBuf);

  pInjector->setInjectorEnabled( true );
  EXPECT_TRUE( pInjector->getInjectorEnabled() );

  pPipe->run();
  {
    std::chrono::milliseconds start = duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch());
    bool bRunning = true;
    while( bRunning ){
      pInjector->inject( injectBuf );
      std::chrono::milliseconds now = duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch());
      bRunning = ( ( now.count() - start.count() ) <= 2 ) ? true : false; // run 2msec
    };
  }
  pPipe->stop();

  EXPECT_EQ( pSink, pPipe->detachSink());
  EXPECT_EQ( pSource, pPipe->detachSource());
  std::cout << "Sink dump(injected result):" << std::endl;
  pSink->dump();
  pPipe->clearFilters();
  pInjector->unlock();
}


TEST_F(TestCase_PipeAndFilter, testSourceCapture)
{
  std::shared_ptr<ISource> pSource = std::dynamic_pointer_cast<ISource>( std::make_shared<SourceCapture>( std::make_shared<Source>() ) );
  std::shared_ptr<ISink> pSink = std::make_shared<Sink>();
  std::shared_ptr<ICapture> pCapture = std::dynamic_pointer_cast<ICapture>(pSource);
  std::unique_ptr<IPipe> pPipe = std::make_unique<Pipe>();

  pPipe->attachSource( pSource );
  pPipe->addFilterToTail( std::make_shared<FilterIncrement>() );
  pPipe->attachSink( pSink );

  pPipe->run();
  std::this_thread::sleep_for(std::chrono::microseconds(1000));
  AudioBuffer captureBuf( AudioFormat(), 240 );
  std::cout << "captureRead" << std::endl;
  pCapture->captureRead( captureBuf );
  pPipe->stop();
  Util::dumpBuffer( "SourceCapture result", captureBuf );

  EXPECT_EQ( pSink, pPipe->detachSink());
  EXPECT_EQ( pSource, pPipe->detachSource());
  std::cout << "Sink dump:" << std::endl;
  pSink->dump();
  pPipe->clearFilters();
  pCapture->unlock();
}

TEST_F(TestCase_PipeAndFilter, testSourceInjector)
{
  std::shared_ptr<ISource> pSource = std::dynamic_pointer_cast<ISource>( std::make_shared<SourceInjector>( std::make_shared<Source>() ) );
  std::shared_ptr<ISink> pSink = std::make_shared<Sink>();
  std::shared_ptr<IInjector> pInjector = std::dynamic_pointer_cast<IInjector>(pSource);
  std::unique_ptr<IPipe> pPipe = std::make_unique<Pipe>();

  pPipe->attachSource( pSource );
  pPipe->addFilterToTail( std::make_shared<Filter>() );
  pPipe->attachSink( pSink );

  // 1st step: non injected
  EXPECT_FALSE( pInjector->getInjectorEnabled() );
  pPipe->run();
  std::this_thread::sleep_for(std::chrono::microseconds(100));
  pPipe->stop();
  std::cout << "Sink dump(non injected):" << std::endl;
  pSink->dump();
  EXPECT_EQ( pSink, pPipe->detachSink());
  pSink = std::make_shared<Sink>();
  pPipe->attachSink( pSink );

  // 2nd step: injected
  AudioBuffer injectBuf( AudioFormat(), 240 );
  ByteBuffer rawInjectBuf = injectBuf.getRawBuffer();
  for(int i=0; i<rawInjectBuf.size(); i=i+2){
    rawInjectBuf[i] = i % 16;
    rawInjectBuf[i+1] = 0;
  }
  injectBuf.setRawBuffer( rawInjectBuf );
  Util::dumpBuffer("injectBuf", injectBuf);

  pInjector->setInjectorEnabled( true );
  EXPECT_TRUE( pInjector->getInjectorEnabled() );

  pPipe->run();
  {
    std::chrono::milliseconds start = duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch());
    bool bRunning = true;
    while( bRunning ){
      pInjector->inject( injectBuf );
      std::chrono::milliseconds now = duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch());
      bRunning = ( ( now.count() - start.count() ) <= 2 ) ? true : false; // run 2msec
    };
  }
  pPipe->stop();

  EXPECT_EQ( pSink, pPipe->detachSink());
  EXPECT_EQ( pSource, pPipe->detachSource());
  std::cout << "Sink dump(injected result):" << std::endl;
  pSink->dump();
  pPipe->clearFilters();
  pInjector->unlock();
}


TEST_F(TestCase_PipeAndFilter, testFilterCapture)
{
  std::shared_ptr<ISource> pSource = std::make_shared<Source>();
  std::shared_ptr<ISink> pSink = std::make_shared<Sink>();
  std::unique_ptr<IPipe> pPipe = std::make_unique<Pipe>();

  pPipe->attachSource( pSource );
  pPipe->attachSink( pSink );

  pPipe->addFilterToTail( std::make_shared<FilterIncrement>() );
  std::shared_ptr<IFilter> pFilter = std::make_shared<FilterCapture>();
  pPipe->addFilterToTail( pFilter );
  std::shared_ptr<ICapture> pCapture = std::dynamic_pointer_cast<ICapture>(pFilter);

  pPipe->run();
  std::this_thread::sleep_for(std::chrono::microseconds(1000));
  AudioBuffer captureBuf( AudioFormat(), 240 );
  std::cout << "captureRead" << std::endl;
  pCapture->captureRead( captureBuf );
  pPipe->stop();
  Util::dumpBuffer( "FilterCapture result", captureBuf );

  EXPECT_EQ( pSink, pPipe->detachSink());
  EXPECT_EQ( pSource, pPipe->detachSource());
  std::cout << "Sink dump:" << std::endl;
  pSink->dump();
  pPipe->clearFilters();
  pCapture->unlock();
}


TEST_F(TestCase_PipeAndFilter, testFilterInjector)
{
  std::shared_ptr<ISource> pSource = std::make_shared<Source>();
  std::shared_ptr<ISink> pSink = std::make_shared<Sink>();
  std::unique_ptr<IPipe> pPipe = std::make_unique<Pipe>();

  pPipe->attachSource( pSource );
  pPipe->attachSink( pSink );

  pPipe->addFilterToTail( std::make_shared<FilterIncrement>() );
  std::shared_ptr<IFilter> pFilter = std::make_shared<FilterInjector>();
  pPipe->addFilterToTail( pFilter );
  std::shared_ptr<IInjector> pInjector = std::dynamic_pointer_cast<IInjector>(pFilter);

  // 1st step: non injected
  EXPECT_FALSE( pInjector->getInjectorEnabled() );
  pPipe->run();
  std::this_thread::sleep_for(std::chrono::microseconds(100));
  pPipe->stop();
  std::cout << "Sink dump(non injected):" << std::endl;
  pSink->dump();
  EXPECT_EQ( pSink, pPipe->detachSink());
  pSink = std::make_shared<Sink>();
  pPipe->attachSink( pSink );

  // 2nd step: injected
  AudioBuffer injectBuf( AudioFormat(), 240 );
  ByteBuffer rawInjectBuf = injectBuf.getRawBuffer();
  for(int i=0; i<rawInjectBuf.size(); i=i+2){
    rawInjectBuf[i] = i % 16;
    rawInjectBuf[i+1] = 0;
  }
  injectBuf.setRawBuffer( rawInjectBuf );
  Util::dumpBuffer("injectBuf", injectBuf);

  pInjector->setInjectorEnabled( true );
  EXPECT_TRUE( pInjector->getInjectorEnabled() );

  pPipe->run();
  {
    std::chrono::milliseconds start = duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch());
    bool bRunning = true;
    while( bRunning ){
      pInjector->inject( injectBuf );
      std::chrono::milliseconds now = duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch());
      bRunning = ( ( now.count() - start.count() ) <= 2 ) ? true : false; // run 2msec
    };
  }
  pPipe->stop();

  EXPECT_EQ( pSink, pPipe->detachSink());
  EXPECT_EQ( pSource, pPipe->detachSource());
  std::cout << "Sink dump:" << std::endl;
  pSink->dump();
  pPipe->clearFilters();
  pInjector->unlock();
}

TEST_F(TestCase_PipeAndFilter, testResourceManager)
{
  CpuResourceManager::admin_setResource(1000);
  IResourceManager* pResourceManager = CpuResourceManager::getInstance();
  EXPECT_NE( pResourceManager, nullptr);

  int nResourceId1 = pResourceManager->acquire(500);
  EXPECT_NE( nResourceId1, -1 );
  EXPECT_EQ( nResourceId1, 0 );

  int nResourceId2 = pResourceManager->acquire(300);
  EXPECT_NE( nResourceId2, -1 );
  EXPECT_EQ( nResourceId2, 1 );

  int nResourceId3 = pResourceManager->acquire(300);
  EXPECT_EQ( nResourceId3, -1 );

  EXPECT_TRUE( pResourceManager->release(nResourceId2) );

  int nResourceId4 = pResourceManager->acquire(500);
  EXPECT_NE( nResourceId4, -1 );
  EXPECT_EQ( nResourceId4, 2 );

  EXPECT_TRUE( pResourceManager->release(nResourceId1) );
  EXPECT_FALSE( pResourceManager->release(nResourceId3) );
  EXPECT_TRUE( pResourceManager->release(nResourceId4) );

  CpuResourceManager::admin_terminate();
  pResourceManager = nullptr;
}

TEST_F(TestCase_PipeAndFilter, testResourceManager_ResourceConsumer)
{
  class DummyConsumer:public IResourceConsumer
  {
  public:
    DummyConsumer(){};
    virtual ~DummyConsumer(){};
    virtual int stateResourceConsumption(void){ return 300; };
  };

  CpuResourceManager::admin_setResource(1000);
  IResourceManager* pResourceManager = CpuResourceManager::getInstance();
  EXPECT_NE( pResourceManager, nullptr);

  DummyConsumer* consumer1 = new DummyConsumer();
  EXPECT_TRUE( pResourceManager->acquire(consumer1) );
  EXPECT_FALSE( pResourceManager->acquire(consumer1) );

  DummyConsumer* consumer2 = new DummyConsumer();
  EXPECT_TRUE( pResourceManager->acquire(consumer2) );

  DummyConsumer* consumer3 = new DummyConsumer();
  EXPECT_TRUE( pResourceManager->acquire(consumer3) );

  DummyConsumer* consumer4 = new DummyConsumer();
  EXPECT_FALSE( pResourceManager->acquire(consumer4) );

  EXPECT_TRUE( pResourceManager->release(consumer3) );
  EXPECT_FALSE( pResourceManager->release(consumer3) );
  EXPECT_TRUE( pResourceManager->acquire(consumer4) );

  delete consumer4; consumer4=nullptr;

  DummyConsumer* consumer5 = new DummyConsumer();
  EXPECT_TRUE( pResourceManager->acquire(consumer5) );
  EXPECT_FALSE( pResourceManager->release(consumer4) );
  EXPECT_FALSE( pResourceManager->release(consumer3) );

  delete consumer1; consumer1=nullptr;
  delete consumer2; consumer2=nullptr;

  CpuResourceManager::admin_terminate();
  pResourceManager = nullptr;

  delete consumer3; consumer3=nullptr;
  delete consumer5; consumer5=nullptr;
}

TEST_F(TestCase_PipeAndFilter, testResourceManager_Filter)
{
  class DummyConsumer:public Filter
  {
  public:
    DummyConsumer():Filter(){};
    virtual ~DummyConsumer(){};
    virtual int stateResourceConsumption(void){
      return (int)((float)CpuResource::getComputingResource()/3.333f);
    };
  };

  CpuResourceManager::admin_setResource( CpuResource::getComputingResource() );
  IResourceManager* pResourceManager = CpuResourceManager::getInstance();
  EXPECT_NE( pResourceManager, nullptr);

  DummyConsumer* consumer1 = new DummyConsumer();
  EXPECT_TRUE( pResourceManager->acquire(consumer1) );
  EXPECT_FALSE( pResourceManager->acquire(consumer1) );

  DummyConsumer* consumer2 = new DummyConsumer();
  EXPECT_TRUE( pResourceManager->acquire(consumer2) );

  DummyConsumer* consumer3 = new DummyConsumer();
  EXPECT_TRUE( pResourceManager->acquire(consumer3) );

  DummyConsumer* consumer4 = new DummyConsumer();
  EXPECT_FALSE( pResourceManager->acquire(consumer4) );

  EXPECT_TRUE( pResourceManager->release(consumer3) );
  EXPECT_FALSE( pResourceManager->release(consumer3) );
  EXPECT_TRUE( pResourceManager->acquire(consumer4) );

  delete consumer4; consumer4=nullptr;

  DummyConsumer* consumer5 = new DummyConsumer();
  EXPECT_TRUE( pResourceManager->acquire(consumer5) );
  EXPECT_FALSE( pResourceManager->release(consumer4) );
  EXPECT_FALSE( pResourceManager->release(consumer3) );

  delete consumer1; consumer1=nullptr;
  delete consumer2; consumer2=nullptr;

  CpuResourceManager::admin_terminate();
  pResourceManager = nullptr;

  delete consumer3; consumer3=nullptr;
  delete consumer5; consumer5=nullptr;
}

TEST_F(TestCase_PipeAndFilter, testResourceManager_Pipe)
{
  class DummyFilter:public Filter
  {
  public:
    DummyFilter():Filter(){};
    virtual ~DummyFilter(){};
    virtual int stateResourceConsumption(void){
      return (int)((float)CpuResource::getComputingResource()/3.333f);
    };
  };

  CpuResourceManager::admin_setResource( CpuResource::getComputingResource() );
  IResourceManager* pResourceManager = CpuResourceManager::getInstance();
  EXPECT_NE( pResourceManager, nullptr);

  IPipe* pPipe = new Pipe();
  pPipe->addFilterToTail( std::make_shared<DummyFilter>() );

  bool bSuccessAcquiredResource = pResourceManager->acquire(pPipe);
  EXPECT_TRUE( bSuccessAcquiredResource );
  if( bSuccessAcquiredResource ){
    pPipe->run();
    pPipe->stop();
    EXPECT_TRUE( pResourceManager->release(pPipe) );
  }

  pPipe->addFilterToTail( std::make_shared<DummyFilter>() );
  bSuccessAcquiredResource = pResourceManager->acquire(pPipe);
  EXPECT_TRUE( bSuccessAcquiredResource );
  if( bSuccessAcquiredResource ){
    pPipe->run();
    pPipe->stop();
    EXPECT_TRUE( pResourceManager->release(pPipe) );
  }

  pPipe->addFilterToTail( std::make_shared<DummyFilter>() );
  bSuccessAcquiredResource = pResourceManager->acquire(pPipe);
  EXPECT_TRUE( bSuccessAcquiredResource );
  if( bSuccessAcquiredResource ){
    pPipe->run();
    pPipe->stop();
    EXPECT_TRUE( pResourceManager->release(pPipe) );
  }

  pPipe->addFilterToTail( std::make_shared<DummyFilter>() );
  bSuccessAcquiredResource = pResourceManager->acquire(pPipe);
  EXPECT_FALSE( bSuccessAcquiredResource );
  if( bSuccessAcquiredResource ){
    pPipe->run();
    pPipe->stop();
    EXPECT_TRUE( pResourceManager->release(pPipe) );
  }

  pPipe->clearFilters();
  delete pPipe;

  pPipe = new PipeMultiThread();
  pPipe->addFilterToTail( std::make_shared<DummyFilter>() );

  bSuccessAcquiredResource = pResourceManager->acquire(pPipe);
  EXPECT_TRUE( bSuccessAcquiredResource );
  if( bSuccessAcquiredResource ){
    pPipe->run();
    pPipe->stop();
    EXPECT_TRUE( pResourceManager->release(pPipe) );
  }

  pPipe->addFilterToTail( std::make_shared<DummyFilter>() );
  bSuccessAcquiredResource = pResourceManager->acquire(pPipe);
  EXPECT_TRUE( bSuccessAcquiredResource );
  if( bSuccessAcquiredResource ){
    pPipe->run();
    pPipe->stop();
    EXPECT_TRUE( pResourceManager->release(pPipe) );
  }

  pPipe->addFilterToTail( std::make_shared<DummyFilter>() );
  bSuccessAcquiredResource = pResourceManager->acquire(pPipe);
  EXPECT_TRUE( bSuccessAcquiredResource );
  if( bSuccessAcquiredResource ){
    pPipe->run();
    pPipe->stop();
    EXPECT_TRUE( pResourceManager->release(pPipe) );
  }

  pPipe->addFilterToTail( std::make_shared<DummyFilter>() );
  bSuccessAcquiredResource = pResourceManager->acquire(pPipe);
  EXPECT_FALSE( bSuccessAcquiredResource );
  if( bSuccessAcquiredResource ){
    pPipe->run();
    pPipe->stop();
    EXPECT_TRUE( pResourceManager->release(pPipe) );
  }

  pPipe->clearFilters();
  delete pPipe;

  CpuResourceManager::admin_terminate();
  pResourceManager = nullptr;
}

TEST_F(TestCase_PipeAndFilter, testStrategy)
{
  class StrategyA : public IStrategy
  {
  public:
    StrategyA():IStrategy(){};
    virtual ~StrategyA(){};

    virtual bool canHandle(StrategyContext context){
      return true;
    }
    virtual bool execute(StrategyContext context){
      std::cout << "StrategyA is executed" << std::endl;
      return true;
    }
  };
  class StrategyB : public IStrategy
  {
  public:
    StrategyB():IStrategy(){};
    virtual ~StrategyB(){};

    virtual bool canHandle(StrategyContext context){
      return false;
    }
    virtual bool execute(StrategyContext context){
      std::cout << "StrategyB is executed" << std::endl;
      return true;
    }
  };
  Strategy strategy;
  StrategyContext context;
  strategy.registerStrategy( new StrategyB() );
  strategy.registerStrategy( new StrategyA() );
  EXPECT_TRUE( strategy.execute(context) );
}


TEST_F(TestCase_PipeAndFilter, testDynamicSignalFlow_AddNewPipeToPipeMixer)
{
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

TEST_F(TestCase_PipeAndFilter, testPowerManager)
{
  IPowerManager* pManager = PowerManager::getManager();

  IPowerManager::CALLBACK callback = [&](IPowerManager::POWERSTATE powerState){
    std::cout << "power state change: " << pManager->getPowerStateString( powerState ) << std::endl;
  };
  int callbackId = pManager->registerCallback( callback );

  PowerManagerPrimitive* pTestShim = dynamic_cast<PowerManagerPrimitive*>( pManager->getTestShim() );
  if( pTestShim ){
    pTestShim->setPowerState( IPowerManager::POWERSTATE::ACTIVE );
    pTestShim->setPowerState( IPowerManager::POWERSTATE::IDLE );
    pTestShim->setPowerState( IPowerManager::POWERSTATE::SUSPEND );
    pTestShim->setPowerState( IPowerManager::POWERSTATE::OFF );
  }

  pManager->unregisterCallback( callbackId );
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


int main(int argc, char **argv)
{
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
