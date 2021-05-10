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

#include "PipeAndFilterCommon.hpp"
#include <string>
#include "Buffer.hpp"
#include "AudioFormat.hpp"
#include <vector>

class ISink : public ISourceSinkCommon
{
public:
  ISink();
  virtual ~ISink(){};

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

protected:
  PRESENTATION mPresentation;
  float mVolume;

public:
  virtual void write(AudioBuffer& buf) = 0;

  virtual std::vector<PRESENTATION> getAvailablePresentations(void);
  virtual bool isAvailablePresentation(PRESENTATION presentation);

  virtual bool setPresentation(PRESENTATION presentation);
  virtual PRESENTATION getPresentation(void);

  virtual bool setAudioFormat(AudioFormat audioFormat) = 0;
  virtual AudioFormat getAudioFormat(void) = 0;

  virtual float getVolume(void);
  virtual bool setVolume(float volumePercentage);

  virtual void dump(void) = 0;
};

class Sink : public ISink, public AudioBase
{
protected:
  AudioBuffer mBuf;

public:
  Sink(){};
  virtual ~Sink(){};
  virtual void write(AudioBuffer& buf);
  virtual std::string toString(void){ return "Sink"; };
  virtual void dump(void);
  virtual bool setAudioFormat(AudioFormat audioFormat);
  virtual AudioFormat getAudioFormat(void);
};

#endif /* __SINK_HPP__ */