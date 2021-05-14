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
#include "Stream.hpp"
#include "StreamSink.hpp"
#include "StreamSource.hpp"
#include "PipeMixer.hpp"
#include "PipedSink.hpp"
#include "Decoder.hpp"
#include "Player.hpp"
#include "ParameterManager.hpp"
#include "Util.hpp"

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

  std::cout << "source PTS: " << pSource->getSourcePts() << std::endl;
  std::cout << "actual sink PTS: " << pActualSink->getSinkPts() << std::endl;
  std::this_thread::sleep_for(std::chrono::microseconds(500));
  std::cout << "source PTS: " << pSource->getSourcePts() << std::endl;
  std::cout << "actual sink PTS: " << pActualSink->getSinkPts() << std::endl;

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
  std::vector<DecoderParam> params;
  DecoderParam param1("testKey1", "abc"); params.push_back(param1);
  DecoderParam param2("testKey2", 3840);  params.push_back(param2);
  DecoderParam param3("testKey3", true);  params.push_back(param3);
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

  std::cout << "source PTS: " << pSource->getSourcePts() << std::endl;
  std::cout << "sink PTS: " << pSink->getSinkPts() << std::endl;
  std::this_thread::sleep_for(std::chrono::microseconds(1000));
  std::cout << "source PTS: " << pSource->getSourcePts() << std::endl;
  std::cout << "sink PTS: " << pSink->getSinkPts() << std::endl;

  pPipe->stop();
  pDecoder->stop();

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
  std::cout << "sink PTS: " << pSink->getSinkPts() << std::endl;
  std::this_thread::sleep_for(std::chrono::microseconds(1000));
  std::cout << "sink PTS: " << pSink->getSinkPts() << std::endl;

  pPlayer->pause();
  std::this_thread::sleep_for(std::chrono::microseconds(1000));
  pPlayer->resume();
  std::this_thread::sleep_for(std::chrono::microseconds(1000));

  pPlayer->stop();
  pPipe->stop();

  pSink->dump();
  ISource* pDetachedSource = pPipe->attachSource( pSourceAdaptor );
  EXPECT_NE( pDetachedSource, nullptr );
  EXPECT_EQ( pDetachedSource, pSourceAdaptor );

  ISource* pDetachedPlayerSource = pPlayer->terminate( pDetachedSource );
  EXPECT_NE( pDetachedPlayerSource, nullptr );
  EXPECT_EQ( pDetachedPlayerSource, pSource );
  delete pDetachedPlayerSource; pSource = pDetachedPlayerSource = nullptr;

  ISink* pDetachedSink = pPipe->detachSink();
  EXPECT_NE( pDetachedSink, nullptr );
  EXPECT_EQ( pDetachedSink, pSink );
  delete pDetachedSink; pSink = pDetachedSink = nullptr;

  pPipe->clearFilters();
  delete pPipe; pPipe = nullptr;
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
  std::vector<ParameterManager::Param> paramsAll = pParams->getParameters();
  for(auto& aParam : paramsAll){
    std::cout << aParam.key << " = " << aParam.value << std::endl;
  }
}


int main(int argc, char **argv)
{
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
