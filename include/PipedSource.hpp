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

#ifndef __PIPEDSOURCE_HPP__
#define __PIPEDSOURCE_HPP__

#include "Source.hpp"
#include "Buffer.hpp"
#include "InterPipeBridge.hpp"
#include "AudioFormat.hpp"
#include "Pipe.hpp"
#include <string>

class PipedSource : public ISource
{
protected:
  ISource* mpSource;
  IPipe* mpPipe;
  InterPipeBridge* mpInterPipeBridge;

public:
  PipedSource();
  virtual ~PipedSource();

  virtual ISource* attachSource(ISource* pSource);
  virtual ISource* detachSource(void);

  virtual void addFilterToHead(std::shared_ptr<IFilter> pFilter);
  virtual void addFilterToTail(std::shared_ptr<IFilter> pFilter);
  virtual void run(void);
  virtual void stop(void);
  virtual bool isRunning(void);
  virtual void clearFilters(void);

  virtual void readPrimitive(IAudioBuffer& buf);
  virtual std::string toString(void){ return "PipedSource"; };

  virtual AudioFormat getAudioFormat(void);

  virtual int stateResourceConsumption(void);
};

#endif /* __PIPEDSOURCE_HPP__ */