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
#include "TestCase_Output.hpp"

TestCase_Output::TestCase_Output()
{
}

TestCase_Output::~TestCase_Output()
{
}

void TestCase_Output::SetUp()
{
}

void TestCase_Output::TearDown()
{
}

TEST_F(TestCase_Output, testPipeSetupByCondition)
{
  std::shared_ptr<ParameterManager> pParams = ParameterManager::getManager().lock();

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

TEST_F(TestCase_Output, testOutputSinkSwitchLpcm)
{
  std::map<std::string, std::shared_ptr<ISink>> sinkManager = SinkFactory::getSinks();
  std::shared_ptr<PipedSink> pSpeakerSink = std::dynamic_pointer_cast<PipedSink>( sinkManager["speaker"] );
  pSpeakerSink->addFilterToTail( std::make_shared<SpeakerProtectionFilter>() );

  // --- start condition : speaker + spdif
  // stream 1: Source -> Pipe -> PipeMixer -> MultipleSink -> PipeSink(SpeakerProtection) -> Speaker
  // stream 2: Source -> Pipe ->                           -> EncodedSink -> Spdif
  std::shared_ptr<OutputManager> pMultiSink = std::make_shared<OutputManager>( sinkManager, "speaker", "spdif" );

  std::shared_ptr<PipeMixer> pPipeMixer = std::make_shared<PipeMixer>();
  pPipeMixer->attachSink( pMultiSink );
  pPipeMixer->run();

  // attaching stream1 to PipeMixer
  std::shared_ptr<IPipe> pStream1 = std::make_shared<Pipe>();
  pStream1->attachSource( std::make_shared<Source>() );
  pStream1->addFilterToTail( std::make_shared<FilterIncrement>());
  pStream1->attachSink( pPipeMixer->allocateSinkAdaptor(pStream1) );

  // attaching stream2 to PipeMixer
  std::shared_ptr<IPipe> pStream2 = std::make_shared<Pipe>();
  pStream2->attachSource( std::make_shared<Source>() );
  pStream2->addFilterToTail( std::make_shared<FilterIncrement>());
  pStream2->attachSink( pPipeMixer->allocateSinkAdaptor(pStream2) );

  pStream1->run();
  pStream2->run();
  std::this_thread::sleep_for(std::chrono::microseconds(1000));

  pMultiSink->dump();

  // --- switch to audio system + spdif
  // switch from:
  //  stream 1: Source -> Pipe -> PipeMixer -> MultipleSink -> PipeSink(SpeakerProtection) -> Speaker
  //  stream 2: Source -> Pipe ->                           -> EncodedSink -> Spdif
  // switch to:
  //  stream 1: Source -> Pipe -> PipeMixer -> MultipleSink -> EncodedSink -> Hdmi
  //  stream 2: Source -> Pipe ->                           -> EncodedSink -> Spdif
  pMultiSink->setPrimaryOutput("hdmi");

  std::this_thread::sleep_for(std::chrono::microseconds(1000));
  pMultiSink->dump();

  // --- switch to bluetooth + spdif
  // switch from:
  //  stream 1: Source -> Pipe -> PipeMixer -> MultipleSink -> EncodedSink -> Hdmi
  //  stream 2: Source -> Pipe ->                           -> EncodedSink -> Spdif
  // switch to:
  //  stream 1: Source -> Pipe -> PipeMixer -> MultipleSink -> Bluetooth
  //  stream 2: Source -> Pipe ->                           -> EncodedSink -> Spdif
  pMultiSink->setPrimaryOutput("bluetooth");

  std::this_thread::sleep_for(std::chrono::microseconds(1000));
  pMultiSink->dump();

  // TODO : convert the above to Stratgey and add several Strategy to switch sink
}

TEST_F(TestCase_Output, testOutputSinkSwitchCompressed)
{
  std::map<std::string, std::shared_ptr<ISink>> sinkManager = SinkFactory::getSinks();
  std::shared_ptr<PipedSink> pSpeakerSink = std::dynamic_pointer_cast<PipedSink>( sinkManager["speaker"] );
  pSpeakerSink->addFilterToTail( std::make_shared<SpeakerProtectionFilter>() );

  // --- start condition : speaker + spdif
  // stream 1: Source -> Pipe -> PipeMixer -> MultipleSink -> PipeSink(SpeakerProtection) -> Speaker
  // stream 2: Source -> Pipe ->                           -> EncodedSink -> Spdif
  std::shared_ptr<OutputManager> pMultiSink = std::make_shared<OutputManager>( sinkManager, "speaker", "spdif" );

  std::shared_ptr<PipeMixer> pPipeMixer = std::make_shared<PipeMixer>();
  pPipeMixer->attachSink( pMultiSink );
  pPipeMixer->run();

  // attaching stream1 to PipeMixer
  std::shared_ptr<IPipe> pStream1 = std::make_shared<Pipe>();
  pStream1->attachSource( std::make_shared<CompressedSource>() );
  pStream1->addFilterToTail( std::make_shared<FilterIncrement>());
  pStream1->attachSink( pPipeMixer->allocateSinkAdaptor(pStream1) );

  // attaching stream2 to PipeMixer
  std::shared_ptr<IPipe> pStream2 = std::make_shared<Pipe>();
  pStream2->attachSource( std::make_shared<Source>() );
  pStream2->addFilterToTail( std::make_shared<FilterIncrement>());
  pStream2->attachSink( pPipeMixer->allocateSinkAdaptor(pStream2) );

  pStream1->run();
  pStream2->run();
  std::this_thread::sleep_for(std::chrono::microseconds(1000));

  pMultiSink->dump();

  // --- switch to audio system + spdif
  // switch from:
  //  stream 1: Source -> Pipe -> PipeMixer -> MultipleSink -> PipeSink(SpeakerProtection) -> Speaker
  //  stream 2: Source -> Pipe ->                           -> EncodedSink -> Spdif
  // switch to:
  //  stream 1: Source -> Pipe -> PipeMixer -> MultipleSink -> EncodedSink -> Hdmi
  //  stream 2: Source -> Pipe ->                           -> EncodedSink -> Spdif
  pMultiSink->setPrimaryOutput("hdmi");

  std::this_thread::sleep_for(std::chrono::microseconds(1000));
  pMultiSink->dump();

  // --- switch to bluetooth + spdif
  // switch from:
  //  stream 1: Source -> Pipe -> PipeMixer -> MultipleSink -> EncodedSink -> Hdmi
  //  stream 2: Source -> Pipe ->                           -> EncodedSink -> Spdif
  // switch to:
  //  stream 1: Source -> Pipe -> PipeMixer -> MultipleSink -> Bluetooth
  //  stream 2: Source -> Pipe ->                           -> EncodedSink -> Spdif
  pMultiSink->setPrimaryOutput("bluetooth");

  std::this_thread::sleep_for(std::chrono::microseconds(1000));
  pMultiSink->dump();

  // TODO : convert the above to Stratgey and add several Strategy to switch sink
}

TEST_F(TestCase_Output, testPipeSetupAndSinkSwitch)
{
  // init
  std::shared_ptr<ParameterManager> pParams = ParameterManager::getManager().lock();

  VirtualizerA::ensureDefaultAssumption();
  VirtualizerB::ensureDefaultAssumption();

  // --- setup Sink
  // Instantiate Sink Manager
  std::map<std::string, std::shared_ptr<ISink>> sinkManager = SinkFactory::getSinks();

  // ensure SpeakerProtection Filter to SpeakerSink
  std::shared_ptr<PipedSink> pSpeakerSink = std::dynamic_pointer_cast<PipedSink>( sinkManager["speaker"] );
  pSpeakerSink->addFilterToTail( std::make_shared<SpeakerProtectionFilter>() );
  pSpeakerSink->run(); // since this is PipedSink.

  // ensure SpeakerSink and SPDIF sink
  std::shared_ptr<OutputManager> pPrimarySink = std::make_shared<OutputManager>( sinkManager, "speaker", "spdif" );

  // setup final mixer and splitter
  std::shared_ptr<MixerSplitter> pMixerSplitter = std::make_shared<MixerSplitter>();
  pMixerSplitter->attachSink( pPrimarySink ); // for Sepeaker+SPDIF, HDMI+SPDIF
  pMixerSplitter->attachSink( pSpeakerSink ); // for pass through's exceptional case such as accessibility

  std::cout << "case 1: Source1(AC3) -> Player(Decoder) --> Pipe(VirtualizerA) --> MixerSplitter --> OutputManager(Primary:Speaker(SPKProtection), Concurrent:SPDIF(transcoder:true))" << std::endl;
  std::cout << "player, virtualizerA filter, Sink" << std::endl;

  // Set up source
  std::shared_ptr<CompressedSource> pSource1 = std::make_shared<CompressedSource>();
  pSource1->setAudioFormat( AudioFormat::ENCODING::COMPRESSED_AC3 );

  // set up pipe with TunnelPlaybackStrategy
  std::shared_ptr<TunnelPlaybackStrategy> pStrategy1 = std::make_shared<TunnelPlaybackStrategy>();
  std::shared_ptr<TunnelPlaybackContext> pContext1 = std::make_shared<TunnelPlaybackContext>();
  std::shared_ptr<ISink> pSinkAdaptor1 = pMixerSplitter->allocateSinkAdaptor( AudioFormat(), pContext1->pPipe ); // but at this time, pPipe should be nullptr. Need to be solved.
  pContext1->pSource = pSource1;
  pContext1->pSink = pSinkAdaptor1;

  // set up MixerSplitter
  // TODO : expand to support this case: if main stream is compressed && the other streams are pcm && sink is hdmi, setup : the main -> hdmi+spdif, the others --> speaker
  pMixerSplitter->conditionalMap( pSinkAdaptor1, pPrimarySink, std::make_shared<MixerSplitter::MapAnyCompressedCondition>() );
  pMixerSplitter->conditionalMap( pSinkAdaptor1, pPrimarySink, std::make_shared<MixerSplitter::MapAnyPcmCondition>() );

  pPrimarySink->setAudioFormat(AudioFormat::ENCODING::PCM_16BIT);
  pParams->setParameterBool("sink.transcoder", false);
  pParams->setParameterBool("sink.passthrough", false);
  EXPECT_TRUE( pStrategy1->execute(pContext1) );
  pContext1->pPipe->dump();

  // run
  pContext1->pPipe->run();
  pMixerSplitter->run();
  std::this_thread::sleep_for(std::chrono::microseconds(1000));
  pMixerSplitter->stop();
  pContext1->pPipe->stop();
  pPrimarySink->dump();

  std::cout << "case 2: Source1(AC3) -> Pipe --> MixerSplitter --> OutputManager(Primary:hdmi, Concurrent:SPDIF(transcoder:true))" << std::endl;
  std::cout << "no player, no virtualizer filter in pipe" << std::endl;
  pPrimarySink->setPrimaryOutput("hdmi");
  pPrimarySink->setAudioFormat(AudioFormat::ENCODING::COMPRESSED_AC3);
  pParams->setParameterBool("sink.transcoder", true);
  pParams->setParameterBool("sink.passthrough", true);
  EXPECT_TRUE( pStrategy1->execute(pContext1) );
  pContext1->pPipe->dump();

  // run
  pContext1->pPipe->run();
  pMixerSplitter->run();
  std::this_thread::sleep_for(std::chrono::microseconds(1000));
  pMixerSplitter->stop();
  pContext1->pPipe->stop();
  pPrimarySink->dump();
}


TEST_F(TestCase_Output, testHqSpeakerSink)
{
  std::shared_ptr<IPipe> pPipe = std::make_shared<Pipe>();
  std::shared_ptr<ISource> pSource = std::make_shared<SinSource>();
  std::shared_ptr<ISink> pSink = std::make_shared<HQSpeakerSink>(true, AudioFormat(AudioFormat::ENCODING::PCM_32BIT));
  pSource->setAudioFormat( AudioFormat(AudioFormat::ENCODING::PCM_FLOAT) );
  pSink->setAudioFormat( AudioFormat(AudioFormat::ENCODING::PCM_FLOAT) );
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