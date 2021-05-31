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

#include <iostream>
#include <filesystem>
#include <chrono>

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
  Filter* pFilter4 = new Filter();

  Pipe* pPipe = new Pipe();
  pPipe->addFilterToTail(pFilter1);
  pPipe->dump();

  pPipe->addFilterToTail(pFilter2);
  pPipe->dump();

  pPipe->addFilterToHead(pFilter3);
  pPipe->dump();

  pPipe->addFilterAfterFilter(pFilter4, pFilter3);
  pPipe->dump();

  pPipe->clearFilters(); // delete filter instances also.
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

  std::this_thread::sleep_for(std::chrono::microseconds(1000));

  pPipe->stop();
  EXPECT_FALSE(pPipe->isRunning());

  pSink = pPipe->detachSink();
  EXPECT_NE(nullptr, pSink);
  pSink->dump();
  ISource* pSource = pPipe->detachSource();
  EXPECT_NE(nullptr, pSource);
  pPipe->dump();

  pPipe->clearFilters(); // delete filter instances also.

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
  std::this_thread::sleep_for(std::chrono::microseconds(1000));
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

  std::this_thread::sleep_for(std::chrono::microseconds(1000));

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
  pPipe1->clearFilters(); // delete filter instances also.
  pPipe2->clearFilters(); // delete filter instances also.

  delete pPipe2; pPipe2 = nullptr;
  delete pPipe1; pPipe1 = nullptr;

  delete pSink; pSink = nullptr;
  delete pSource; pSource = nullptr;
}

TEST_F(TestCase_PipeAndFilter, testPipeMultiThread)
{
  IPipe* pPipe = new PipeMultiThread();

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

  std::this_thread::sleep_for(std::chrono::microseconds(1000));

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

  pPipe->clearFilters(); // delete filter instances also.

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

  ISink* pSink2 = new Sink();
  AudioFormat::ChannelMapper chMap2;
  chMap2.insert( std::make_pair(AudioFormat::CH::L, AudioFormat::CH::R) ); // dst, src
  chMap2.insert( std::make_pair(AudioFormat::CH::R, AudioFormat::CH::R) ); // dst, src

  pMultiSink->addSink( pSink2, chMap2 );

  AudioBuffer buf( AudioFormat(), 256 );
  Source source;
  source.read( buf );
  pMultiSink->write( buf );

  std::cout << "multi sink's latency : " << pMultiSink->getLatencyUSec() << std::endl;
  pMultiSink->dump();
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
  IPipe* pStream1 = new Pipe();
  IPipe* pStream2 = new Pipe();

  PipeMixer* pPipeMixer = new PipeMixer();

  ISource* pSource1 = new Source();
  pStream1->attachSource( pSource1 );
  pStream1->attachSink( pPipeMixer->allocateSinkAdaptor() );
  pStream1->addFilterToTail( new FilterIncrement() );

  ISource* pSource2 = new Source();
  pStream2->attachSource( pSource2 );
  pStream2->attachSink( pPipeMixer->allocateSinkAdaptor() );
  pStream2->addFilterToTail( new FilterIncrement() );

  ISink* pSink = new Sink();
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
  ISink* pSink1 = pStream1->detachSink();
  EXPECT_NE(nullptr, pSink1);
  pPipeMixer->releaseSinkAdaptor( pSink1 );
  pSource1 = pStream1->detachSource();
  EXPECT_NE(nullptr, pSource1);
  delete pSource1; pSource1 = nullptr;
  pStream1->clearFilters();

  // finalize stream2, the source and the sink
  ISink* pSink2 = pStream2->detachSink();
  EXPECT_NE(nullptr, pSink2);
  pPipeMixer->releaseSinkAdaptor( pSink2 );
  pSource2 = pStream2->detachSource();
  EXPECT_NE(nullptr, pSource2);
  delete pSource2; pSource2 = nullptr;
  pStream2->clearFilters();

  // finalize pipemixer
  pSink = pPipeMixer->detachSink();
  EXPECT_NE(nullptr, pSink);
  pSink->dump();

  delete pStream1;    pStream1 = nullptr;
  delete pStream2;    pStream2 = nullptr;
  delete pSink;       pSink = nullptr;
  delete pPipeMixer;  pPipeMixer = nullptr;
}

TEST_F(TestCase_PipeAndFilter, testPipedSink)
{
  ISource* pSource = new Source();
  ISink* pActualSink = new Sink();
  pActualSink->setVolume(50.0);

  PipedSink* pPipedSink = new PipedSink();
  pPipedSink->attachSink( pActualSink );
  pPipedSink->addFilterToTail( new FilterIncrement() );

  IPipe* pPipe = new Pipe();
  pPipe->addFilterToTail( new FilterIncrement() );
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

  ISink* pActualSink2 = pPipedSink->detachSink();
  EXPECT_EQ( pActualSink, pActualSink2 );
  pPipedSink->clearFilters();

  ISink* pPipedSink2 = pPipe->detachSink();
  EXPECT_EQ( pPipedSink, pPipedSink2 );
  ISource* pSource2 = pPipe->detachSource();
  EXPECT_EQ( pSource, pSource2 );
  pPipe->clearFilters();

  delete pPipedSink; pPipedSink = nullptr;
  delete pActualSink; pActualSink = nullptr;
  delete pSource; pSource = nullptr;
}

TEST_F(TestCase_PipeAndFilter, testDecoder)
{
  std::vector<MediaParam> params;
  MediaParam param1("testKey1", "abc"); params.push_back(param1);
  MediaParam param2("testKey2", 3840);  params.push_back(param2);
  MediaParam param3("testKey3", true);  params.push_back(param3);
  IDecoder* pDecoder = new NullDecoder();
  pDecoder->configure(params);

  ISource* pSource = new Source();
  pDecoder->attachSource( pSource );
  ISource* pSourceAdaptor = pDecoder->allocateSourceAdaptor();
  ISink* pSink = new Sink();

  IPipe* pPipe = new Pipe();
  pPipe->attachSource( pSourceAdaptor );
  pPipe->attachSink( pSink );
  pPipe->addFilterToTail( new FilterIncrement() );

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

  ISource* pDetachedSource = pPipe->attachSource( pSourceAdaptor );
  EXPECT_NE( pDetachedSource, nullptr );
  EXPECT_EQ( pDetachedSource, pSourceAdaptor );
  pDecoder->releaseSourceAdaptor( pDetachedSource ); pSourceAdaptor = pDetachedSource = nullptr;

  ISink* pDetachedSink = pPipe->detachSink();
  EXPECT_NE( pDetachedSink, nullptr );
  EXPECT_EQ( pDetachedSink, pSink );
  delete pDetachedSink; pSink = pDetachedSink = nullptr;

  pPipe->clearFilters();
  delete pPipe; pPipe = nullptr;

  ISource* pDetachedDecoderSource = pDecoder->detachSource();
  EXPECT_NE( pDetachedDecoderSource, nullptr );
  EXPECT_EQ( pDetachedDecoderSource, pSource );
  delete pDetachedSource; pDetachedSource = nullptr;

  delete pDecoder; pDecoder = nullptr;
}

TEST_F(TestCase_PipeAndFilter, testPlayer)
{
  // player(source, decoder) --via source adaptor -> pipe(filter) -> sink
  ISource* pSource = new Source();
  ISink* pSink = new Sink();

  IDecoder* pDecoder = new NullDecoder();
  IPlayer* pPlayer = new Player();
  ISource* pSourceAdaptor = pPlayer->prepare( pSource, pDecoder ); // handover the source and decoder
  EXPECT_EQ( pPlayer->isReady(), true );

  IPipe* pPipe = new Pipe();
  pPipe->attachSource( pSourceAdaptor );
  pPipe->attachSink( pSink );
  pPipe->addFilterToTail( new FilterIncrement() );

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
  pPlayer->stop();
  pPipe->stop();
  std::cout << "stopped" << std::endl;

  pSink->dump();

  // detach source (=player's source adaptor) from pipe
  ISource* pDetachedSourceAdaptor = pPipe->detachSource();
  EXPECT_NE( pDetachedSourceAdaptor, nullptr );
  EXPECT_EQ( pDetachedSourceAdaptor, pSourceAdaptor );

  // release the Detached Source Adaptor & detach the decoder(=player)'s source from player. note that player terminate will delete the decoder instance
  ISource* pDetachedPlayerSource = pPlayer->terminate( pDetachedSourceAdaptor );
  EXPECT_NE( pDetachedPlayerSource, nullptr );
  EXPECT_EQ( pDetachedPlayerSource, pSource );
  delete pDetachedPlayerSource; pSource = pDetachedPlayerSource = nullptr;

  // detach sink from pipe
  ISink* pDetachedSink = pPipe->detachSink();
  EXPECT_NE( pDetachedSink, nullptr );
  EXPECT_EQ( pDetachedSink, pSink );
  delete pDetachedSink; pSink = pDetachedSink = nullptr;

  // clear (delete) the filters
  pPipe->clearFilters();

  // then dispose the pipe
  delete pPipe; pPipe = nullptr;
}

TEST_F(TestCase_PipeAndFilter, testEncoder)
{
  std::vector<MediaParam> params;
  MediaParam param1("testKey1", "abc"); params.push_back(param1);
  MediaParam param2("testKey2", 3840);  params.push_back(param2);
  MediaParam param3("testKey3", true);  params.push_back(param3);
  IEncoder* pEncoder = new NullEncoder();
  pEncoder->configure(params);

  ISource* pSource = new Source();
  ISink* pSink = new EncodedSink();
  IPipe* pPipe = new Pipe();
  pPipe->attachSource( pSource );
  pPipe->addFilterToTail( new FilterIncrement() );
  ISink* pSinkAdaptor = pEncoder->allocateSinkAdaptor();
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

  std::cout << "Pipe::stop" << std::endl;
  pPipe->stop();
  std::cout << "Encoder::stop" << std::endl;
  pEncoder->stop();
  std::cout << "Encoder::stopped" << std::endl;

  pSink->dump();

  // detach sink from encoder
  ISink* pDetachedSink = pEncoder->detachSink();
  EXPECT_NE( pDetachedSink, nullptr );
  EXPECT_EQ( pDetachedSink, pSink );
  delete pSink; pDetachedSink = pSink = nullptr;

  // detach sink adaptor from pipe (=release encoder's sink adaptor)
  ISink* pDetachedSinkEnc = pPipe->detachSink();
  EXPECT_NE( pDetachedSinkEnc, nullptr );
  EXPECT_EQ( pDetachedSinkEnc, pSinkAdaptor );
  pEncoder->releaseSinkAdaptor( pSinkAdaptor ); pSinkAdaptor = pDetachedSinkEnc = nullptr;

  // detach source from pipe
  ISource* pDetachedSource = pPipe->detachSource();
  EXPECT_NE( pDetachedSource, nullptr );
  EXPECT_EQ( pDetachedSource, pSource );
  delete pDetachedSource; pDetachedSource = pSource = nullptr;

  // clear filter in the pipe
  pPipe->clearFilters();

  // then clear pipe
  delete pPipe; pPipe = nullptr;

  // then clear encoder
  delete pEncoder; pEncoder = nullptr;
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
    IFilter* pFilter = FilterManager::newInstanceById( aPlugInId );
    EXPECT_NE( nullptr, pFilter );
    delete pFilter;
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
    ISource* pFilter = SourceManager::newInstanceById( aPlugInId );
    EXPECT_NE( nullptr, pFilter );
    delete pFilter;
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
    ISink* pFilter = SinkManager::newInstanceById( aPlugInId );
    EXPECT_NE( nullptr, pFilter );
    delete pFilter;
  }
  EXPECT_FALSE( SinkManager::newInstanceById( "hogehogehoge" ) );

  pManager->terminate();
}


TEST_F(TestCase_PipeAndFilter, testDelayFilter)
{
  ISource* pSource = new Source();
  ISink* pSink = new Sink();
  IPipe* pPipe = new Pipe();

  pPipe->attachSource(pSource);
  PerChannelDelayFilter::ChannelDelay channelDelay;
  channelDelay[AudioFormat::CH::L] = 0;
  channelDelay[AudioFormat::CH::R] = 20*120; // 20us = 1 sample delay @ 48KHz

  pPipe->addFilterToTail( new DelayFilter( AudioFormat(), 20*120 ) );
  pPipe->addFilterToTail( new PerChannelDelayFilter( AudioFormat(), channelDelay ) );
  pPipe->attachSink(pSink);

  pPipe->run();
  std::this_thread::sleep_for(std::chrono::microseconds(1000));

  pPipe->stop();

  EXPECT_EQ( pSink, pPipe->detachSink());
  EXPECT_EQ( pSource, pPipe->detachSource());
  pSink->dump();
  pPipe->clearFilters();

  delete pPipe; pPipe = nullptr;
  delete pSink; pSink = nullptr;
  delete pSource; pSource = nullptr;
}


TEST_F(TestCase_PipeAndFilter, testSinkCapture)
{
  ISource* pSource = new Source();
  ISink* pSink = dynamic_cast<ISink*>( new SinkCapture( new Sink() ) );
  ICapture* pCapture = dynamic_cast<ICapture*>(pSink);
  IPipe* pPipe = new Pipe();

  pPipe->attachSource( pSource );
  pPipe->addFilterToTail( new FilterIncrement() );
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

  delete pPipe; pPipe = nullptr;
  delete pSink; pSink = nullptr;
  delete pSource; pSource = nullptr;
}

TEST_F(TestCase_PipeAndFilter, testSinkInjector)
{
  ISource* pSource = new Source();
  ISink* pSink = dynamic_cast<ISink*>( new SinkInjector( new Sink() ) );
  IInjector* pInjector = dynamic_cast<IInjector*>(pSink);
  IPipe* pPipe = new Pipe();

  pPipe->attachSource( pSource );
  pPipe->addFilterToTail( new Filter() );
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

  delete pPipe; pPipe = nullptr;
  delete pSink; pSink = nullptr;
  delete pSource; pSource = nullptr;

}


TEST_F(TestCase_PipeAndFilter, testSourceCapture)
{
  ISource* pSource = dynamic_cast<ISource*>( new SourceCapture( new Source() ) );
  ISink* pSink = new Sink();
  ICapture* pCapture = dynamic_cast<ICapture*>(pSource);
  IPipe* pPipe = new Pipe();

  pPipe->attachSource( pSource );
  pPipe->addFilterToTail( new FilterIncrement() );
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

  delete pPipe; pPipe = nullptr;
  delete pSink; pSink = nullptr;
  delete pSource; pSource = nullptr;
}

TEST_F(TestCase_PipeAndFilter, testSourceInjector)
{
  ISource* pSource = dynamic_cast<ISource*>( new SourceInjector( new Source() ) );
  ISink* pSink = new Sink();
  IInjector* pInjector = dynamic_cast<IInjector*>(pSource);
  IPipe* pPipe = new Pipe();

  pPipe->attachSource( pSource );
  pPipe->addFilterToTail( new Filter() );
  pPipe->attachSink( pSink );

  // 1st step: non injected
  EXPECT_FALSE( pInjector->getInjectorEnabled() );
  pPipe->run();
  std::this_thread::sleep_for(std::chrono::microseconds(100));
  pPipe->stop();
  std::cout << "Sink dump(non injected):" << std::endl;
  pSink->dump();
  EXPECT_EQ( pSink, pPipe->detachSink());
  delete pSink;
  pSink = new Sink();
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

  delete pPipe; pPipe = nullptr;
  delete pSink; pSink = nullptr;
  delete pSource; pSource = nullptr;
}


TEST_F(TestCase_PipeAndFilter, testFilterCapture)
{
  ISource* pSource = new Source();
  ISink* pSink = new Sink();
  IPipe* pPipe = new Pipe();

  pPipe->attachSource( pSource );
  pPipe->attachSink( pSink );

  pPipe->addFilterToTail( new FilterIncrement() );
  IFilter* pFilter = new FilterCapture();
  pPipe->addFilterToTail( pFilter );
  ICapture* pCapture = dynamic_cast<ICapture*>(pFilter);

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

  delete pPipe; pPipe = nullptr;
  delete pSink; pSink = nullptr;
  delete pSource; pSource = nullptr;
}


TEST_F(TestCase_PipeAndFilter, testFilterInjector)
{
  ISource* pSource = new Source();
  ISink* pSink = new Sink();
  IPipe* pPipe = new Pipe();

  pPipe->attachSource( pSource );
  pPipe->attachSink( pSink );

  pPipe->addFilterToTail( new FilterIncrement() );
  IFilter* pFilter = new FilterInjector();
  pPipe->addFilterToTail( pFilter );
  IInjector* pInjector = dynamic_cast<IInjector*>(pFilter);

  // 1st step: non injected
  EXPECT_FALSE( pInjector->getInjectorEnabled() );
  pPipe->run();
  std::this_thread::sleep_for(std::chrono::microseconds(100));
  pPipe->stop();
  std::cout << "Sink dump(non injected):" << std::endl;
  pSink->dump();
  EXPECT_EQ( pSink, pPipe->detachSink());
  delete pSink;
  pSink = new Sink();
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

  delete pPipe; pPipe = nullptr;
  delete pSink; pSink = nullptr;
  delete pSource; pSource = nullptr;
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
  pPipe->addFilterToTail( new DummyFilter() );

  bool bSuccessAcquiredResource = pResourceManager->acquire(pPipe);
  EXPECT_TRUE( bSuccessAcquiredResource );
  if( bSuccessAcquiredResource ){
    pPipe->run();
    pPipe->stop();
    EXPECT_TRUE( pResourceManager->release(pPipe) );
  }

  pPipe->addFilterToTail( new DummyFilter() );
  bSuccessAcquiredResource = pResourceManager->acquire(pPipe);
  EXPECT_TRUE( bSuccessAcquiredResource );
  if( bSuccessAcquiredResource ){
    pPipe->run();
    pPipe->stop();
    EXPECT_TRUE( pResourceManager->release(pPipe) );
  }

  pPipe->addFilterToTail( new DummyFilter() );
  bSuccessAcquiredResource = pResourceManager->acquire(pPipe);
  EXPECT_TRUE( bSuccessAcquiredResource );
  if( bSuccessAcquiredResource ){
    pPipe->run();
    pPipe->stop();
    EXPECT_TRUE( pResourceManager->release(pPipe) );
  }

  pPipe->addFilterToTail( new DummyFilter() );
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
  pPipe->addFilterToTail( new DummyFilter() );

  bSuccessAcquiredResource = pResourceManager->acquire(pPipe);
  EXPECT_TRUE( bSuccessAcquiredResource );
  if( bSuccessAcquiredResource ){
    pPipe->run();
    pPipe->stop();
    EXPECT_TRUE( pResourceManager->release(pPipe) );
  }

  pPipe->addFilterToTail( new DummyFilter() );
  bSuccessAcquiredResource = pResourceManager->acquire(pPipe);
  EXPECT_TRUE( bSuccessAcquiredResource );
  if( bSuccessAcquiredResource ){
    pPipe->run();
    pPipe->stop();
    EXPECT_TRUE( pResourceManager->release(pPipe) );
  }

  pPipe->addFilterToTail( new DummyFilter() );
  bSuccessAcquiredResource = pResourceManager->acquire(pPipe);
  EXPECT_TRUE( bSuccessAcquiredResource );
  if( bSuccessAcquiredResource ){
    pPipe->run();
    pPipe->stop();
    EXPECT_TRUE( pResourceManager->release(pPipe) );
  }

  pPipe->addFilterToTail( new DummyFilter() );
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

TEST_F(TestCase_PipeAndFilter, testDynamicSignalFlow_AddNewPipe)
{
  PipeMixer* pPipeMixer = new PipeMixer();
  ISink* pSink = new Sink();
  pPipeMixer->attachSink( pSink );

  IPipe* pStream1 = new Pipe();
  ISource* pSource1 = new Source();
  pStream1->attachSource( pSource1 );
  pStream1->attachSink( pPipeMixer->allocateSinkAdaptor() );
  pStream1->addFilterToTail( new FilterIncrement() );

  std::cout << "start:mixer & stream1" << std::endl;
  pStream1->run();
  pPipeMixer->run();
  std::cout << "started" << std::endl;
  std::this_thread::sleep_for(std::chrono::microseconds(1000));

  std::cout << "add:stream2 during stream1 is running" << std::endl;
  IPipe* pStream2 = new Pipe();
  ISource* pSource2 = new Source();
  pStream2->attachSource( pSource2 );
  pStream2->attachSink( pPipeMixer->allocateSinkAdaptor() );
  std::cout << "added:stream2 during stream1 is running" << std::endl;
  pStream2->addFilterToTail( new FilterIncrement() );
  std::cout << "start:stream2 during stream1 is running" << std::endl;
  pStream2->run();
  std::cout << "started:stream2 during stream1 is running" << std::endl;
  std::this_thread::sleep_for(std::chrono::microseconds(1000));
  std::cout << "stop:stream2 during stream1 is running" << std::endl;
  pStream2->stop();
  std::cout << "stopped:stream2 during stream1 is running" << std::endl;
  // finalize stream2, the source and the sink
  ISink* pSink2 = pStream2->detachSink();
  EXPECT_NE(nullptr, pSink2);
  std::cout << "detach:stream2 during stream1 is running" << std::endl;
  pPipeMixer->releaseSinkAdaptor( pSink2 );
  pSource2 = pStream2->detachSource();
  EXPECT_NE(nullptr, pSource2);
  delete pSource2; pSource2 = nullptr;
  pStream2->clearFilters();
  std::this_thread::sleep_for(std::chrono::microseconds(1000));

  std::cout << "stop all" << std::endl;

  pPipeMixer->stop();
  pStream1->stop();
  EXPECT_FALSE(pPipeMixer->isRunning());
  std::cout << "stopped" << std::endl;

  // finalize stream1, the source and the sink
  ISink* pSink1 = pStream1->detachSink();
  EXPECT_NE(nullptr, pSink1);
  pPipeMixer->releaseSinkAdaptor( pSink1 );
  pSource1 = pStream1->detachSource();
  EXPECT_NE(nullptr, pSource1);
  delete pSource1; pSource1 = nullptr;
  pStream1->clearFilters();

  // finalize pipemixer
  pSink = pPipeMixer->detachSink();
  EXPECT_NE(nullptr, pSink);
  pSink->dump();

  delete pStream1;    pStream1 = nullptr;
  delete pStream2;    pStream2 = nullptr;
  delete pSink;       pSink = nullptr;
  delete pPipeMixer;  pPipeMixer = nullptr;
}

int main(int argc, char **argv)
{
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
