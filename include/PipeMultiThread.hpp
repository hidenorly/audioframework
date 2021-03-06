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

#ifndef __PIPEMANAGER_HPP__
#define __PIPEMANAGER_HPP__

#include "Pipe.hpp"
#include "InterPipeBridge.hpp"
#include <vector>
#include <memory>

class PipeMultiThread : public IPipe
{
public:
  PipeMultiThread();
  virtual ~PipeMultiThread();

  virtual void addFilterToHead(std::shared_ptr<IFilter> pFilter);
  virtual void addFilterToTail(std::shared_ptr<IFilter> pFilter);
  virtual bool addFilterAfterFilter(std::shared_ptr<IFilter> pFilter, std::shared_ptr<IFilter> pPosition);
  virtual bool removeFilter(std::shared_ptr<IFilter> pFilter);
  virtual bool isFilterIncluded(std::shared_ptr<IFilter> pFilter);

  virtual std::shared_ptr<ISink> attachSink(std::shared_ptr<ISink> pSink);
  virtual std::shared_ptr<ISink> detachSink(void);
  virtual std::shared_ptr<ISink> getSinkRef(void);
  virtual std::shared_ptr<ISource> attachSource(std::shared_ptr<ISource> pSource);
  virtual std::shared_ptr<ISource> detachSource(void);
  virtual std::shared_ptr<ISource> getSourceRef(void);

  virtual void run(void);
  virtual void stop(void);
  virtual void stopAndFlush(void);
  virtual bool isRunning(void);

  virtual void dump(void);
  virtual void clearFilters(void);

  virtual AudioFormat getFilterAudioFormat(AudioFormat theUsingFormat = AudioFormat());
  virtual int getWindowSizeUsec(void);
  virtual int getLatencyUSec(void);
  virtual int stateResourceConsumption(void);

protected:
  std::shared_ptr<IPipe> getHeadPipe(bool bCreateInstance = false);
  std::shared_ptr<IPipe> getTailPipe(bool bCreateInstance = false);
  std::shared_ptr<IPipe> findPipe(std::shared_ptr<IFilter> pFilter);
  std::vector<std::shared_ptr<IPipe>> mPipes;
  std::vector<std::shared_ptr<InterPipeBridge>> mInterPipeBridges;
  std::shared_ptr<ISink> mpSink;
  std::shared_ptr<ISource> mpSource;
  bool mSinkAttached;
  bool mSourceAttached;

  void createAndConnectPipesToTail(std::shared_ptr<IPipe> pCurrentPipe);
  void createAndConnectPipesToHead(std::shared_ptr<IPipe> pCurrentPipe);
  void ensureSourceSink(void);

  std::mutex mMutexThreads;
  std::mutex mMutexFilters;
};

#endif /* __PIPEMANAGER_HPP__ */