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
#include <filesystem>

AudioFormat getAudioFormatFromOpts( std::string encoding, std::string samplingRate, std::string channels )
{
  AudioFormat::ENCODING _encoding = AudioFormat::getEncodingFromString(encoding);
  int _samplingRate = std::stoi(samplingRate);
  AudioFormat::CHANNEL _channel = AudioFormat::getChannelsFromString(channels);

  return AudioFormat(_encoding, _samplingRate, _channel);
}


int main(int argc, char **argv)
{
  std::vector<OptParse::OptParseItem> options;
  options.push_back( OptParse::OptParseItem("-r", "--sampleRate", true, "48000", "Set Sampling Rate"));
  options.push_back( OptParse::OptParseItem("-e", "--encoding", true, "PCM_16BIT", "Set Encoding PCM_8BIT, PCM_16BIT, PCM_24BIT, PCM_32BIT, PCM_FLOAT"));
  options.push_back( OptParse::OptParseItem("-c", "--channel", true, "2", "Set channel 2, 2.1, 4, 4.1, 5, 5.1, 5.1.2, 7.1"));
  options.push_back( OptParse::OptParseItem("-f", "--filter", true, "", "Specify filter.so (dylib)"));
  options.push_back( OptParse::OptParseItem("-p", "--parameters", true, "", "Specify parameters (filter.paramA=0.2;filter.paramB=true)"));
  options.push_back( OptParse::OptParseItem("-i", "--input", true, "input.pcm", "Specify input(source) file"));
  options.push_back( OptParse::OptParseItem("-o", "--output", true, "", "Specify output(sink) file"));

  std::filesystem::path fdkPath = argv[0];
  OptParse optParser( argc, argv, options, std::string("Filter executor e.g. ")+std::string(fdkPath.filename())+std::string(" -f lib/filter-plugin/libfilter_example.so") );

  FilterManager::setPlugInPath(optParser.values["-f"]);
  FilterManager* pManager = FilterManager::getInstance();
  pManager->initialize();

  std::shared_ptr<IPipe> pPipe = std::make_shared<Pipe>();

  std::vector<std::string> plugInIds = pManager->getPlugInIds();
  for(auto& aPlugInId : plugInIds){
//    std::shared_ptr<IFilter> pFilter = FilterManager::newInstanceById( aPlugInId );
    std::shared_ptr<IFilter> pFilter = std::dynamic_pointer_cast<IFilter>( pManager->getPlugIn( aPlugInId ) );
    pPipe->addFilterToTail( pFilter );
  }

  AudioFormat format = getAudioFormatFromOpts( optParser.values["-e"], optParser.values["-r"], optParser.values["-c"] );
  std::cout << "Specified audio format : " << format.toString() << std::endl;

  std::shared_ptr<ISource> pSource;
  if( !std::filesystem::exists( optParser.values["-i"] ) ){
    pSource = std::make_shared<PcmSource>();
  } else {
    std::shared_ptr<FileStream> pStream = std::make_shared<FileStream>(optParser.values["-i"]);
    pSource = std::make_shared<StreamSource>(format, pStream);
  }
  pSource->setAudioFormat( format );
  pPipe->attachSource( pSource );

  std::shared_ptr<ISink> pSink;
  if( optParser.values["-o"].empty() ){
    pSink = std::make_shared<PcmSink>();
  } else {
    std::shared_ptr<FileStream> pStream = std::make_shared<FileStream>(optParser.values["-o"]);
    pSink = std::make_shared<StreamSink>(format, pStream);
  }
  pSink->setAudioFormat( format );
  pPipe->attachSink( pSink );

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

  pPipe->dump();

  pPipe->run();
  std::this_thread::sleep_for(std::chrono::microseconds(1000));
  pPipe->stop();

  pSink->dump();

  pPipe->clearFilters(); // IMPORTANT: Before FilterManager's terminate(), all of references are needed to clear.
  pPipe.reset();

  pManager->terminate();

  return 0;
}
