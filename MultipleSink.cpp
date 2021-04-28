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

#include "MultipleSink.hpp"

MultipleSink::MultipleSink(AudioFormat audioFormat):mFormat(audioFormat)
{

}

MultipleSink::~MultipleSink()
{
  clearSinks();
}


void MultipleSink::addSink(ISink* pSink, ChannelMapper& map)
{
  mpSinks.push_back( pSink );
  mChannelMaps.insert( std::make_pair(pSink, map) );
}

void MultipleSink::clearSinks(void)
{
  for(auto& pSink : mpSinks ){
    delete pSink;
  }
  mpSinks.clear();
  mChannelMaps.clear();
}

AudioBuffer MultipleSink::selectedChannels(AudioBuffer& srcBuf, ChannelMapper& mapper)
{
  // TODO: extract corresponding channel's data & reconstruct the buffer
  AudioBuffer selectedBuf( srcBuf.getAudioFormat(), srcBuf.getSamples() );
  selectedBuf = srcBuf;
  return selectedBuf;
}

void MultipleSink::write(AudioBuffer& buf)
{
  for(auto& pSink : mpSinks ){
    ChannelMapper mapper = mChannelMaps[ pSink ];
    AudioBuffer selectedBuf = selectedChannels( buf, mapper );
    pSink->write( selectedBuf );
  }
}

void MultipleSink::dump(void)
{
  std::cout << "MultipleSink::list of sinks" << std::endl;

  for(auto& pSink : mpSinks ){
    std::cout << "Sink:" << pSink << std::endl;
    ChannelMapper mapper = mChannelMaps[ pSink ];
    for( const auto& [dstCh, srcCh] : mapper ){
      std::cout << "SrcCh:" << srcCh << " -> DstCh:" << dstCh << std::endl; 
    }
    pSink->dump();
  }
}

bool MultipleSink::setAudioFormat(AudioFormat audioFormat)
{
  mFormat = audioFormat;
  return true;
}

AudioFormat MultipleSink::getAudioFormat(void)
{
  return mFormat;
}

