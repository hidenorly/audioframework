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
#include "Pipe.hpp"
#include "InterPipeBridge.hpp"
#include "AudioFormat.hpp"
#include "ThreadBase.hpp"
#include <vector>
#include <mutex>
#include <thread>
#include <map>
#include <memory>

class PipeMixer : public ThreadBase
{
protected:
  std::mutex mMutexPipe;
  std::vector<std::shared_ptr<InterPipeBridge>> mpInterPipeBridges;
  std::map<std::shared_ptr<InterPipeBridge>, std::weak_ptr<IPipe>> mpPipes;
  AudioFormat mFormat;
  std::shared_ptr<ISink> mpSink;

  virtual void process(void);
  virtual void unlockToStop(void);
  std::shared_ptr<ISink> getSinkFromPipe(std::shared_ptr<IPipe> pArgPipe);
  bool isPipeRunningOrNotRegistered(std::shared_ptr<InterPipeBridge> srcSink);

public:
  PipeMixer(AudioFormat format = AudioFormat(), std::shared_ptr<ISink> pSink = nullptr );
  virtual ~PipeMixer();
  virtual bool setAudioFormat(AudioFormat audioFormat);
  virtual AudioFormat getAudioFormat(void);

  virtual std::shared_ptr<ISink> attachSink(std::shared_ptr<ISink> pSink);
  virtual std::shared_ptr<ISink> detachSink(void);

  virtual void attachSinkAdaptor(std::shared_ptr<InterPipeBridge> pSource, std::shared_ptr<IPipe> pPipe = nullptr);
  virtual std::shared_ptr<ISink> allocateSinkAdaptor(std::shared_ptr<IPipe> pPipe = nullptr);
  virtual void releaseSinkAdaptor(std::shared_ptr<ISink> pSink);
  virtual std::vector<std::shared_ptr<ISink>> getSinkAdaptors(void);
};

#endif /* __PIPEMIXER_HPP__ */