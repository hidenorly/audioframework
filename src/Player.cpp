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

#include "Player.hpp"
#include "PipeManager.hpp"
#include "InterPipeBridge.hpp"

Player::Player():mpDecoder(nullptr), mpPipe(nullptr), mbIsSetupDone(false), mbIsPaused(false), mPosition(0)
{

}

Player::~Player()
{

}


ISource* Player::prepare(ISource* pSource, IDecoder* pDecoder)
{
  InterPipeBridge* pDec2Pipe = nullptr;

  if( !mpDecoder && pSource && pDecoder ){
    mpDecoder = pDecoder;
    pDecoder->attachSource( pSource );
    pDec2Pipe = new InterPipeBridge();
    mpPipe = new PipeManager();
    mpPipe->attachSource( pDecoder->allocateSourceAdaptor() );
    mpPipe->attachSink( dynamic_cast<ISink*>(pDec2Pipe) );
    mbIsSetupDone = true;
    mbIsPaused = false;
  }

  return pDec2Pipe;
}

ISource* Player::terminate(ISource* pSource)
{
  ISource* pDecoderSource = nullptr;
  InterPipeBridge* pDec2Pipe = nullptr;
  if( mpPipe ){
    mpPipe->stop();
    ISink* pSink = mpPipe->detachSink();
    if( dynamic_cast<InterPipeBridge*>(pSource) == pSink ){
      delete pSink;
      pSink = nullptr;
      pSource = nullptr;
    }
    pDec2Pipe = dynamic_cast<InterPipeBridge*>( mpPipe->detachSource() );
    delete mpPipe; mpPipe = nullptr;
  }
  if( mpDecoder ){
    mpDecoder->stop();
    if( pDec2Pipe ){
      mpDecoder->releaseSourceAdaptor( pDec2Pipe );
    }
    pDecoderSource = mpDecoder->detachSource();
    delete mpDecoder; mpDecoder = nullptr;
  }
  mbIsSetupDone = false;
  mbIsPaused = false;

  return pDecoderSource;
}

bool Player::isReady(void)
{
  return mbIsSetupDone;
}

void Player::play(int64_t ptsUSec)
{
  if( mbIsSetupDone && mpDecoder && mpPipe ){
    mpDecoder->seek( ptsUSec );
    mpDecoder->run();
    mpPipe->run();  
    mbIsPaused = false;
  }
}

void Player::stop(void)
{
  if( mbIsSetupDone && mpDecoder && mpPipe ){
    mpDecoder->stop();
    mpPipe->stop();  
    mbIsPaused = false;
  }
}

void Player::pause(void)
{
  if( mbIsSetupDone && mpDecoder && mpPipe ){
    mPosition = mpDecoder->getPosition();
    mpDecoder->stop();
    mpPipe->stop();
    mbIsPaused = true;
  }
}

void Player::resume(void)
{
  if( mbIsSetupDone && mpDecoder && mpPipe ){
    if( mbIsPaused ){
      play( mPosition );
    }
    mbIsPaused = false;
  }
}

void Player::seek(int64_t ptsUSec)
{
  if( mbIsSetupDone && mpDecoder && mpPipe ){
    mpDecoder->stop();
    mpPipe->stop();  
    mpDecoder->seek( ptsUSec );
    mpDecoder->run();
    mpPipe->run();  
  }
}

int64_t Player::getPosition(void)
{
  int64_t pos = 0;
  if( mbIsSetupDone && mpDecoder ){
    pos = mbIsPaused ? mPosition : mpDecoder->getPosition();
  }
  return pos;
}

