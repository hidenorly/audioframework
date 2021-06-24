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

#include "Encoder.hpp"
#include "Buffer.hpp"

IEncoder::IEncoder() : IMediaCodec(), mpSink(nullptr)
{

}

IEncoder::~IEncoder()
{

}

std::shared_ptr<ISink> IEncoder::attachSink(std::shared_ptr<ISink> pSink)
{
  std::shared_ptr<ISink> pPrevSink = mpSink;
  mpSink = pSink;
  return pPrevSink;
}

std::shared_ptr<ISink> IEncoder::detachSink(void)
{
  std::shared_ptr<ISink> pPrevSink = mpSink;
  mpSink = nullptr;
  return pPrevSink;
}

std::shared_ptr<ISink> IEncoder::allocateSinkAdaptor(void)
{
  std::shared_ptr<InterPipeBridge> pSink = std::make_shared<InterPipeBridge>();
  pSink->setRequiredResourceConsumption( stateResourceConsumption() );
  mpInterPipeBridges.push_back( pSink );
  return std::dynamic_pointer_cast<ISink>( pSink );
}

void IEncoder::releaseSinkAdaptor(std::shared_ptr<ISink> pSink)
{
  std::shared_ptr<InterPipeBridge> pInterPipeBridge = std::dynamic_pointer_cast<InterPipeBridge>(pSink);
  if( pInterPipeBridge ){
    pInterPipeBridge->unlock();
    std::erase( mpInterPipeBridges, pInterPipeBridge );
  }
}

void IEncoder::process(void)
{
  AudioFormat format(AudioFormat::ENCODING::COMPRESSED);

  CompressAudioBuffer esBuf( format, getEsChunkSize() );

  AudioFormat inBufFormat;
  for( auto& pInterPipe : mpInterPipeBridges ){
    inBufFormat = pInterPipe->getAudioFormat();
    break;
  }
  AudioBuffer inPcmBuf( inBufFormat, getRequiredSamples() );
  while( mbIsRunning && mpSink && !mpInterPipeBridges.empty() ){
    for( auto& pInterPipe : mpInterPipeBridges ){
      pInterPipe->read( inPcmBuf );
      break;
    }
    doProcess( inPcmBuf, esBuf );
    mpSink->write( esBuf );
  }
}

std::shared_ptr<IMediaCodec> IEncoder::createByFormat(AudioFormat format, bool bDecoder)
{
  // TODO get instance from EncoderManager with the format
  return std::make_shared<NullEncoder>(format);
}


NullEncoder::NullEncoder(AudioFormat format):IEncoder(),mFormat(format)
{

}

NullEncoder::~NullEncoder()
{

}

void NullEncoder::configure(MediaParam param)
{

}

int NullEncoder::stateResourceConsumption(void)
{
  return 0;
}

int NullEncoder::getEsChunkSize(void)
{
  return 256; // dummy size of ES chunk
}

int NullEncoder::getRequiredSamples(void)
{
  return 256; // dummy required samples
}

void NullEncoder::doProcess(IAudioBuffer& inBuf, IAudioBuffer& outBuf)
{
  outBuf.setRawBuffer( inBuf.getRawBuffer() );
}

AudioFormat NullEncoder::getFormat(void)
{
  return mFormat;
}