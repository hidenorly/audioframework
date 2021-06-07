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

#include "AccousticEchoCancelledSource.hpp"
#include "DelayFilter.hpp"
#include "Buffer.hpp"

AccousticEchoCancelledSource::AccousticEchoCancelledSource(ISource* pSource, ISink* pReferenceSound, bool bDelayOnly) : mpSource(pSource)
{
  mpDelay = nullptr;
  mpAecFilter = nullptr;

  if( pSource && pReferenceSound ){
    int latencyUsec = pSource->getLatencyUSec() + pReferenceSound->getLatencyUSec();
    mpDelay = new DelayFilter( pSource->getAudioFormat(), latencyUsec );
    if( !bDelayOnly ){
      mpAecFilter = new AccousticEchoCancelFilter( pReferenceSound );
    }
  }
}

AccousticEchoCancelledSource::~AccousticEchoCancelledSource()
{
  if( mpDelay ){
    delete mpDelay;
    mpDelay = nullptr;
  }
  if( mpAecFilter ){
    delete mpAecFilter;
    mpAecFilter = nullptr;
  }
}

void AccousticEchoCancelledSource::readPrimitive(IAudioBuffer& buf)
{
  if( mpSource ){
    AudioBuffer* pOutBuf = dynamic_cast<AudioBuffer*>(&buf);
    if( mpDelay && pOutBuf ){
      AudioBuffer tmpBuffer( pOutBuf->getAudioFormat(), pOutBuf->getSamples() );
      mpSource->read( tmpBuffer );
      mpDelay->process( tmpBuffer, *pOutBuf );
      if( mpAecFilter ){
        mpAecFilter->process( *pOutBuf, tmpBuffer );
        *pOutBuf = tmpBuffer;
      }
    } else {
      mpSource->read( buf );
    }
  }
}
