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

#include "Media.hpp"
#include "Decoder.hpp"
#include "Encoder.hpp"
#include <iostream>

IMediaCodec::IMediaCodec() : ThreadBase(), IResourceConsumer()
{

}

IMediaCodec::~IMediaCodec()
{

}

void IMediaCodec::configure(std::vector<MediaParam> params)
{
  for(MediaParam& aParam : params){
    configure(aParam);
  }
}

void IMediaCodec::unlockToStop(void)
{
  for( auto& pInterPipeBridge : mpInterPipeBridges ){
    pInterPipeBridge->unlock();
  }
}

void IMediaCodec::seek(int64_t position)
{

}

int64_t IMediaCodec::getPosition(void)
{
  return 0;
}

std::shared_ptr<IMediaCodec> IMediaCodec::createByFormat(AudioFormat format, bool bDecoder)
{
  std::shared_ptr<IMediaCodec> result;

  MediaCodecManager* pManager = MediaCodecManager::getInstance();
  std::vector<std::string> plugInIds = pManager->getPlugInIds();
  for(auto& aPlugInId : plugInIds){
    std::shared_ptr<IMediaCodec> pCodec = std::dynamic_pointer_cast<IMediaCodec>( pManager->getPlugIn( aPlugInId ) );
    if( pCodec && ( pCodec->isDecoder() == bDecoder ) && pCodec->canHandle( format ) ){
      result = std::dynamic_pointer_cast<IMediaCodec>( pCodec->newInstance() );
      break;
    }
  }

  if( !result ){
    std::cout << "Not found specified format handle-able codec in the plug-in." << std::endl;
    result = bDecoder ? std::dynamic_pointer_cast<IMediaCodec>( std::make_shared<NullDecoder>() ) : std::dynamic_pointer_cast<IMediaCodec>( std::make_shared<NullEncoder>() );
  }

  return result;
}
