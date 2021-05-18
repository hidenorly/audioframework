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

#ifndef __ENCODER_HPP__
#define __ENCODER_HPP__

#include "Buffer.hpp"
#include "Sink.hpp"
#include "InterPipeBridge.hpp"
#include "ThreadBase.hpp"
#include <string>
#include <vector>
#include <thread>
#include "Media.hpp"

class IEncoder : public ThreadBase
{
protected:
  ISink* mpSink;
  std::vector<InterPipeBridge*> mpInterPipeBridges;
  virtual void unlockToStop(void);

public:
  IEncoder();
  virtual ~IEncoder();

  virtual void configure(MediaParam param);
  virtual void configure(std::vector<MediaParam> params);

  virtual ISink* allocateSinkAdaptor(void); // as encoder's source
  virtual void releaseSinkAdaptor(ISink* pSink);

  virtual ISink* attachSink(ISink* pSink);
  virtual ISink* detachSink(void);
  virtual int64_t getPosition(void);
};

class NullEncoder : public IEncoder
{
public:
  NullEncoder();
  ~NullEncoder();

  virtual void configure(MediaParam param);

protected:
  virtual void process(void);
};

#endif /* __ENCODER_HPP__ */