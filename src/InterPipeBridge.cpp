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

#include "InterPipeBridge.hpp"

InterPipeBridge::InterPipeBridge(AudioFormat format) : ISource(), ISink(), mFifoBuffer(format), mRequiredResource(0)
{

}


void InterPipeBridge::readPrimitive(IAudioBuffer& buf)
{
  mFifoBuffer.read(buf);
}

void InterPipeBridge::writePrimitive(IAudioBuffer& buf)
{
  AudioBuffer* pBuf = dynamic_cast<AudioBuffer*>(&buf);
  if( pBuf ){
    int nSamples = pBuf->getSamples();
    if( nSamples ){
      mFifoBuffer.setFifoSizeLimit( nSamples*3 ); // at least tripple buffer
    }
  }
  mFifoBuffer.write(buf);
}

bool InterPipeBridge::setAudioFormat(AudioFormat audioFormat)
{
  throw std::runtime_error( "setAudioFormat is unsupported" );
	return false;
}

AudioFormat InterPipeBridge::getAudioFormat(void)
{
  return mFifoBuffer.getAudioFormat();
}

int InterPipeBridge::stateResourceConsumption(void)
{
  return mRequiredResource;
}

void InterPipeBridge::setRequiredResourceConsumption(int nRequiredResource)
{
  mRequiredResource = nRequiredResource;
}