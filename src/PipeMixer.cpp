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
#include <memory>

PipeMixer::PipeMixer(AudioFormat format, std::shared_ptr<ISink> pSink) : ThreadBase(), mFormat(format), mpSink(pSink)
{

}

PipeMixer::~PipeMixer()
{
  stop();
  detachSink();
  mpInterPipeBridges.clear();
  mpPipes.clear();
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
        std::shared_ptr<IPipe> pPipe = nullptr;
        if( mpPipes.contains( mpInterPipeBridges[i] ) ){
          pPipe = mpPipes[mpInterPipeBridges[i]].lock();
        }
        if( pPipe && pPipe->isRunning() || !pPipe ){
          mpInterPipeBridges[i]->read( *buffers[i] );
        } else {
          ByteBuffer zeroBuffer( buffers[i]->getRawBuffer().size(), 0 );
          buffers[i]->setRawBuffer(zeroBuffer);
        }
        pPipe.reset();
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

std::shared_ptr<ISink> PipeMixer::getSinkFromPipe(std::shared_ptr<IPipe> pArgPipe)
{
  std::shared_ptr<ISink> result;
  if( pArgPipe ){
    mMutexPipe.lock();
    for( auto& [pInterPipeBridge, pPipe] : mpPipes){
      if( pPipe.lock() == pArgPipe ){
        result = pInterPipeBridge;
        break;
      }
    }
    mMutexPipe.unlock();
  }
  return result;
}

void PipeMixer::attachSinkAdaptor(std::shared_ptr<InterPipeBridge> pSource, std::shared_ptr<IPipe> pPipe)
{
  mMutexPipe.lock();
  if( !mpPipes.contains( pSource ) /* std::find(mpInterPipeBridges.begin(), mpInterPipeBridges.end(), pSource) == mpInterPipeBridges.end()*/ ){
    mpInterPipeBridges.push_back( pSource );
    mpPipes.insert_or_assign( pSource, pPipe );
  }
  mMutexPipe.unlock();
}


std::shared_ptr<ISink> PipeMixer::allocateSinkAdaptor(std::shared_ptr<IPipe> pPipe)
{
  std::shared_ptr<ISink> result = getSinkFromPipe( pPipe );
  if( !result ){
    std::shared_ptr<InterPipeBridge> pInterPipeBridge = std::make_shared<InterPipeBridge>( mFormat );
    attachSinkAdaptor( pInterPipeBridge, pPipe );
    result = pInterPipeBridge;
  }
  return result;
}

void PipeMixer::releaseSinkAdaptor(std::shared_ptr<ISink> pSink)
{
  std::shared_ptr<InterPipeBridge> pInterPipeBridge = std::dynamic_pointer_cast<InterPipeBridge>(pSink);
  if( pInterPipeBridge ){
    pInterPipeBridge->unlock(); // TODO: May not enough to unlock this only.
    mMutexPipe.lock();
    std::erase( mpInterPipeBridges, pInterPipeBridge );
    mpPipes.erase( pInterPipeBridge );
    mMutexPipe.unlock();
  }
}

