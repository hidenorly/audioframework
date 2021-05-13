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

#ifndef __PIPEMIXER_HPP__
#define __PIPEMIXER_HPP__

#include "Sink.hpp"
#include "InterPipeBridge.hpp"
#include "AudioFormat.hpp"
#include "ThreadBase.hpp"
#include <vector>
#include <mutex>
#include <thread>

class PipeMixer : public ThreadBase
{
protected:
  AudioFormat mFormat;
  ISink* mpSink;
  std::vector<InterPipeBridge*> mpInterPipeBridges;

  virtual void process(void);

public:
  PipeMixer(AudioFormat format = AudioFormat(), ISink* pSink = nullptr );
  virtual ~PipeMixer();
  virtual bool setAudioFormat(AudioFormat audioFormat);
  virtual AudioFormat getAudioFormat(void);

  virtual ISink* attachSink(ISink* pSink);
  virtual ISink* detachSink(void);

  virtual ISink* allocateSinkAdaptor(void);
  virtual void releaseSinkAdaptor(ISink* pSink);
};

#endif /* __PIPEMIXER_HPP__ */