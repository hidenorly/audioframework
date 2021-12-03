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

#ifndef __PIPEANDFILTER_COMMON_HPP__
#define __PIPEANDFILTER_COMMON_HPP__

#include "AudioFormat.hpp"
#include "ResourceManager.hpp"
#include "Volume.hpp"
#include <string>

/* block-able class should implement this */
class IUnlockable
{
public:
  /* when stopping thread, the thread might be blocked. then use this to unblock it.  */
  virtual void unlock(void) = 0;
};

/* mute-able class should implement this */
class IMuteable
{
protected:
  bool mMuteEnabled;
  bool mMuteUseZeroEnabled;

protected:
  virtual bool getUseZeroEnabledInMute(void){ return mMuteUseZeroEnabled; };
  virtual void mutePrimitive(bool bEnableMute, bool bUseZero=false){};

public:
  IMuteable():mMuteEnabled(false), mMuteUseZeroEnabled(false){};
  virtual ~ IMuteable(){};

  /*
    @desc get mute state
    @return true: muted, false:not muted.
  */
  virtual bool getMuteEnabled(void){ return mMuteEnabled; };

  /*
    @desc Mute control
    @arg bEnableMute. true: mute, false: unmute
    @arg bUseZero. true: use zero data during mute is enabled.
  */
  virtual void setMuteEnabled(bool bEnableMute, bool bUseZero=false);
};

/* Common interfaces on ISink and ISource */
class ISourceSinkCommon : public AudioBase, public IResourceConsumer, public IMuteable
{
protected:
  float mVolume;
  bool mIsPerChannelVolume;
  std::vector<float> mPerChannelVolumes;

public:
  ISourceSinkCommon():AudioBase(), IResourceConsumer(), IMuteable(), mVolume(100.0f), mIsPerChannelVolume(false){};
  virtual ~ISourceSinkCommon(){};
  virtual std::string toString(void){ return std::string("ISourceSinkCommon"); };
  virtual AudioFormat getAudioFormat(void) = 0;
  virtual long getPresentationTime(void){ return 0;}; // usec

  virtual float getVolume(void);
  virtual bool setVolume(float volumePercentage);
  virtual bool setVolume(Volume::CHANNEL_VOLUME perChannelVolumes);
  virtual bool setVolume(std::vector<float> perChannelVolumes);
};

#endif /* __PIPEANDFILTER_COMMON_HPP__ */