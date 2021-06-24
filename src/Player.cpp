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

Player::Player():mpDecoder(nullptr), mbIsSetupDone(false), mbIsPaused(false), mPosition(0)
{

}

Player::~Player()
{

}

std::shared_ptr<ISource> Player::prepare(std::shared_ptr<ISource> pSource, std::shared_ptr<IMediaCodec> pDecoder)
{
  std::shared_ptr<ISource> pDec2Pipe = nullptr;

  if( !mpDecoder && pSource && pDecoder ){
    mpDecoder = std::dynamic_pointer_cast<IDecoder>(pDecoder);
    mpDecoder->attachSource( pSource );
    pDec2Pipe = mpDecoder->allocateSourceAdaptor();
    mbIsSetupDone = true;
    mbIsPaused = false;
  }

  return pDec2Pipe;
}

std::shared_ptr<ISource> Player::terminate(std::shared_ptr<ISource> pSourceAdaptor)
{
  std::shared_ptr<ISource> pDecoderSource = nullptr;
  if( mpDecoder ){
    mpDecoder->stop();
    mpDecoder->releaseSourceAdaptor( pSourceAdaptor );
    pDecoderSource = mpDecoder->detachSource();
    mpDecoder.reset();
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
  if( mbIsSetupDone && mpDecoder ){
    mpDecoder->seek( ptsUSec );
    mpDecoder->run();
    mbIsPaused = false;
  }
}

void Player::stop(void)
{
  if( mbIsSetupDone && mpDecoder ){
    mpDecoder->stop();
    mbIsPaused = false;
  }
}

void Player::pause(void)
{
  if( mbIsSetupDone && mpDecoder ){
    mPosition = mpDecoder->getPosition();
    mpDecoder->stop();
    mbIsPaused = true;
  }
}

void Player::resume(void)
{
  if( mbIsSetupDone && mpDecoder ){
    if( mbIsPaused ){
      play( mPosition );
    }
    mbIsPaused = false;
  }
}

void Player::seek(int64_t ptsUSec)
{
  if( mbIsSetupDone && mpDecoder ){
    mpDecoder->stop();
    mpDecoder->seek( ptsUSec );
    mpDecoder->run();
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

