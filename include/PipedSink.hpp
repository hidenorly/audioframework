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
#include <memory>

class PipedSink : public ISink
{
protected:
  std::shared_ptr<ISink> mpSink;
  std::shared_ptr<IPipe> mpPipe;
  std::shared_ptr<InterPipeBridge> mpInterPipeBridge;
  virtual void setAudioFormatPrimitive(AudioFormat audioFormat);

public:
  PipedSink( std::shared_ptr<ISink> pSink = nullptr );
  virtual ~PipedSink();

  virtual std::shared_ptr<ISink> attachSink(std::shared_ptr<ISink> pSink);
  virtual std::shared_ptr<ISink> detachSink(void);

  virtual void addFilterToHead(std::shared_ptr<IFilter> pFilter);
  virtual void addFilterToTail(std::shared_ptr<IFilter> pFilter);
  virtual void run(void);
  virtual void stop(void);
  virtual bool isRunning(void);
  virtual void clearFilters(void);
  virtual void writePrimitive(IAudioBuffer& buf);

  virtual std::string toString(void){ return std::string("PipedSink(") + (mpSink ? mpSink->toString() : "") + ")"; };
  virtual void dump(void);

  virtual AudioFormat getAudioFormat(void);
  virtual std::vector<AudioFormat> getSupportedAudioFormats(void);
  virtual bool isAvailableFormat(AudioFormat format);

  virtual int stateResourceConsumption(void);
};

#endif /* __PIPEDSINK_HPP__ */