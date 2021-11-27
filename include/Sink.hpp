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
#include "Volume.hpp"
#include <string>
#include <vector>
#include <memory>

class SinkCapture;
class SinkInjector;
class ReferenceSoundSinkSource;

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
  bool mIsPerChannelVolume;
  std::vector<float> mPerChannelVolumes;
  int mLatencyUsec;
  int64_t mSinkPosition;

protected:
  virtual void writePrimitive(IAudioBuffer& buf) = 0;
  virtual void mutePrimitive(bool bEnableMute, bool bUseZero=false);

public:
  virtual void write(IAudioBuffer& buf);
  virtual void flush(void){};
  friend SinkCapture;
  friend SinkInjector;
  friend ReferenceSoundSinkSource;

  virtual std::vector<PRESENTATION> getAvailablePresentations(void);
  virtual bool isAvailablePresentation(PRESENTATION presentation);

  virtual bool setPresentation(PRESENTATION presentation);
  virtual PRESENTATION getPresentation(void);

  virtual float getVolume(void);
  virtual bool setVolume(float volumePercentage);
  virtual bool setVolume(Volume::CHANNEL_VOLUME perChannelVolumes);
  virtual bool setVolume(std::vector<float> perChannelVolumes);

  virtual int getLatencyUSec(void);
  virtual int64_t getSinkPts(void);
  virtual long getPresentationTime(void){ return getSinkPts(); };

  virtual void dump(void){};
  virtual int stateResourceConsumption(void){return 0;};
};

class Sink : public ISink
{
protected:
  std::shared_ptr<IAudioBuffer> mpBuf;

protected:
  void setAudioFormatPrimitive(AudioFormat format);
  virtual void writePrimitive(IAudioBuffer& buf);

public:
  Sink();
  virtual ~Sink();
  virtual std::string toString(void){ return "Sink"; };
  virtual void dump(void);
  virtual AudioFormat getAudioFormat(void);
};

class SinkPlugIn : public ISink, public IPlugIn
{
protected:
  virtual void setAudioFormatPrimitive(AudioFormat audioFormat);
  virtual void writePrimitive(IAudioBuffer& buf);

public:
  SinkPlugIn();
  virtual ~SinkPlugIn();

  virtual void onLoad(void);
  virtual void onUnload(void);
  virtual std::string getId(void);
  virtual std::shared_ptr<IPlugIn> newInstance(void);

  virtual AudioFormat getAudioFormat(void);
  virtual void dump(void);
};

typedef TPlugInManager<SinkPlugIn> SinkManager;

#endif /* __SINK_HPP__ */