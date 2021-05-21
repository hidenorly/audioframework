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

#ifndef __PIPEDSINK_HPP__
#define __PIPEDSINK_HPP__

#include "Sink.hpp"
#include "Buffer.hpp"
#include "InterPipeBridge.hpp"
#include "AudioFormat.hpp"
#include "Pipe.hpp"
#include <string>

class PipedSink : public ISink
{
protected:
  ISink* mpSink;
  IPipe* mpPipe;
  InterPipeBridge* mpInterPipeBridge;

public:
  PipedSink();
  virtual ~PipedSink();

  virtual ISink* attachSink(ISink* pSink);
  virtual ISink* detachSink(void);

  virtual void addFilterToHead(Filter* pFilter);
  virtual void addFilterToTail(Filter* pFilter);
  virtual void run(void);
  virtual void stop(void);
  virtual bool isRunning(void);
  virtual void clearFilters(void);

  virtual void writePrimitive(IAudioBuffer& buf);
  virtual std::string toString(void){ return "PipedSink"; };
  virtual void dump(void);

  virtual bool setAudioFormat(AudioFormat audioFormat);
  virtual AudioFormat getAudioFormat(void);
};

#endif /* __PIPEDSINK_HPP__ */