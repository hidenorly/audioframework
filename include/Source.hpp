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

#ifndef __SOURCE_HPP__
#define __SOURCE_HPP__

#include <string>
#include "Buffer.hpp"
#include "AudioFormat.hpp"
#include "PipeAndFilterCommon.hpp"
#include "PlugInManager.hpp"

class SourceCapture;
class SourceInjector;

class ISource : public ISourceSinkCommon
{
protected:
  int mLatencyUsec;
  int64_t mSourcePosition;

protected:
  virtual void readPrimitive(IAudioBuffer& buf) = 0;

public:
  ISource();
  virtual ~ISource();
  virtual void read(IAudioBuffer& buf);
  virtual int getLatencyUSec(void);
  virtual int64_t getSourcePts(void);
  virtual AudioFormat getAudioFormat(void);

  friend SourceCapture;
  friend SourceInjector;
};

class Source : public ISource
{
protected:
  virtual void readPrimitive(IAudioBuffer& buf);

public:
  Source();
  virtual ~Source();
  virtual std::string toString(void){return "Source";};
};

class SourcePlugIn : public ISource, public IPlugIn
{
protected:
  virtual void readPrimitive(IAudioBuffer& buf);

public:
  SourcePlugIn();
  virtual ~SourcePlugIn();

  virtual void onLoad(void);
  virtual void onUnload(void);
  virtual std::string getId(void);
  virtual IPlugIn* newInstance(void);
};

typedef TPlugInManager<SourcePlugIn> SourceManager;

#endif /* __SOURCE_HPP__ */