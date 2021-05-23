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

#include "Sink.hpp"
#include "Util.hpp"
#include "Volume.hpp"

ISink::ISink() : mVolume(100.0f), mLatencyUsec(0), mSinkPosition(0)
{

}

std::vector<ISink::PRESENTATION> ISink::getAvailablePresentations(void)
{
  std::vector<PRESENTATION> supportedPresentations;
  supportedPresentations.push_back( PRESENTATION_DEFAULT );
  return supportedPresentations;
}

bool ISink::isAvailablePresentation(ISink::PRESENTATION presentation)
{
  bool bResult = false;

  std::vector<PRESENTATION> presentations = getAvailablePresentations();
  for(auto& aPresentation : presentations){
    bResult |= (aPresentation == presentation);
    if( bResult ) break;
  }

  return bResult;
}

bool ISink::setPresentation(PRESENTATION presentation)
{
  bool bSuccess = isAvailablePresentation(presentation);

  if( bSuccess ){
    mPresentation = presentation;
  }

  return bSuccess;
}

ISink::PRESENTATION ISink::getPresentation(void)
{
  return mPresentation;
}

float ISink::getVolume(void)
{
  return mVolume;
}

bool ISink::setVolume(float volumePercentage)
{
  mVolume = volumePercentage;
  return true;
}

void ISink::write(IAudioBuffer& buf)
{
  int nSamples = 0;
  AudioFormat format;
  AudioBuffer* pBuf = dynamic_cast<AudioBuffer*>(&buf);
  if( pBuf ){
    nSamples = pBuf->getSamples();
    format = pBuf->getAudioFormat();
    if( nSamples ){
      mLatencyUsec = 1000000 * nSamples / format.getSamplingRate();
    }
    mSinkPosition += (mLatencyUsec ? mLatencyUsec : buf.getRawBuffer().size());
  } else {
    mSinkPosition += buf.getRawBuffer().size();
  }
  if( 100.0f == mVolume || !pBuf ){
    writePrimitive( buf );
  } else {
    // pBuf is already checked in the above
    AudioBuffer volumedBuf( format, nSamples );
    if( Volume::process( pBuf, &volumedBuf, mVolume ) ){
      writePrimitive( volumedBuf );
    } else {
      writePrimitive( buf );
    }
  }
}

int ISink::getLatencyUSec(void)
{
  return mLatencyUsec;
}

int64_t ISink::getSinkPts(void)
{
  return mSinkPosition;
}


void ISink::_testWritePrimitive(IAudioBuffer& buf)
{
  writePrimitive( buf );
}


Sink::Sink():ISink()
{
  mpBuf = new AudioBuffer();
}

Sink::~Sink()
{
  delete mpBuf;
  mpBuf = nullptr;
}


void Sink::writePrimitive(IAudioBuffer& buf)
{
  if( mpBuf ){
    mpBuf->append( buf );
  }
}

void Sink::dump(void)
{
  Util::dumpBuffer("Dump Sink data", mpBuf);
}

bool Sink::setAudioFormat(AudioFormat audioFormat)
{
  bool bSuccess = isAvailableFormat(audioFormat);

  if( bSuccess ) {
    if( mpBuf ){
      mpBuf->setAudioFormat( audioFormat );
    }
  }

  return bSuccess;
}

AudioFormat Sink::getAudioFormat(void)
{
  return mpBuf ? mpBuf->getAudioFormat() : AudioFormat();
}

SinkPlugIn::SinkPlugIn()
{

}

SinkPlugIn::~SinkPlugIn()
{

}

void SinkPlugIn::onLoad(void)
{

}

void SinkPlugIn::onUnload(void)
{

}

std::string SinkPlugIn::getId(void)
{
  return "SinkPlugInBase";
}

IPlugIn* SinkPlugIn::newInstance(void)
{
  return new SinkPlugIn();
}

void SinkPlugIn::writePrimitive(IAudioBuffer& buf)
{

}

bool SinkPlugIn::setAudioFormat(AudioFormat audioFormat)
{
  return false;
}

AudioFormat SinkPlugIn::getAudioFormat(void)
{
  return AudioFormat();
}

void SinkPlugIn::dump(void)
{

}