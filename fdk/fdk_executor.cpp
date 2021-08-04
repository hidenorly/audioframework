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

#include <iostream>
#include <string>
#include "Util.hpp"
#include "Filter.hpp"
#include "Pipe.hpp"
#include "Stream.hpp"
#include "StreamSource.hpp"
#include "StreamSink.hpp"
#include "PcmSourceSink.hpp"
#include "OptParse.hpp"
#include "StringTokenizer.hpp"
#include "ParameterManager.hpp"
#include "Media.hpp"
#include "Decoder.hpp"
#include <filesystem>

AudioFormat getAudioFormatFromOpts( std::string encoding, std::string samplingRate, std::string channels )
{
  AudioFormat::ENCODING _encoding = AudioFormat::getEncodingFromString(encoding);
  int _samplingRate = std::stoi(samplingRate);
  AudioFormat::CHANNEL _channel = AudioFormat::getChannelsFromString(channels);

  return AudioFormat(_encoding, _samplingRate, _channel);
}

class PassThroughFilter : public Filter
{
protected:
  std::vector<AudioFormat> mSupportedFormats;
public:
  PassThroughFilter(){
    for(int anEncoding = AudioFormat::ENCODING::PCM_8BIT; anEncoding < AudioFormat::ENCODING::COMPRESSED_UNKNOWN; anEncoding++){
      for( int aChannel = AudioFormat::CHANNEL::CHANNEL_MONO; aChannel < AudioFormat::CHANNEL::CHANNEL_UNKNOWN; aChannel++){
        mSupportedFormats.push_back( AudioFormat((AudioFormat::ENCODING)anEncoding, 48000, (AudioFormat::CHANNEL)aChannel) );
        mSupportedFormats.push_back( AudioFormat((AudioFormat::ENCODING)anEncoding, 96000, (AudioFormat::CHANNEL)aChannel) );
      }
    }
  };
  virtual ~PassThroughFilter(){};
  virtual bool isAvailableFormat(AudioFormat format){ return true; };
  virtual std::vector<AudioFormat> getSupportedAudioFormats(void){ return mSupportedFormats; };
  virtual void process(AudioBuffer& inBuf, AudioBuffer& outBuf){ outBuf.setRawBuffer(inBuf.getRawBuffer()); };
  virtual std::string toString(void){ return "PassThroughFilter"; };
};


int main(int argc, char **argv)
{
  std::vector<OptParse::OptParseItem> options;
  options.push_back( OptParse::OptParseItem("-r", "--sampleRate", true, "48000", "Set Sampling Rate"));
  options.push_back( OptParse::OptParseItem("-e", "--encoding", true, "PCM_16BIT", "Set Encoding PCM_8BIT, PCM_16BIT, PCM_24BIT, PCM_32BIT, PCM_FLOAT"));
  options.push_back( OptParse::OptParseItem("-c", "--channel", true, "2", "Set channel 2, 2.1, 4, 4.1, 5, 5.1, 5.1.2, 7.1"));
  options.push_back( OptParse::OptParseItem("-f", "--filter", true, "", "Specify filter.so (dylib)"));
  options.push_back( OptParse::OptParseItem("-p", "--parameters", true, "", "Specify parameters (filter.paramA=0.2;filter.paramB=true)"));
  options.push_back( OptParse::OptParseItem("-i", "--input", true, "", "Specify input(source) file"));
  options.push_back( OptParse::OptParseItem("-o", "--output", true, "", "Specify output(sink) file (prioritized than -s)"));
  options.push_back( OptParse::OptParseItem("-s", "--sink", true, "", "Specify sink.so (dylib)"));
  options.push_back( OptParse::OptParseItem("-u", "--source", true, "", "Specify source.so (dylib)"));
  options.push_back( OptParse::OptParseItem("-d", "--decoder", true, "", "Specify decoder and input format, e.g. decoder.so,COMPRESSED_0"));
  options.push_back( OptParse::OptParseItem("-t", "--threadduration", true, "1000", "Specify execution time (usec), e.g. 1000"));

  std::filesystem::path fdkPath = argv[0];
  OptParse optParser( argc, argv, options, std::string("Filter executor e.g. ")+std::string(fdkPath.filename())+std::string(" -f lib/filter-plugin/libfilter_example.so") );

  SourceManager* pSourceManager = nullptr;
  SinkManager* pSinkManager = nullptr;
  FilterManager* pFilterManager = nullptr;
  MediaCodecManager* pCodecManager = nullptr;
  std::shared_ptr<ThreadBase::RunnerListener> pPipeRunnerListener;

  std::shared_ptr<IPipe> pPipe = std::make_shared<Pipe>();

  // setup filter
  std::shared_ptr<IFilter> pFilter;
  if( !optParser.values["-f"].empty() ){
    FilterManager::setPlugInPath(optParser.values["-f"]);
    pFilterManager = FilterManager::getInstance();
    pFilterManager->initialize();

    std::vector<std::string> plugInIds = pFilterManager->getPlugInIds();
    for(auto& aPlugInId : plugInIds){
  //    pFilter = FilterManager::newInstanceById( aPlugInId );
      pFilter = std::dynamic_pointer_cast<IFilter>( pFilterManager->getPlugIn( aPlugInId ) );
    }
  }
  if( pFilter ){
    pPipe->addFilterToTail( pFilter );
    pFilter.reset();
  } else {
    pPipe->addFilterToTail( std::make_shared<PassThroughFilter>() );
  }

  // setup audio format
  AudioFormat format = getAudioFormatFromOpts( optParser.values["-e"], optParser.values["-r"], optParser.values["-c"] );
  std::cout << "Specified audio format : " << format.toString() << std::endl;

  // setup source
  std::shared_ptr<ISource> pSource;
  if( std::filesystem::exists( optParser.values["-i"] ) ){
    std::shared_ptr<FileStream> pStream = std::make_shared<FileStream>(optParser.values["-i"]);
    pSource = std::make_shared<StreamSource>(format, pStream);
  } else {
    if( !optParser.values["-u"].empty() ){
      SourceManager::setPlugInPath(optParser.values["-u"]);
      pSourceManager = SourceManager::getInstance();
      pSourceManager->initialize();

      std::vector<std::string> plugInIds = pSourceManager->getPlugInIds();
      for(auto& aPlugInId : plugInIds){
        pSource = std::dynamic_pointer_cast<ISource>( pSourceManager->getPlugIn( aPlugInId ) );
        if( pSource ){
          break;
        }
      }
    }
    if( !pSource ){
      pSource = std::make_shared<PcmSource>();
    }
  }
  pSource->setAudioFormat( format );

  // setup decoder & source (remaining)
  if( !optParser.values["-d"].empty() ){
    StringTokenizer tok( optParser.values["-d"], "," );
    std::string plugInPath;
    if( tok.hasNext() ){
      plugInPath = StringUtil::trim( tok.getNext() );
    } else {
      plugInPath = optParser.values["-d"];
    }
    std::shared_ptr<AudioFormat> pDecoderSourceFormat;
    if( tok.hasNext() ){
      pDecoderSourceFormat = std::make_shared<AudioFormat>( AudioFormat::getEncodingFromString( tok.getNext() ));
    }
    if( !pDecoderSourceFormat ){
      std::make_shared<AudioFormat>( AudioFormat::ENCODING::COMPRESSED );
    }
    pSource->setAudioFormat( *pDecoderSourceFormat );
    std::shared_ptr<IDecoder> pDecoder;
    MediaCodecManager::setPlugInPath(plugInPath);
    pCodecManager = MediaCodecManager::getInstance();
    pCodecManager->initialize();
    std::vector<std::string> plugInIds = pCodecManager->getPlugInIds();
    for(auto& aPlugInId : plugInIds){
      pDecoder = std::dynamic_pointer_cast<IDecoder>( pCodecManager->getPlugIn( aPlugInId ) );
      if( pDecoder ){
        break;
      }
    }
    class PipeRunnerListener : public ThreadBase::RunnerListener
    {
    public:
      std::shared_ptr<IDecoder> pDecoder;
      PipeRunnerListener(std::shared_ptr<IDecoder> pDecoder = nullptr):pDecoder(pDecoder){};
      virtual ~PipeRunnerListener(){
        if( pDecoder ){
          pDecoder->stop();
          pDecoder.reset();
        }
      };
      virtual void onRunnerStatusChanged(bool bRunning){
        if( bRunning ){
          pDecoder->run();
        } else {
          pDecoder->stop();
        }
      };
    };
    if( pSource == nullptr ){
      std::cout << "Unexpected error. pSource == nullptr" << std::endl;
      exit(-1);
    }
    std::cout << "Decoder source format: " << pDecoderSourceFormat->toString() << std::endl;
    if( !pDecoder->canHandle( *pDecoderSourceFormat ) ){
      std::cout << "Warning: " << pDecoder->toString() << ": canHandle() returns false" << std::endl;
    }
    pDecoder->attachSource( pSource );
    std::cout << "Source for decoder:" << pSource->toString() << std::endl;
    std::shared_ptr<ISource> pSourceAdaptor = pDecoder->allocateSourceAdaptor();
    pSourceAdaptor->setAudioFormat( format );
    pPipe->attachSource( pSourceAdaptor );
    pPipeRunnerListener = std::make_shared<PipeRunnerListener>( pDecoder );
    pPipe->registerRunnerStatusListener( pPipeRunnerListener );
  } else {
    pPipe->attachSource( pSource );
  }

  // setup sink
  std::shared_ptr<ISink> pSink;
  if( !optParser.values["-o"].empty() ){
    std::shared_ptr<FileStream> pStream = std::make_shared<FileStream>(optParser.values["-o"]);
    pSink = std::make_shared<StreamSink>(format, pStream);
  } else {
    if( !optParser.values["-s"].empty() ){
      SinkManager::setPlugInPath(optParser.values["-s"]);
      pSinkManager = SinkManager::getInstance();
      pSinkManager->initialize();

      std::vector<std::string> plugInIds = pSinkManager->getPlugInIds();
      for(auto& aPlugInId : plugInIds){
        pSink = std::dynamic_pointer_cast<ISink>( pSinkManager->getPlugIn( aPlugInId ) );
        if( pSink ){
          break;
        }
      }
    }
    if( !pSink ){
      pSink = std::make_shared<PcmSink>();
    }
  }
  if( pSink == nullptr ){
    std::cout << "Unexpected error. pSink == nullptr" << std::endl;
    exit(-1);
  }
  pSink->setAudioFormat( format );
  pPipe->attachSink( pSink );

  // setup parameter
  ParameterManager* pParams = ParameterManager::getManager();
  if( !optParser.values["-p"].ends_with(";") ){
    optParser.values["-p"] = optParser.values["-p"] + ";";
  }
  StringTokenizer token( optParser.values["-p"], ";" );
  while( token.hasNext() ){
    StringTokenizer aParam( token.getNext(), "=" );
    while( aParam.hasNext() ){
      pParams->setParameter( aParam.getNext(), aParam.getNext() );
    }
  }

  // dump the setup-ed pipe, source, filter, sink
  pPipe->dump();

  // execute the pipe
  pPipe->run();
  int nExecLatency = std::stoi( optParser.values["-t"] );
  std::this_thread::sleep_for(std::chrono::microseconds(nExecLatency));
  pPipe->stop();

  // dump the pipe execution result
  pSink->dump();

  // finalize
  pPipe->clearFilters(); // IMPORTANT: Before FilterManager's terminate(), all of references are needed to clear.
  if( pPipeRunnerListener ){
    pPipe->unregisterRunnerStatusListener( pPipeRunnerListener );
    pPipeRunnerListener.reset();
  }
  pPipe.reset();

  if( pFilterManager ) pFilterManager->terminate();
  if( pSinkManager ) pSinkManager->terminate();
  if( pCodecManager ) pCodecManager->terminate();
  if( pSourceManager ) pSourceManager->terminate();

  return 0;
}
