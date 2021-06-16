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

AccousticEchoCancelledSource::AccousticEchoCancelledSource(std::shared_ptr<ISource> pSource, std::shared_ptr<ISource> pReferenceSound, bool bDelayOnly) : mpSource(pSource), mpReferenceSource( pReferenceSound )
{
  mpDelay = nullptr;
  mpAecFilter = nullptr;
  mDelayUsec = -1;

  createDelayFilter();
  if( !bDelayOnly ){
    mpAecFilter = new AccousticEchoCancelFilter();
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

void AccousticEchoCancelledSource::createDelayFilter()
{
  if( mpSource && mpReferenceSource ){
    int latencyUsec = mpSource->getLatencyUSec() + mpReferenceSource->getLatencyUSec();
    if( mDelayUsec != latencyUsec ){
      mDelayUsec = latencyUsec;
      mMutexDelay.lock();
      if( mpDelay ){
        delete mpDelay; mpDelay = nullptr;
      }
      mpDelay = new DelayFilter( mpSource->getAudioFormat(), latencyUsec );
      mMutexDelay.unlock();
    }
  }
}

void AccousticEchoCancelledSource::readPrimitive(IAudioBuffer& buf)
{
  if( mpSource ){
    AudioBuffer* pOutBuf = dynamic_cast<AudioBuffer*>(&buf);
    if( mpDelay && pOutBuf ){
      AudioBuffer tmpBuffer( pOutBuf->getAudioFormat(), pOutBuf->getNumberOfSamples() );
      mpSource->read( tmpBuffer );
      mMutexDelay.lock();
      mpDelay->process( tmpBuffer, *pOutBuf );
      mMutexDelay.unlock();
      if( mpAecFilter ){
        mpAecFilter->process( *pOutBuf, tmpBuffer ); // outBuf = outBuf - tmpBuffer
      }
    } else {
      mpSource->read( buf );
    }
  }
}

void AccousticEchoCancelledSource::adjustDelay(void)
{
  createDelayFilter();
}