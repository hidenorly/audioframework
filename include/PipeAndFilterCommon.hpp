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
#include <string>

class IUnlockable
{
public:
  virtual void unlock(void) = 0;
};

class IMutable
{
protected:
  bool mMuteEnabled;
  bool mMuteUseZeroEnabled;

protected:
  virtual bool getUseZeroEnabledInMute(void){ return mMuteUseZeroEnabled; };
  virtual void mutePrimitive(bool bEnableMute, bool bUseZero=false){};

public:
  IMutable():mMuteEnabled(false), mMuteUseZeroEnabled(false){};
  virtual ~ IMutable(){};

  virtual bool getMuteEnabled(void){ return mMuteEnabled; };
  virtual void setMuteEnabled(bool bEnableMute, bool bUseZero=false){
    bool bChanged = ( bEnableMute != mMuteEnabled );
    mMuteEnabled = bEnableMute;
    mMuteUseZeroEnabled = bUseZero;
    if( bChanged ){
      mutePrimitive(bEnableMute, bUseZero);
    }
  }
};

class ISourceSinkCommon : public AudioBase, public IResourceConsumer, public IMutable
{
public:
  ISourceSinkCommon():AudioBase(), IResourceConsumer(), IMutable(){};
  virtual ~ISourceSinkCommon(){};
  virtual std::string toString(void){ return std::string("ISourceSinkCommon"); };
  virtual AudioFormat getAudioFormat(void) = 0;
};

#endif /* __PIPEANDFILTER_COMMON_HPP__ */