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

PipeMixer::PipeMixer(AudioFormat format, ISink* pSink) : mFormat(format), mpSink(pSink)
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

void PipeMixer::run(void)
{

}
void PipeMixer::stop(void)
{

}

bool PipeMixer::isRunning(void)
{
  return false;
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
  delete pInterPipeBridge;
  std::erase( mpInterPipeBridges, pInterPipeBridge );
}

