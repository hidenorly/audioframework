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

#include "Source.hpp"
#include <iostream>

ISource::ISource():ISourceSinkCommon(), mLatencyUsec(0), mSourcePosition(0)
{

}

ISource::~ISource()
{

}

void ISource::read(IAudioBuffer& buf)
{
  AudioBuffer* pBuf = dynamic_cast<AudioBuffer*>(&buf);
  if( pBuf ){
    int nSamples = pBuf->getNumberOfSamples();
    AudioFormat format = pBuf->getAudioFormat();
    if( nSamples ){
      mLatencyUsec = 1000000 * nSamples / format.getSamplingRate();
    }
    mSourcePosition += (mLatencyUsec ? mLatencyUsec : buf.getRawBuffer().size());
  } else {
    mSourcePosition += buf.getRawBuffer().size();
  }
  if( !getMuteEnabled() ){
    readPrimitive(buf);
  } else if ( getUseZeroEnabledInMute() ) {
    // mute enabled && use zero enabled
    ByteBuffer rawZeroBuffer( buf.getRawBuffer().size(), 0 );
    buf.setRawBuffer( rawZeroBuffer );
  }
}

int ISource::getLatencyUSec(void)
{
  return mLatencyUsec;
}

int64_t ISource::getSourcePts(void)
{
  return mSourcePosition;
}

AudioFormat ISource::getAudioFormat(void)
{
  return AudioFormat();
}


Source::Source():ISource()
{

}

Source::~Source()
{

}

void Source::readPrimitive(IAudioBuffer& buf)
{
  ByteBuffer rawBuffer = buf.getRawBuffer();
  ByteBuffer bufZero(rawBuffer.size(), 0);
  uint8_t* ptr = bufZero.data();
  for(int i=0; i<bufZero.size(); i++){
    *ptr++ = i % 256;
  }
  rawBuffer = bufZero;
  buf.setRawBuffer( rawBuffer );
}


SourcePlugIn::SourcePlugIn()
{

}

SourcePlugIn::~SourcePlugIn()
{

}

void SourcePlugIn::onLoad(void)
{

}

void SourcePlugIn::onUnload(void)
{

}

std::string SourcePlugIn::getId(void)
{
  return "SourcePlugInBase";
}

std::shared_ptr<IPlugIn> SourcePlugIn::newInstance(void)
{
  return std::make_shared<SourcePlugIn>();
}

void SourcePlugIn::readPrimitive(IAudioBuffer& buf)
{

}
