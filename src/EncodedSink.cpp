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

#include "EncodedSink.hpp"
#include "Util.hpp"

EncodedSink::EncodedSink(std::shared_ptr<ISink> pSink, bool bTranscode):ISink(), mpSink(pSink), mbTranscode(bTranscode), mpDecoder(nullptr), mpEncoder(nullptr)
{
  mpBuf = new CompressAudioBuffer( AudioFormat(AudioFormat::ENCODING::COMPRESSED), 0 );
}

EncodedSink::~EncodedSink()
{
  if( mpBuf ){
    delete mpBuf; mpBuf = nullptr;
  }
  mpSink.reset();
  mpDecoder.reset();
  mpEncoder.reset();
}

std::shared_ptr<ISink> EncodedSink::attachSink(std::shared_ptr<ISink> pSink)
{
  std::shared_ptr<ISink> pPrevSink = mpSink;
  mpSink = pSink;
  return pPrevSink;
}

std::shared_ptr<ISink> EncodedSink::detachSink(void)
{
  std::shared_ptr<ISink> pPrevSink = mpSink;
  mpSink.reset();
  return pPrevSink;
}

void EncodedSink::clearSink()
{
  mpSink.reset();
}

void EncodedSink::writePrimitive(IAudioBuffer& buf)
{
  if( mpSink ){
    mpSink->write( buf );
  } else {
    if( mpBuf ){
      mpBuf->append( buf );
    }
  }
}

bool EncodedSink::setAudioFormat(AudioFormat audioFormat)
{
  bool result = false;
  if( mpSink ){
    result = mpSink->setAudioFormat( audioFormat );
  }
  return result;
}

AudioFormat EncodedSink::getAudioFormat(void)
{
  return mpSink ? mpSink->getAudioFormat() : AudioFormat();
}

std::vector<AudioFormat> EncodedSink::getSupportedAudioFormats(void)
{
  std::vector<AudioFormat> result;
  if( mpSink ){
    result = mpSink->getSupportedAudioFormats();
  }
  return result;
}

float EncodedSink::getVolume(void)
{
  return mpSink ? mpSink->getVolume() : 0.0f;
}

bool EncodedSink::setVolume(float volumePercentage)
{
  return mpSink ? mpSink->setVolume(volumePercentage) : false;
}

bool EncodedSink::setVolume(Volume::CHANNEL_VOLUME perChannelVolumes)
{
  return mpSink ? mpSink->setVolume(perChannelVolumes) : false;
}

bool EncodedSink::setVolume(std::vector<float> perChannelVolumes)
{
  return mpSink ? mpSink->setVolume(perChannelVolumes) : false;
}

std::vector<ISink::PRESENTATION> EncodedSink::getAvailablePresentations(void)
{
  std::vector<ISink::PRESENTATION> result;
  if( mpSink ){
    result = mpSink->getAvailablePresentations();
  }
  return result;
}

bool EncodedSink::isAvailablePresentation(ISink::PRESENTATION presentation)
{
  return mpSink ? mpSink->isAvailablePresentation(presentation) : false;
}

bool EncodedSink::setPresentation(ISink::PRESENTATION presentation)
{
  return mpSink ? mpSink->setPresentation(presentation) : false;
}

ISink::PRESENTATION EncodedSink::getPresentation(void)
{
  return mpSink ? mpSink->getPresentation() : ISink::PRESENTATION();
}

int EncodedSink::getLatencyUSec(void)
{
  return mpSink ? mpSink->getLatencyUSec() : 0;
}

int64_t EncodedSink::getSinkPts(void)
{
  return mpSink ? mpSink->getSinkPts() : 0;
}

void EncodedSink::dump(void)
{
  if( mpSink ){
    mpSink->dump();
  } else {
    Util::dumpBuffer("Dump Sink data", mpBuf);
  }
}

int EncodedSink::stateResourceConsumption(void)
{
  return mpSink ? mpSink->stateResourceConsumption() : 0;
}
