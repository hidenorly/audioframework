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

PipeMixer::PipeMixer(AudioFormat format, ISink* pSink) : ThreadBase(), mFormat(format), mpSink(pSink)
{

}

PipeMixer::~PipeMixer()
{
  stop();
  ISink* pSink = detachSink();
  delete pSink;
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

ISink* PipeMixer::attachSink(ISink* pSink)
{
  ISink* pPrevSink = mpSink;
  mpSink = pSink;
  return pPrevSink;
}

ISink* PipeMixer::detachSink(void)
{
  ISink* pPrevSink = mpSink;
  mpSink = nullptr;
  return pPrevSink;
}

void PipeMixer::process(void)
{
  if( mpSink && !mpInterPipeBridges.empty() ){

    int nSamples = 256;
    AudioBuffer outBuf( mFormat, nSamples );
    std::vector<AudioBuffer*> buffers;
    for(int i=0; i<mpInterPipeBridges.size(); i++){
      buffers.push_back( new AudioBuffer(mFormat, nSamples) );
    }

    while( mbIsRunning ){
      for(int i=0; mbIsRunning && i<mpInterPipeBridges.size(); i++){
        mpInterPipeBridges[i]->read( *buffers[i] );
      }
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

ISink* PipeMixer::allocateSinkAdaptor(void)
{
  InterPipeBridge* pInterPipeBridge = new InterPipeBridge( mFormat );
  mpInterPipeBridges.push_back( pInterPipeBridge );
  return (ISink*)pInterPipeBridge;
}

void PipeMixer::releaseSinkAdaptor(ISink* pSink)
{
  InterPipeBridge* pInterPipeBridge = dynamic_cast<InterPipeBridge*>(pSink);
  if( pInterPipeBridge ){
    delete pInterPipeBridge;
    std::erase( mpInterPipeBridges, pInterPipeBridge );
  }
}

