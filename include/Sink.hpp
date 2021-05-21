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
#include "Buffer.hpp"
#include "AudioFormat.hpp"
#include "PlugInManager.hpp"
#include <string>
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
  int mLatencyUsec;
  int64_t mSinkPosition;

protected:
  virtual void writePrimitive(IAudioBuffer& buf) = 0;

public:
  virtual void write(IAudioBuffer& buf);

  virtual std::vector<PRESENTATION> getAvailablePresentations(void);
  virtual bool isAvailablePresentation(PRESENTATION presentation);

  virtual bool setPresentation(PRESENTATION presentation);
  virtual PRESENTATION getPresentation(void);

  virtual bool setAudioFormat(AudioFormat audioFormat) = 0;
  virtual AudioFormat getAudioFormat(void) = 0;

  virtual float getVolume(void);
  virtual bool setVolume(float volumePercentage);

  virtual int getLatencyUSec(void);
  virtual int64_t getSinkPts(void);

  virtual void dump(void) = 0;
};

class Sink : public ISink
{
protected:
  IAudioBuffer* mpBuf;

public:
  Sink();
  virtual ~Sink();
  virtual void writePrimitive(IAudioBuffer& buf);
  virtual std::string toString(void){ return "Sink"; };
  virtual void dump(void);
  virtual bool setAudioFormat(AudioFormat audioFormat);
  virtual AudioFormat getAudioFormat(void);
};

class SinkPlugIn : public ISink, public IPlugIn
{
public:
  SinkPlugIn();
  virtual ~SinkPlugIn();

  virtual void onLoad(void);
  virtual void onUnload(void);
  virtual std::string getId(void);
  virtual IPlugIn* newInstance(void);

  virtual void writePrimitive(IAudioBuffer& buf);
  virtual bool setAudioFormat(AudioFormat audioFormat);
  virtual AudioFormat getAudioFormat(void);
  virtual void dump(void);
};

typedef TPlugInManager<SinkPlugIn> SinkManager;

#endif /* __SINK_HPP__ */