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

ISink::ISink() : mVolume(100.0f), mLatencyUsec(0)
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

void ISink::write(AudioBuffer& buf)
{
  int nSamples = buf.getSamples();
  AudioFormat format = buf.getAudioFormat();
  if( nSamples ){
    mLatencyUsec = 1000000 * nSamples / format.getSamplingRate();
  }
  if( 100.0f == mVolume ){
    writePrimitive( buf );
  } else {
    AudioBuffer volumedBuf( format, nSamples );
    Volume::process( &buf, &volumedBuf, mVolume );
    writePrimitive( volumedBuf );
  }
}

int ISink::getLatencyUSec(void)
{
  return mLatencyUsec;
}

Sink::Sink():ISink()
{

}

void Sink::writePrimitive(AudioBuffer& buf)
{
  mBuf.append( buf );
}

void Sink::dump(void)
{
  Util::dumpBuffer("Dump Sink data", mBuf);
}

bool Sink::setAudioFormat(AudioFormat audioFormat)
{
  bool bSuccess = isAvailableFormat(audioFormat);

  if( bSuccess ) {
    mBuf.setAudioFormat( audioFormat );
  }

  return bSuccess;
}

AudioFormat Sink::getAudioFormat(void)
{
  return mBuf.getAudioFormat();
}