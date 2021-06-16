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

#include "PipeMixer.hpp"
#include "Buffer.hpp"
#include "Mixer.hpp"
#include <vector>

PipeMixer::PipeMixer(AudioFormat format, std::shared_ptr<ISink> pSink) : ThreadBase(), mFormat(format), mpSink(pSink)
{

}

PipeMixer::~PipeMixer()
{
  stop();
  detachSink();
}

bool PipeMixer::setAudioFormat(AudioFormat audioFormat)
{
  mFormat = audioFormat;
  return true;
}

AudioFormat PipeMixer::getAudioFormat(void)
{
  return mFormat;
}

std::shared_ptr<ISink> PipeMixer::attachSink(std::shared_ptr<ISink> pSink)
{
  std::shared_ptr<ISink> pPrevSink = mpSink;
  mpSink = pSink;
  return pPrevSink;
}

std::shared_ptr<ISink> PipeMixer::detachSink(void)
{
  std::shared_ptr<ISink> pPrevSink = mpSink;
  mpSink = nullptr;
  return pPrevSink;
}

void PipeMixer::process(void)
{
  while( mbIsRunning && mpSink && !mpInterPipeBridges.empty() ){
    int nSamples = 256;
    AudioBuffer outBuf( mFormat, nSamples );
    std::vector<AudioBuffer*> buffers;
    int nCurrentPipeSize = mpInterPipeBridges.size();
    for(int i=0; i<nCurrentPipeSize; i++){
      buffers.push_back( new AudioBuffer(mFormat, nSamples) );
    }

    while( mbIsRunning && (nCurrentPipeSize == mpInterPipeBridges.size()) ){
      mMutexPipe.lock();
      for(int i=0; mbIsRunning && i<mpInterPipeBridges.size(); i++){
        mpInterPipeBridges[i]->read( *buffers[i] );
      }
      mMutexPipe.unlock();
      if( mbIsRunning ){
        Mixer::process( buffers, &outBuf );
      }
      if( mbIsRunning && mpSink ){
        mpSink->write( outBuf );
      }
    }

    for( AudioBuffer* pBuffer : buffers ){
      delete pBuffer;
    }
    buffers.clear();
  }
}

void PipeMixer::unlockToStop(void)
{
  for( auto& pPipeBridge : mpInterPipeBridges ){
    pPipeBridge->unlock();
  }
}

std::shared_ptr<ISink> PipeMixer::allocateSinkAdaptor(void)
{
  std::shared_ptr<InterPipeBridge> pInterPipeBridge = std::make_shared<InterPipeBridge>( mFormat );
  mMutexPipe.lock();
  mpInterPipeBridges.push_back( pInterPipeBridge );
  mMutexPipe.unlock();
  return pInterPipeBridge;
}

void PipeMixer::releaseSinkAdaptor(std::shared_ptr<ISink> pSink)
{
  std::shared_ptr<InterPipeBridge> pInterPipeBridge = std::dynamic_pointer_cast<InterPipeBridge>(pSink);
  if( pInterPipeBridge ){
    pInterPipeBridge->unlock(); // TODO: May not enough to unlock this only.
    mMutexPipe.lock();
    std::erase( mpInterPipeBridges, pInterPipeBridge );
    mMutexPipe.unlock();
  }
}

