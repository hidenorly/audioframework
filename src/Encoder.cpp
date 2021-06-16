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
#include "ThreadBase.hpp"
#include "Buffer.hpp"
#include <vector>

IEncoder::IEncoder() : ThreadBase(), IResourceConsumer(), mpSink(nullptr)
{

}

IEncoder::~IEncoder()
{

}

void IEncoder::configure(MediaParam param)
{

}

void IEncoder::configure(std::vector<MediaParam> params)
{
  for(MediaParam& aParam : params){
    configure(aParam);
  }
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

int64_t IEncoder::getPosition(void)
{
  return 0;
}

void IEncoder::unlockToStop(void)
{
  for( auto& pInterPipeBridge : mpInterPipeBridges ){
    pInterPipeBridge->unlock();
  }
}


NullEncoder::NullEncoder():IEncoder()
{

}

NullEncoder::~NullEncoder()
{

}

void NullEncoder::configure(MediaParam param)
{

}

void NullEncoder::process(void)
{
  AudioFormat format(AudioFormat::ENCODING::COMPRESSED);

  CompressAudioBuffer esBuf( format );

  AudioFormat inBufFormat;
  for( auto& pInterPipe : mpInterPipeBridges ){
    inBufFormat = pInterPipe->getAudioFormat();
    break;
  }
  AudioBuffer inPcmBuf(inBufFormat, 256);
  while( mbIsRunning && mpSink && !mpInterPipeBridges.empty() ){
    for( auto& pInterPipe : mpInterPipeBridges ){
      pInterPipe->read( inPcmBuf );
      break;
    }
    // TODO : encode. the following is no encoded just copy the byte data...
    esBuf.setRawBuffer( inPcmBuf.getRawBuffer() );
    mpSink->write( esBuf );
  }
}

int NullEncoder::stateResourceConsumption(void)
{
  return 0;
}