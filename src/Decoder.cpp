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
#include "ThreadBase.hpp"
#include "Buffer.hpp"
#include <vector>

IDecoder::IDecoder() : ThreadBase(), mpSource(nullptr)
{

}

IDecoder::~IDecoder()
{

}

void IDecoder::configure(DecoderParam param)
{

}

void IDecoder::configure(std::vector<DecoderParam> params)
{
  for(DecoderParam& aParam : params){
    configure(aParam);
  }
}

ISource* IDecoder::attachSource(ISource* pSource)
{
  ISource* pPrevSource = mpSource;
  mpSource = pSource;
  return pPrevSource;
}

ISource* IDecoder::detachSource(void)
{
  ISource* pPrevSource = mpSource;
  mpSource = nullptr;
  return pPrevSource;
}


ISource* IDecoder::allocateSourceAdaptor(void)
{
  InterPipeBridge* pSource = new InterPipeBridge();
  mpInterPipeBridges.push_back( pSource );
  return dynamic_cast<ISource*>( pSource );
}

void IDecoder::releaseSourceAdaptor(ISource* pSource)
{
  InterPipeBridge* pInterPipeBridge = dynamic_cast<InterPipeBridge*>(pSource);
  delete pInterPipeBridge;
  std::erase( mpInterPipeBridges, pInterPipeBridge );
}

void IDecoder::seek(int64_t position)
{

}

int64_t IDecoder::getPosition(void)
{
  return 0;
}

void IDecoder::unlockToStop(void)
{
  for( auto& pInterPipeBridge : mpInterPipeBridges ){
    pInterPipeBridge->unlock();
  }
}


NullDecoder::NullDecoder():IDecoder()
{

}

NullDecoder::~NullDecoder()
{

}

void NullDecoder::configure(DecoderParam param)
{

}

void NullDecoder::process(void)
{
  AudioFormat format(AudioFormat::ENCODING::COMPRESSED);

  CompressAudioBuffer esBuf( format );

  AudioBuffer outBuf;
  while( mbIsRunning && mpSource && !mpInterPipeBridges.empty() ){
    mpSource->read( esBuf );
    // do decode the buf
    ByteBuffer tmpBuffer(256, 0);
    outBuf.setRawBuffer(tmpBuffer);

    for( auto& pInterPipe : mpInterPipeBridges ){
      pInterPipe->write( outBuf );
    }
  }
}
