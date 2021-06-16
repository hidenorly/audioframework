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

#include "ReferenceSoundSinkSource.hpp"
#include <algorithm>

ReferenceSoundSinkSource::ReferenceSoundSinkSource( std::shared_ptr<ISink> pSink ) : InterPipeBridge( pSink ? pSink->getAudioFormat() : AudioFormat() ), mpSink( pSink )
{
  setRequiredResourceConsumption(0);
}

ReferenceSoundSinkSource::~ReferenceSoundSinkSource()
{

}

void ReferenceSoundSinkSource::readPrimitive(IAudioBuffer& buf)
{
  if( mpSink ){
    InterPipeBridge::readPrimitive( buf );
  } else {
    ByteBuffer zeroBuf( buf.getRawBuffer().size(), 0 );
    buf.setRawBuffer( zeroBuf );
  }
}

void ReferenceSoundSinkSource::writePrimitive(IAudioBuffer& buf)
{
  mMutexSink.lock();
  if( mpSink ){
    mpSink->writePrimitive( buf );
  }
  mMutexSink.unlock();
  InterPipeBridge::writePrimitive( buf );
}

std::shared_ptr<ISink> ReferenceSoundSinkSource::attachSink(std::shared_ptr<ISink> pSink)
{
  mMutexSink.lock();
  std::shared_ptr<ISink> pPrevSink = mpSink;
  mpSink = pSink;
  mMutexSink.unlock();
  return pPrevSink;
}

std::shared_ptr<ISink> ReferenceSoundSinkSource::detachSink(void)
{
  mMutexSink.lock();
  std::shared_ptr<ISink> pPrevSink = mpSink;
  mpSink = nullptr;
  clearBuffer();
  mMutexSink.unlock();
  return pPrevSink;
}

void ReferenceSoundSinkSource::clearBuffer(void)
{
  unlock();
  mFifoBuffer.clearBuffer();
}

bool ReferenceSoundSinkSource::setAudioFormat(AudioFormat audioFormat)
{
  bool result = false;
  mMutexSink.lock();
  if( mpSink ){
    result = mpSink->setAudioFormat( audioFormat );
  }
  mMutexSink.unlock();
  if( result ){
    mFifoBuffer.setAudioFormat( audioFormat ); // clear buffer is done by this call internally
  }
  return result;
}

AudioFormat ReferenceSoundSinkSource::getAudioFormat(void)
{
  AudioFormat result = InterPipeBridge::getAudioFormat();
  AudioFormat resultSink = result;
  mMutexSink.lock();
  if( mpSink ){
    AudioFormat resultSink = mpSink->getAudioFormat();
  }
  mMutexSink.unlock();

  if( !resultSink.equal( result ) ){
    setAudioFormat( resultSink );
    result = resultSink;
  }

  return result;
}

int ReferenceSoundSinkSource::stateResourceConsumption(void)
{
  int result = InterPipeBridge::stateResourceConsumption();
  int resultSink = 0;
  mMutexSink.lock();
  if( mpSink ){
    resultSink = mpSink->stateResourceConsumption();
  }
  mMutexSink.unlock();
  return std::max(result, resultSink);
}