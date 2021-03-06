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

#include "Testability.hpp"
#include <iostream>

ICapture::ICapture(AudioFormat format)
{
  mpRefBuf = std::make_shared<FifoBufferReadReference>( format );
}

ICapture::~ICapture()
{
  unlock();
  mpRefBuf.reset();
}

void ICapture::enqueToRefBuf(IAudioBuffer& buf)
{
  if( mpRefBuf ){
    mpRefBuf->write( buf );
  }
}

void ICapture::setCaptureBufferSize(int nSamples)
{
  if( mpRefBuf ){
    mpRefBuf->setFifoSizeLimit( nSamples );
  }
}

void ICapture::captureRead(IAudioBuffer& buf)
{
  if( mpRefBuf ){
    mpRefBuf->readReference( buf );
  }
}

void ICapture::setCaptureAudioFormat(AudioFormat audioFormat)
{
  if( mpRefBuf ){
    mpRefBuf->setAudioFormat( audioFormat );
  }
}


void ICapture::unlock(void)
{
  if( mpRefBuf ){
    mpRefBuf->unlock();
  }
}

SinkTestBase::SinkTestBase(std::shared_ptr<ISink> pSink) : ISink(), mpSink(pSink)
{

}

SinkTestBase::~SinkTestBase()
{
  mpSink = nullptr;
}

std::vector<ISink::PRESENTATION> SinkTestBase::getAvailablePresentations(void)
{
  return mpSink ? mpSink->getAvailablePresentations() : ISink::getAvailablePresentations();
}

bool SinkTestBase::isAvailablePresentation(PRESENTATION presentation)
{
  return mpSink ? mpSink->isAvailablePresentation( presentation ) : ISink::isAvailablePresentation( presentation );
}

bool SinkTestBase::setPresentation(PRESENTATION presentation)
{
  return mpSink ? mpSink->setPresentation( presentation ) : ISink::setPresentation( presentation );
}

ISink::PRESENTATION SinkTestBase::getPresentation(void)
{
  return mpSink ? mpSink->getPresentation() : ISink::getPresentation();
}

void SinkTestBase::setAudioFormatPrimitive(AudioFormat audioFormat)
{
  if( mpSink ){
    mpSink->setAudioFormat( audioFormat );
  }
}

AudioFormat SinkTestBase::getAudioFormat(void)
{
  return mpSink ? mpSink->getAudioFormat() : AudioFormat();
}

float SinkTestBase::getVolume(void)
{
  return mpSink ? mpSink->getVolume() : ISink::getVolume();
}

bool SinkTestBase::setVolume(float volumePercentage)
{
  return mpSink ? mpSink->setVolume( volumePercentage ) : ISink::setVolume( volumePercentage );
}

int SinkTestBase::getLatencyUSec(void)
{
  return mpSink ? mpSink->getLatencyUSec() : ISink::getLatencyUSec();
}

int64_t SinkTestBase::getSinkPts(void)
{
  return mpSink ? mpSink->getSinkPts() : ISink::getSinkPts();
}

void SinkTestBase::dump(void)
{
 if( mpSink ){
    return mpSink->dump();
  }
}

int SinkTestBase::stateResourceConsumption(void)
{
  return mpSink ? mpSink->stateResourceConsumption() : ISink::stateResourceConsumption();
}


SinkCapture::SinkCapture(std::shared_ptr<ISink> pSink) : SinkTestBase(pSink), ICapture( pSink ? pSink->getAudioFormat() : AudioFormat() )
{

}

SinkCapture::~SinkCapture()
{
}

void SinkCapture::writePrimitive(IAudioBuffer& buf)
{
  if( mpSink ){
    mpSink->writePrimitive( buf );
  }
  enqueToRefBuf( buf );
}

void SinkCapture::setAudioFormatPrimitive(AudioFormat audioFormat)
{
  setCaptureAudioFormat( audioFormat );
  if( mpSink ){
    mpSink->setAudioFormat( audioFormat );
  }
}

SinkInjector::SinkInjector(std::shared_ptr<ISink> pSink) : SinkTestBase(pSink), IInjector()
{

}

SinkInjector::~SinkInjector()
{
}

void SinkInjector::writePrimitive(IAudioBuffer& buf)
{
  if( getInjectorEnabled() ){
    dequeFromInjectBuf( buf );
  }
  if( mpSink ){
    mpSink->writePrimitive( buf );
  }
}

void SinkInjector::setAudioFormatPrimitive(AudioFormat audioFormat)
{
  setInjectAudioFormat( audioFormat );
  if( mpSink ){
    mpSink->setAudioFormat( audioFormat );
  }
}

SourceTestBase::SourceTestBase(std::shared_ptr<ISource> pSource) : ISource(), mpSource( pSource )
{

}

SourceTestBase::~SourceTestBase()
{
  mpSource = nullptr;
};

SourceCapture::SourceCapture(std::shared_ptr<ISource> pSource) : SourceTestBase( pSource), ICapture( pSource ? pSource->getAudioFormat() : AudioFormat() )
{

}

SourceCapture::~SourceCapture()
{
}

void SourceCapture::readPrimitive(IAudioBuffer& buf)
{
  if( mpSource ){
    mpSource->readPrimitive( buf );
  }
  enqueToRefBuf( buf );
}

int SourceTestBase::getLatencyUSec(void)
{
  return mpSource ? mpSource->getLatencyUSec() : ISource::getLatencyUSec();
}

int64_t SourceTestBase::getSourcePts(void)
{
  return mpSource ? mpSource->getSourcePts() : ISource::getSourcePts();
}

AudioFormat SourceTestBase::getAudioFormat(void)
{
  return mpSource ? mpSource->getAudioFormat() : ISource::getAudioFormat();
}

void FilterCapture::process(AudioBuffer& inBuf, AudioBuffer& outBuf)
{
  outBuf = inBuf;

  setCaptureBufferSize( inBuf.getNumberOfSamples() * 3 );
  setCaptureAudioFormat( inBuf.getAudioFormat() );
  enqueToRefBuf( inBuf );
}

IInjector::IInjector( AudioFormat format ) : mInjectorEnabled(false)
{
  mpInjectorBuf = std::make_shared<FifoBuffer>( format );
}

IInjector::~IInjector()
{
  unlock();
  mpInjectorBuf.reset();
}

void IInjector::dequeFromInjectBuf(IAudioBuffer& buf)
{
  if( mpInjectorBuf ){
    mpInjectorBuf->read( buf );
  }
}

void IInjector::setInjectBufferSize(int nSamples)
{
  if( mpInjectorBuf ){
    mpInjectorBuf->setFifoSizeLimit( nSamples );
  }
}

void IInjector::inject(IAudioBuffer& buf)
{
  if( mpInjectorBuf ){
    mpInjectorBuf->write( buf );
  }
}

void IInjector::setInjectAudioFormat(AudioFormat audioFormat)
{
  if( mpInjectorBuf ){
    mpInjectorBuf->setAudioFormat( audioFormat );
  }
}

void IInjector::unlock(void)
{
  if( mpInjectorBuf ){
    mpInjectorBuf->unlock();
  }
}

void IInjector::setInjectorEnabled(bool bEnabled)
{
  mInjectorEnabled = bEnabled;
}

bool IInjector::getInjectorEnabled(void)
{
  return mInjectorEnabled;
}

SourceInjector::SourceInjector(std::shared_ptr<ISource> pSource):SourceTestBase(pSource), IInjector()
{

}

SourceInjector::~SourceInjector()
{
}

void SourceInjector::readPrimitive(IAudioBuffer& buf)
{
  if( mpSource && !getInjectorEnabled() ){
    mpSource->readPrimitive( buf );
  } else {
    dequeFromInjectBuf( buf );
  }
}

void FilterInjector::process(AudioBuffer& inBuf, AudioBuffer& outBuf)
{
  if( !getInjectorEnabled() ){
    outBuf = inBuf;
  } else {
    dequeFromInjectBuf( outBuf );
  }
}