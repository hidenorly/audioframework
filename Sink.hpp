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

#ifndef __SINK_HPP__
#define __SINK_HPP__

#include <string>
#include "Buffer.hpp"
#include "Util.hpp"
#include "AudioFormat.hpp"
#include <vector>

class Sink : public AudioBase
{
protected:
  AudioBuffer mBuf;

public:
  Sink(){};
  virtual ~Sink(){};
  virtual void write(AudioBuffer& buf){
    ByteBuffer internalBuffer = mBuf.getRawBuffer();
    ByteBuffer externalBuffer = buf.getRawBuffer();

    int newSize = externalBuffer.size() + internalBuffer.size();
    internalBuffer.reserve( newSize );

    std::copy( externalBuffer.begin(), externalBuffer.end(), std::back_inserter( internalBuffer ) );
    mBuf.setRawBuffer( internalBuffer );
  };
  virtual std::string toString(void){return "Sink";}
  virtual void dump(void){
    Util::dumpBuffer("Dump Sink data", mBuf);
  }

  enum PRESENTATION {
    SPEAKER_MONO,
    SPEAKER_STEREO,
    SPEAKER_FL_FR_SL_SR,
    SPEAKER_FL_FR_C_SL_SR,
    SPEAKER_FL_FR_C_SL_SR_SW,
    SPEAKER_FL_FR_C_SL_SR_SW_EL_ER, // Atmos Enabled L/R
    HEADPHONE_MONO,
    HEADPHONE_STEREO,
    PRESENTATION_DEFAULT = SPEAKER_STEREO,
    UNKNOWN
  };

  virtual std::vector<PRESENTATION> getAvailablePresentations(void){
    std::vector<PRESENTATION> supportedPresentations;
    supportedPresentations.push_back( PRESENTATION_DEFAULT );
    return supportedPresentations;
  }

  bool isAvailablePresentation(PRESENTATION presentation)
  {
    bool bResult = false;

    std::vector<PRESENTATION> presentations = getAvailablePresentations();
    for(auto& aPresentation : presentations){
      bResult |= (aPresentation == presentation);
      if( bResult ) break;
    }

    return bResult;
  }

protected:
  PRESENTATION mPresentation;

public:
  bool setAudioFormat(AudioFormat audioFormat){
    bool bSuccess = isAvailableFormat(audioFormat);

    if( bSuccess ) {
      mBuf.setAudioFormat( audioFormat );
    }

    return bSuccess;
  }
  AudioFormat getAudioFormat(void){
    return mBuf.getAudioFormat();
  }

  bool setPresentation(PRESENTATION presentation){
    bool bSuccess = isAvailablePresentation(presentation);

    if( bSuccess ){
      mPresentation = presentation;
    }

    return bSuccess;
  }

  PRESENTATION getPresentation(void){
    return mPresentation;
  }

};

#endif /* __SINK_HPP__ */