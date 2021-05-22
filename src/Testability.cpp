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

SinkCapture::SinkCapture(ISink* pSink) : mpSink(pSink)
{

}

SinkCapture::~SinkCapture()
{
	delete mpSink;
	mpSink = nullptr;
}

void SinkCapture::write(IAudioBuffer& buf)
{
	if( mpSink ){
		mpSink->write( buf );
	}
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
