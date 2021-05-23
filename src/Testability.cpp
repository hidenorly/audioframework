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

ICapture::ICapture(AudioFormat format)
{
  mpRefBuf = new FifoBufferReadReference( format );
}

ICapture::~ICapture()
{
  delete mpRefBuf;
  mpRefBuf = nullptr;
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

SinkCapture::SinkCapture(ISink* pSink) : ISink(), mpSink(pSink), ICapture( pSink ? pSink->getAudioFormat() : AudioFormat() )
{
}

SinkCapture::~SinkCapture()
{
  ICapture::~ICapture();
	delete mpSink;
	mpSink = nullptr;
}

void SinkCapture::writePrimitive(IAudioBuffer& buf)
{
  if( mpSink ){
    mpSink->_testWritePrimitive( buf );
  }
  enqueToRefBuf( buf );
}

std::vector<ISink::PRESENTATION> SinkCapture::getAvailablePresentations(void)
{
  if( mpSink ){
    return mpSink->getAvailablePresentations();
  }

  return ISink::getAvailablePresentations();
}

bool SinkCapture::isAvailablePresentation(PRESENTATION presentation)
{
  if( mpSink ){
    return mpSink->isAvailablePresentation( presentation );
  }

  return ISink::isAvailablePresentation( presentation );
}

bool SinkCapture::setPresentation(PRESENTATION presentation)
{
  if( mpSink ){
    return mpSink->setPresentation( presentation );
  }
  return ISink::setPresentation( presentation );
}

ISink::PRESENTATION SinkCapture::getPresentation(void)
{
 if( mpSink ){
    return mpSink->getPresentation();
  }
  return ISink::getPresentation();
}

bool SinkCapture::setAudioFormat(AudioFormat audioFormat)
{
 if( mpSink ){
    return mpSink->setAudioFormat( audioFormat );
  }
  setCaptureAudioFormat( audioFormat );

  return false;
}

AudioFormat SinkCapture::getAudioFormat(void)
{
 if( mpSink ){
    return mpSink->getAudioFormat();
  }
  return AudioFormat();
}

float SinkCapture::getVolume(void)
{
 if( mpSink ){
    return mpSink->getVolume();
  }
  return ISink::getVolume();
}

bool SinkCapture::setVolume(float volumePercentage)
{
 if( mpSink ){
    return mpSink->setVolume( volumePercentage );
  }
  return ISink::setVolume( volumePercentage );
}

int SinkCapture::getLatencyUSec(void)
{
 if( mpSink ){
    return mpSink->getLatencyUSec();
  }
  return ISink::getLatencyUSec();
}

int64_t SinkCapture::getSinkPts(void)
{
 if( mpSink ){
    return mpSink->getSinkPts();
  }
  return ISink::getSinkPts();
}

void SinkCapture::dump(void)
{
 if( mpSink ){
    return mpSink->dump();
  }
}


SourceCapture::SourceCapture(ISource* pSource) : ISource(), mpSource(pSource), ICapture( pSource ? pSource->getAudioFormat() : AudioFormat() )
{

}

SourceCapture::~SourceCapture()
{
  if( mpSource ){
    delete mpSource; mpSource = nullptr;
  }
}

void SourceCapture::readPrimitive(IAudioBuffer& buf)
{
  if( mpSource ){
    mpSource->_testReadPrimitive( buf );
  }
  enqueToRefBuf( buf );
}

int SourceCapture::getLatencyUSec(void)
{
  if( mpSource ){
    return mpSource->getLatencyUSec();
  }
  return ISource::getLatencyUSec();
}

int64_t SourceCapture::getSourcePts(void)
{
  if( mpSource ){
    return mpSource->getSourcePts();
  }
  return ISource::getSourcePts();
}

AudioFormat SourceCapture::getAudioFormat(void)
{
  if( mpSource ){
    return mpSource->getAudioFormat();
  }
  return ISource::getAudioFormat();
}


