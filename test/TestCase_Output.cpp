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
