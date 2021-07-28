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

#include "Decoder.hpp"
#include "Buffer.hpp"

IDecoder::IDecoder() : IMediaCodec(), mpSource(nullptr)
{

}

IDecoder::~IDecoder()
{

}

std::shared_ptr<ISource> IDecoder::attachSource(std::shared_ptr<ISource> pSource)
{
  std::shared_ptr<ISource> pPrevSource = mpSource;
  mpSource = pSource;
  return pPrevSource;
}

std::shared_ptr<ISource> IDecoder::detachSource(void)
{
  std::shared_ptr<ISource> pPrevSource = mpSource;
  mpSource = nullptr;
  return pPrevSource;
}

std::shared_ptr<ISource> IDecoder::allocateSourceAdaptor(void)
{
  std::shared_ptr<InterPipeBridge> pSource = std::make_shared<InterPipeBridge>();
  pSource->setRequiredResourceConsumption( stateResourceConsumption() );
  mpInterPipeBridges.push_back( pSource );
  return std::dynamic_pointer_cast<ISource>( pSource );
}

void IDecoder::releaseSourceAdaptor(std::shared_ptr<ISource> pSource)
{
  std::shared_ptr<InterPipeBridge> pInterPipeBridge = std::dynamic_pointer_cast<InterPipeBridge>(pSource);
  if( pInterPipeBridge ){
    pInterPipeBridge->unlock();
    std::erase( mpInterPipeBridges, pInterPipeBridge );
  }
}

void IDecoder::process(void)
{
  AudioFormat format(AudioFormat::ENCODING::COMPRESSED);
  CompressAudioBuffer esBuf( format, getEsChunkSize() );

  AudioBuffer outBuf;
  for( auto& pInterPipe : mpInterPipeBridges ){
    outBuf.setAudioFormat( pInterPipe->getAudioFormat() );
    break;
  }

  while( mbIsRunning && mpSource && !mpInterPipeBridges.empty() ){
    mpSource->read( esBuf );
    doProcess( esBuf, outBuf );

    for( auto& pInterPipe : mpInterPipeBridges ){
      pInterPipe->write( outBuf );
    }
  }
}

std::shared_ptr<IMediaCodec> IDecoder::createByFormat(AudioFormat format, bool bDecoder)
{
  // TODO get instance from DecoderManager with the format
  return std::make_shared<NullDecoder>(format);
}

NullDecoder::NullDecoder(AudioFormat format):IDecoder(),mFormat(format)
{

}

NullDecoder::~NullDecoder()
{

}

void NullDecoder::configure(MediaParam param)
{

}

int NullDecoder::stateResourceConsumption(void)
{
  return 0;
}

int NullDecoder::getEsChunkSize(void)
{
  return 256; // dummy size of ES chunk
}

void NullDecoder::doProcess(IAudioBuffer& inBuf, IAudioBuffer& outBuf)
{
  ByteBuffer tmpBuffer(256, 0);
  outBuf.setRawBuffer(tmpBuffer);
}

AudioFormat NullDecoder::getFormat(void)
{
  return mFormat;
}