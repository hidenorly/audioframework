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

ISink::ISink() : ISourceSinkCommon(), mVolume(100.0f), mIsPerChannelVolume(false), mLatencyUsec(0), mSinkPosition(0)
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
  return getMuteEnabled() ? 0.0f : !mIsPerChannelVolume ? mVolume : Volume::getVolumeMax(mPerChannelVolumes);
}

bool ISink::setVolume(float volumePercentage)
{
  mIsPerChannelVolume = false;
  mVolume = volumePercentage;
  return true;
}

bool ISink::setVolume(Volume::CHANNEL_VOLUME perChannelVolumes)
{
  return setVolume( Volume::getPerChannelVolumes(getAudioFormat(), perChannelVolumes) );
}

bool ISink::setVolume(std::vector<float> perChannelVolumes)
{
  mIsPerChannelVolume = true;
  mPerChannelVolumes = perChannelVolumes;
  return true;
}


void ISink::mutePrimitive(bool bEnableMute, bool bUseZero)
{
}

void ISink::write(IAudioBuffer& buf)
{
  int nSamples = 0;
  AudioFormat format;
  AudioBuffer* pBuf = dynamic_cast<AudioBuffer*>(&buf);
  if( pBuf ){
    // AudioBuffer instance
    nSamples = pBuf->getNumberOfSamples();
    format = pBuf->getAudioFormat();
    if( nSamples ){
      mLatencyUsec = 1000000 * nSamples / format.getSamplingRate();
    }
    mSinkPosition += (mLatencyUsec ? mLatencyUsec : buf.getRawBuffer().size());
  } else {
    // CompressedAudioBuffer instance
    mSinkPosition += buf.getRawBuffer().size();
  }
  if( !getMuteEnabled() ){
    if( (!mIsPerChannelVolume && (100.0f == mVolume)) || (mIsPerChannelVolume && !Volume::isVolumeRequired(mPerChannelVolumes) ) || !pBuf ){
      writePrimitive( buf );
    } else {
      // pBuf is already checked in the above
      AudioBuffer volumedBuf( format, nSamples );
      if( (!mIsPerChannelVolume && Volume::process( pBuf, &volumedBuf, mVolume )) || (mIsPerChannelVolume && Volume::process( pBuf, &volumedBuf, mPerChannelVolumes )) ){
        writePrimitive( volumedBuf );
      } else {
        writePrimitive( buf );
      }
    }
  } else if ( getUseZeroEnabledInMute() ) {
    // mute enabled && zero out enabled
    AudioBuffer zeroBuffer( format, nSamples );
    ByteBuffer rawZeroBuffer( zeroBuffer.getRawBuffer().size(), 0 );
    zeroBuffer.setRawBuffer( rawZeroBuffer );
    writePrimitive( zeroBuffer );
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

std::shared_ptr<IPlugIn> SinkPlugIn::newInstance(void)
{
  return std::make_shared<SinkPlugIn>();
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
