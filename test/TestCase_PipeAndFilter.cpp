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

#include "TestCase_Common.hpp"
#include "TestCase_PipeAndFilter.hpp"

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
  std::shared_ptr<IStream> pStream = std::make_shared<FileStream>("test.bin");
  std::shared_ptr<StreamSink> pSink = std::make_shared<StreamSink>(AudioFormat(), pStream);
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
  std::shared_ptr<IStream> pStream = std::make_shared<FileStream>("test_32b96k.bin");
  std::shared_ptr<StreamSink> pSink = std::make_shared<StreamSink>(AudioFormat(AudioFormat::ENCODING::PCM_32BIT, AudioFormat::SAMPLING_RATE::SAMPLING_RATE_96_KHZ, AudioFormat::CHANNEL::CHANNEL_STEREO), pStream);
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
  std::shared_ptr<IStream> pStream = std::make_shared<FileStream>("test.bin");
  std::shared_ptr<StreamSource> pSource = std::make_shared<StreamSource>(AudioFormat(), pStream);
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

TEST_F(TestCase_PipeAndFilter, testMixerSplitter)
{
  // Signal flow
  //  Source1 -> Pipe1(->FilterIncrement->) -> |MixerSplitter | -> Sink
  //  Source2 -> Pipe2(->FilterIncrement->) -> |(mix here)    |

  std::shared_ptr<MixerSplitter> pMixerSplitter = std::make_shared<MixerSplitter>();
  std::shared_ptr<ISink> pSink1 = std::make_shared<LPcmSink>();
  std::shared_ptr<ISink> pSink2 = std::make_shared<CompressedSink>();
  pSink2->setAudioFormat( AudioFormat(AudioFormat::ENCODING::PCM_16BIT));
  pMixerSplitter->addSink( pSink1 );
  pMixerSplitter->addSink( pSink2 );

  std::shared_ptr<IPipe> pStream1 = std::make_shared<Pipe>();
  std::shared_ptr<ISource> pSource1 = std::make_shared<Source>();
  pStream1->attachSource( pSource1 );
  pStream1->addFilterToTail( std::make_shared<FilterIncrement>() );
  std::shared_ptr<ISink> pSinkAdaptor1 = pMixerSplitter->allocateSinkAdaptor( AudioFormat(), pStream1 );
  pStream1->attachSink( pSinkAdaptor1 );

  std::shared_ptr<IPipe> pStream2 = std::make_shared<Pipe>();
  std::shared_ptr<ISource> pSource2 = std::make_shared<Source>();
  pStream2->attachSource( pSource2 );
  pStream2->attachSink( pMixerSplitter->allocateSinkAdaptor( AudioFormat(), pStream2 ) );
  pStream2->addFilterToTail( std::make_shared<FilterIncrement>() );
  std::shared_ptr<ISink> pSinkAdaptor2 = pMixerSplitter->allocateSinkAdaptor( AudioFormat(), pStream2 );
  pStream2->attachSink( pSinkAdaptor2 );

  pMixerSplitter->map( pSinkAdaptor1, pSink1 );
  pMixerSplitter->map( pSinkAdaptor2, pSink1 );

  std::cout << "pStream1+pStream2=>pSink1" << std::endl;
  pStream1->run();
  pStream2->run();
  pMixerSplitter->run();

  std::this_thread::sleep_for(std::chrono::microseconds(1000));

  pMixerSplitter->stop();
  pStream1->stop();
  pStream2->stop();
  EXPECT_FALSE(pMixerSplitter->isRunning());

  pMixerSplitter->dump();

  EXPECT_TRUE( pMixerSplitter->unmap( pSinkAdaptor2 ) );
  pMixerSplitter->map( pSinkAdaptor2, pSink2 );

  std::cout << "pStream1=>pSink1, pStream2=>pSink2" << std::endl;
  pStream1->run();
  pStream2->run();
  pMixerSplitter->run();
  std::this_thread::sleep_for(std::chrono::microseconds(1000));

  pMixerSplitter->stop();
  pStream1->stop();
  pStream2->stop();
  EXPECT_FALSE(pMixerSplitter->isRunning());

  pMixerSplitter->dump();


  EXPECT_TRUE( pMixerSplitter->unmap( pSinkAdaptor2 ) );
  pMixerSplitter->conditionalMap( pSinkAdaptor2, pSink1, std::make_shared<MixerSplitter::MapAnyPcmCondition>() );
  pMixerSplitter->conditionalMap( pSinkAdaptor2, pSink1, std::make_shared<MixerSplitter::MapAnyCompressedCondition>() );

  std::cout << "pStream1+pStream2(if PCM)=>pSink1" << std::endl;
  std::cout << "pStream1=>pSink1, pStream2(if Compressed)=>pSink2" << std::endl;
  std::cout << "playback:pStream2 as PCM" << std::endl;

  pStream1->run();
  pStream2->run();
  pMixerSplitter->run();
  std::this_thread::sleep_for(std::chrono::microseconds(1000));

  pMixerSplitter->stop();
  pStream1->stop();
  pStream2->stop();
  EXPECT_FALSE(pMixerSplitter->isRunning());

  pMixerSplitter->dump();

  std::cout << "pStream1+pStream2(if PCM)=>pSink1" << std::endl;
  std::cout << "pStream1=>pSink1, pStream2(if Compressed)=>pSink2" << std::endl;
  std::cout << "playback:pStream2 as compressed" << std::endl;

  pStream2->attachSource( std::make_shared<CompressedSource>() );
  pSinkAdaptor2->setAudioFormat(AudioFormat(AudioFormat::ENCODING::COMPRESSED));
  pSink2->setAudioFormat(AudioFormat(AudioFormat::ENCODING::COMPRESSED));
  pStream2->clearFilters();

  pStream1->run();
  pStream2->run();
  pMixerSplitter->run();
  std::this_thread::sleep_for(std::chrono::microseconds(1000));

  pMixerSplitter->stop();
  pStream1->stop();
  pStream2->stop();
  EXPECT_FALSE(pMixerSplitter->isRunning());

  pMixerSplitter->dump();
}

TEST_F(TestCase_PipeAndFilter, testPatchPanel)
{
  std::cout << "--- case 1: Source-Sink 1:1" << std::endl;
  std::vector<std::shared_ptr<ISource>> pSources;
    pSources.push_back( std::make_shared<Source>() );
  std::vector<std::shared_ptr<ISink>> pSinks;
    pSinks.push_back( std::make_shared<Sink>() );

  std::cout << "createPatch" << std::endl;
  std::shared_ptr<PatchPanel> pPatchPanel = PatchPanel::createPatch( pSources, pSinks );

  std::cout << "get mixer splitter" << std::endl;
  std::shared_ptr<MixerSplitter> pMixerSplitter = pPatchPanel->getMixerSplitter();
  std::cout << "dump the mixer splitter" << std::endl;
  pMixerSplitter->dump();
  std::cout << "run the mixer splitter" << std::endl;
  pMixerSplitter->run();
  std::this_thread::sleep_for(std::chrono::microseconds(1000));
  std::cout << "stop the mixer splitter" << std::endl;
  pMixerSplitter->stop();
  std::cout << "dump the mixer splitter" << std::endl;
  pMixerSplitter->dump();
  std::cout << "finalize..." << std::endl;

  std::cout << "--- case 2: Source1,2-Sink 2:1" << std::endl;
  pSources.push_back( std::make_shared<Source>() );
  pPatchPanel->updatePatch( pSources, pSinks );

  std::cout << "dump the mixer splitter" << std::endl;
  pMixerSplitter->dump();
  std::cout << "run the mixer splitter" << std::endl;
  pMixerSplitter->run();
  std::this_thread::sleep_for(std::chrono::microseconds(1000));
  std::cout << "stop the mixer splitter" << std::endl;
  pMixerSplitter->stop();
  std::cout << "dump the mixer splitter" << std::endl;
  pMixerSplitter->dump();
  std::cout << "finalize..." << std::endl;

  std::cout << "--- case 3: Source1,2-Sink1,2 2:2" << std::endl;
  pSinks.push_back( std::make_shared<Sink>() );
  pPatchPanel->updatePatch( pSources, pSinks );

  std::cout << "dump the mixer splitter" << std::endl;
  pMixerSplitter->dump();
  std::cout << "run the mixer splitter" << std::endl;
  pMixerSplitter->run();
  std::this_thread::sleep_for(std::chrono::microseconds(1000));
  std::cout << "stop the mixer splitter" << std::endl;
  pMixerSplitter->stop();
  std::cout << "dump the mixer splitter" << std::endl;
  pMixerSplitter->dump();
  std::cout << "finalize..." << std::endl;

  std::cout << "--- case 3: Source1,2-Sink1 2:1" << std::endl;
  pSinks.resize(1);
  pPatchPanel->updatePatch( pSources, pSinks );

  std::cout << "dump the mixer splitter" << std::endl;
  pMixerSplitter->dump();
  std::cout << "run the mixer splitter" << std::endl;
  pMixerSplitter->run();
  std::this_thread::sleep_for(std::chrono::microseconds(1000));
  std::cout << "stop the mixer splitter" << std::endl;
  pMixerSplitter->stop();
  std::cout << "dump the mixer splitter" << std::endl;
  pMixerSplitter->dump();
  std::cout << "finalize..." << std::endl;

  std::cout << "--- case 4: Source1-Sink1 1:1" << std::endl;
  pSources.resize(1);
  pPatchPanel->updatePatch( pSources, pSinks );

  std::cout << "dump the mixer splitter" << std::endl;
  pMixerSplitter->dump();
  std::cout << "run the mixer splitter" << std::endl;
  pMixerSplitter->run();
  std::this_thread::sleep_for(std::chrono::microseconds(1000));
  std::cout << "stop the mixer splitter" << std::endl;
  pMixerSplitter->stop();
  std::cout << "dump the mixer splitter" << std::endl;
  pMixerSplitter->dump();
  std::cout << "finalize..." << std::endl;
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

TEST_F(TestCase_PipeAndFilter, testChannelDemuxMux)
{
  std::shared_ptr<AudioBuffer> pSrcBuf = std::make_shared<AudioBuffer>( AudioFormat(AudioFormat::ENCODING::PCM_16BIT, AudioFormat::SAMPLING_RATE::SAMPLING_RATE_48_KHZ, AudioFormat::CHANNEL::CHANNEL_7_1CH ), 256 );
  uint8_t* pRawSrcBuffer = pSrcBuf->getRawBufferPointer();
  for( int i=0; i<pSrcBuf->getRawBuffer().size(); i++ ){
    *pRawSrcBuffer++ = (i % 256);
  }
  Util::dumpBuffer( "source buffer", pSrcBuf.get() );
  std::vector<std::shared_ptr<AudioBuffer>> pPerChannelBufs = ChannelDemuxer::perChannelDemux(pSrcBuf);
  int i=0;
  for( auto& pBuf : pPerChannelBufs ){
    std::string channel = "Channel:" + std::to_string(i++);
    Util::dumpBuffer( channel, pBuf.get() );
  }

  std::shared_ptr<AudioBuffer> pMuxedBuf = ChannelMuxer::perChannelMux(pPerChannelBufs, AudioFormat::CHANNEL::CHANNEL_7_1CH);
  EXPECT_TRUE( pSrcBuf->getAudioFormat().equal( pMuxedBuf->getAudioFormat() ) );
  Util::dumpBuffer( "MuxedBuf", pMuxedBuf.get() );

  uint8_t* pRawSrcBuf = pSrcBuf->getRawBufferPointer();
  uint8_t* pRawDstBuf = pMuxedBuf->getRawBufferPointer();
  int nSrcRawSize = pSrcBuf->getNumberOfSamples();
  int nDstRawSize = pMuxedBuf->getNumberOfSamples();
  EXPECT_EQ( nSrcRawSize, nDstRawSize );
  for(int i=0; i<nSrcRawSize && i<nDstRawSize; i++){
    EXPECT_EQ( *pRawSrcBuf++, *pRawDstBuf++ );
  }
}

TEST_F(TestCase_PipeAndFilter, testChannelDemuxMux2)
{
  std::shared_ptr<AudioBuffer> pSrcBuf = std::make_shared<AudioBuffer>( AudioFormat(AudioFormat::ENCODING::PCM_16BIT, AudioFormat::SAMPLING_RATE::SAMPLING_RATE_48_KHZ, AudioFormat::CHANNEL::CHANNEL_7_1CH ), 256 );
  uint8_t* pRawSrcBuffer = pSrcBuf->getRawBufferPointer();
  for( int i=0; i<pSrcBuf->getRawBuffer().size(); i++ ){
    *pRawSrcBuffer++ = (i % 256);
  }
  Util::dumpBuffer( "source buffer", pSrcBuf.get() );

  std::vector<std::vector<AudioFormat::CH>> channels;
  {
    std::vector<AudioFormat::CH> channel;
    channel.push_back( AudioFormat::CH::L );
    channel.push_back( AudioFormat::CH::R );
    channel.push_back( AudioFormat::CH::SL );
    channel.push_back( AudioFormat::CH::SR );
    channels.push_back( channel );
  }
  {
    std::vector<AudioFormat::CH> channel;
    channel.push_back( AudioFormat::CH::C );
    channels.push_back( channel );
  }
  {
    std::vector<AudioFormat::CH> channel;
    channel.push_back( AudioFormat::CH::SW );
    channels.push_back( channel );
  }
  {
    std::vector<AudioFormat::CH> channel;
    channel.push_back( AudioFormat::CH::SBL );
    channel.push_back( AudioFormat::CH::SBR );
    channels.push_back( channel );
  }
  std::vector<std::shared_ptr<AudioBuffer>> pPerChannelBufs = ChannelDemuxer::perChannelDemux(pSrcBuf, channels);
  int i=0;
  for( auto& pBuf : pPerChannelBufs ){
    std::string channel = pBuf->getAudioFormat().toString() + " Channel:" + std::to_string(i++);
    Util::dumpBuffer( channel, pBuf.get() );
  }

  std::shared_ptr<AudioBuffer> pMuxedBuf = ChannelMuxer::perChannelMux(pPerChannelBufs, channels, AudioFormat::CHANNEL::CHANNEL_7_1CH);
  EXPECT_TRUE( pSrcBuf->getAudioFormat().equal( pMuxedBuf->getAudioFormat() ) );
  Util::dumpBuffer( "MuxedBuf", pMuxedBuf.get() );

  uint8_t* pRawSrcBuf = pSrcBuf->getRawBufferPointer();
  uint8_t* pRawDstBuf = pMuxedBuf->getRawBufferPointer();
  int nSrcRawSize = pSrcBuf->getNumberOfSamples();
  int nDstRawSize = pMuxedBuf->getNumberOfSamples();
  EXPECT_EQ( nSrcRawSize, nDstRawSize );
  for(int i=0; i<nSrcRawSize && i<nDstRawSize; i++){
    EXPECT_EQ( *pRawSrcBuf++, *pRawDstBuf++ );
  }
}

TEST_F(TestCase_PipeAndFilter, testAudioBaseFormatChanged)
{
  class MyAudioFormatListener : public AudioBase::AudioFormatListener
  {
  public:
    AudioFormat mFormat;
    MyAudioFormatListener(){};
    virtual ~MyAudioFormatListener(){};
    virtual void onFormatChanged(AudioFormat format){
      std::cout << "MyAudioFormatListener::MyAudioFormatListener" << format.toString() << std::endl;
      mFormat = format;
    };
  };

  std::shared_ptr<ISink> pSink = std::make_shared<CompressedSink>();
  std::shared_ptr<MyAudioFormatListener> pMyListener = std::make_shared<MyAudioFormatListener>();
  pSink->registerAudioFormatListener( pMyListener );

  EXPECT_TRUE( pSink->setAudioFormat(AudioFormat::ENCODING::COMPRESSED_AAC) );
  EXPECT_TRUE( pSink->getAudioFormat().equal( pMyListener->mFormat ) );
}

TEST_F(TestCase_PipeAndFilter, testFilterExample16)
{
  std::shared_ptr<IPipe> pPipe = std::make_shared<Pipe>();
  std::shared_ptr<ISource> pSource = std::make_shared<TestSource>();
  std::shared_ptr<ISink> pSink = std::make_shared<TestSink>();
  pSource->setAudioFormat( AudioFormat(AudioFormat::ENCODING::PCM_16BIT) );
  pSink->setAudioFormat( AudioFormat(AudioFormat::ENCODING::PCM_16BIT) );
  pPipe->attachSource( pSource );
  pPipe->attachSink( pSink );
  pPipe->addFilterToTail( std::make_shared<FilterReverb>() );

  std::shared_ptr<ParameterManager> pParams = ParameterManager::getManager().lock();
  pParams->setParameterFloat("filter.exampleReverb.delay", 0.5f);
  pParams->setParameterFloat("filter.exampleReverb.power", 0.01f);

  pPipe->run();
  std::this_thread::sleep_for(std::chrono::microseconds(1000));
  pPipe->stop();
  pPipe->detachSink()->dump();
}

TEST_F(TestCase_PipeAndFilter, testFilterExample32)
{
  std::shared_ptr<IPipe> pPipe = std::make_shared<Pipe>();
  std::shared_ptr<ISource> pSource = std::make_shared<TestSource>();
  std::shared_ptr<ISink> pSink = std::make_shared<TestSink>();
  pSource->setAudioFormat( AudioFormat(AudioFormat::ENCODING::PCM_32BIT) );
  pSink->setAudioFormat( AudioFormat(AudioFormat::ENCODING::PCM_32BIT) );
  pPipe->attachSource( pSource );
  pPipe->attachSink( pSink );
  pPipe->addFilterToTail( std::make_shared<FilterReverb>() );

  std::shared_ptr<ParameterManager> pParams = ParameterManager::getManager().lock();
  pParams->setParameterFloat("filter.exampleReverb.delay", 0.5f);
  pParams->setParameterFloat("filter.exampleReverb.power", 0.01f);

  pPipe->run();
  std::this_thread::sleep_for(std::chrono::microseconds(1000));
  pPipe->stop();
  pPipe->detachSink()->dump();
}

TEST_F(TestCase_PipeAndFilter, testSinSource)
{
  std::shared_ptr<IPipe> pPipe = std::make_shared<Pipe>();
  std::shared_ptr<ISource> pSource = std::make_shared<SinSource>();
  std::shared_ptr<ISink> pSink = std::make_shared<TestSink>();
  pSource->setAudioFormat( AudioFormat(AudioFormat::ENCODING::PCM_8BIT) );
  pSink->setAudioFormat( AudioFormat(AudioFormat::ENCODING::PCM_8BIT) );
  pPipe->attachSource( pSource );
  pPipe->attachSink( pSink );
  pPipe->addFilterToTail( std::make_shared<FilterReverb>() );

  std::shared_ptr<ParameterManager> pParams = ParameterManager::getManager().lock();
  pParams->setParameterFloat("filter.exampleReverb.delay", 0.0f);
  pParams->setParameterFloat("filter.exampleReverb.power", 0.0f);

  pPipe->run();
  std::this_thread::sleep_for(std::chrono::microseconds(1000));
  pPipe->stop();
  pPipe->detachSink()->dump();
}

TEST_F(TestCase_PipeAndFilter, testPipeThroughputPerformace)
{
  class PerfSource : public Source, public IPerformanceMeasurement
  {
  protected:
    virtual void readPrimitive(IAudioBuffer& buf){
      int nSize = buf.getRawBuffer().size();
      ByteBuffer zeroBuf( nSize, 0 );
      buf.setRawBuffer( zeroBuf );
      updateAdd( nSize );
    }
  public:
    PerfSource():Source(), IPerformanceMeasurement(){};
    virtual ~PerfSource(){};
  };

  class PerfSink : public Sink, public IPerformanceMeasurement
  {
  protected:
    virtual void writePrimitive(IAudioBuffer& buf){
      int nSize = buf.getRawBuffer().size();
      updateAdd( nSize );
    }
  public:
    PerfSink():Sink(), IPerformanceMeasurement(){};
    virtual ~PerfSink(){};
  };

  std::shared_ptr<IPipe> pPipe = std::make_shared<Pipe>();
  std::shared_ptr<PerfSource> pSource = std::make_shared<PerfSource>();
  std::shared_ptr<PerfSink> pSink = std::make_shared<PerfSink>();
  std::shared_ptr<WindowSizeVariableFilter> pFilter = std::make_shared<WindowSizeVariableFilter>();

  pPipe->attachSource( pSource );
  pPipe->attachSink( pSink );
  pPipe->addFilterToTail( pFilter );

  std::vector<int> testWindowSizes = {5000, 10000};
  for(auto& aWindowSize : testWindowSizes){
    pFilter->setWindowSizeUsec( aWindowSize );
    std::cout << std::endl << "test window size : " << aWindowSize << " pipe's report:" << pPipe->getWindowSizeUsec() << std::endl;
    pSource->reset();
    pSink->reset();
    pPipe->run();
    std::this_thread::sleep_for(std::chrono::microseconds(1000000)); // run 1 sec!
    pPipe->stop();
    double sourcePerf = pSource->getCountPerTime();
    double sinkPerf = pSink->getCountPerTime();
    std::cout << "source performance : " << sourcePerf/1000000 << "Mbytes/second" << std::endl;
    std::cout << "sink   performance : " << sinkPerf/1000000   << "Mbytes/second" << std::endl;
  }
}

class LatencyPerfSource : public Source, public IPerformanceMeasurement
{
protected:
  bool mbFirstTime;
  virtual void readPrimitive(IAudioBuffer& buf){
    int nSize = buf.getRawBuffer().size();
    ByteBuffer zeroBuf( nSize, 0 );
    buf.setRawBuffer( zeroBuf );
    if( !mbFirstTime ){
      update( nSize );
      mbFirstTime = true;
    }
  }
public:
  LatencyPerfSource():Source(), IPerformanceMeasurement(), mbFirstTime(false){};
  virtual ~LatencyPerfSource(){};
};

class LatencyPerfSink : public Sink, public IPerformanceMeasurement
{
protected:
  bool mbFirstTime;
  virtual void writePrimitive(IAudioBuffer& buf){
    int nSize = buf.getRawBuffer().size();
    if( !mbFirstTime ){
      update( nSize );
      mbFirstTime = true;
    }
  }
public:
  LatencyPerfSink():Sink(), IPerformanceMeasurement(), mbFirstTime(false){};
  virtual ~LatencyPerfSink(){};
};

TEST_F(TestCase_PipeAndFilter, testPipeLatencyPerformace)
{
  std::shared_ptr<IPipe> pPipe = std::make_shared<Pipe>();
  std::shared_ptr<LatencyPerfSource> pSource = std::make_shared<LatencyPerfSource>();
  std::shared_ptr<LatencyPerfSink> pSink = std::make_shared<LatencyPerfSink>();
  std::shared_ptr<WindowSizeVariableFilter> pFilter1 = std::make_shared<WindowSizeVariableFilter>();
  std::shared_ptr<WindowSizeVariableFilter> pFilter2 = std::make_shared<WindowSizeVariableFilter>();
  pFilter1->setWindowSizeUsec( 5000 );
  pFilter2->setWindowSizeUsec( 10000 );

  pPipe->attachSource( pSource );
  pPipe->attachSink( pSink );
  pPipe->addFilterToTail( pFilter1 );
  pPipe->addFilterToTail( pFilter2 );

  std::cout << std::endl << "pipe's report:" << pPipe->getWindowSizeUsec() << std::endl;
  pSource->reset();
  pSink->reset();
  pPipe->run();
  std::this_thread::sleep_for(std::chrono::microseconds(100000)); // run 100msec
  pPipe->stop();
  double latency = (double)(std::chrono::duration_cast<std::chrono::nanoseconds>(pSink->getLastTime() - pSource->getLastTime()).count());
  std::cout << "latency from Source to Sink : " << latency/1000   << "usec" << std::endl;
}

TEST_F(TestCase_PipeAndFilter, testPipeMtLatencyPerformace)
{
  std::shared_ptr<IPipe> pPipe = std::make_shared<PipeMultiThread>();
  std::shared_ptr<LatencyPerfSource> pSource = std::make_shared<LatencyPerfSource>();
  std::shared_ptr<LatencyPerfSink> pSink = std::make_shared<LatencyPerfSink>();
  std::shared_ptr<WindowSizeVariableFilter> pFilter1 = std::make_shared<WindowSizeVariableFilter>();
  std::shared_ptr<WindowSizeVariableFilter> pFilter2 = std::make_shared<WindowSizeVariableFilter>();
  pFilter1->setWindowSizeUsec( 5000 );
  pFilter2->setWindowSizeUsec( 10000 );

  pPipe->attachSource( pSource );
  pPipe->attachSink( pSink );
  pPipe->addFilterToTail( pFilter1 );
  pPipe->addFilterToTail( pFilter2 );

  std::cout << std::endl << "pipe's report:" << pPipe->getWindowSizeUsec() << std::endl;
  pSource->reset();
  pSink->reset();
  pPipe->run();
  std::this_thread::sleep_for(std::chrono::microseconds(100000)); // run 100msec
  pPipe->stop();
  double latency = (double)(std::chrono::duration_cast<std::chrono::nanoseconds>(pSink->getLastTime() - pSource->getLastTime()).count());
  std::cout << "latency from Source to Sink : " << latency/1000   << "usec" << std::endl;
}