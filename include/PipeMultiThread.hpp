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

class PipeMultiThread : public IPipe
{
public:
  PipeMultiThread();
  virtual ~PipeMultiThread();

  virtual void addFilterToHead(IFilter* pFilter);
  virtual void addFilterToTail(IFilter* pFilter);
  virtual void addFilterAfterFilter(IFilter* pFilter, IFilter* pPosition);
  virtual bool isFilterIncluded(IFilter* pFilter);

  virtual ISink* attachSink(ISink* pSink);
  virtual ISink* detachSink(void);
  virtual ISource* attachSource(ISource* pSource);
  virtual ISource* detachSource(void);

  virtual void run(void);
  virtual void stop(void);
  virtual bool isRunning(void);

  virtual void dump(void);
  virtual void clearFilters(void);

  virtual AudioFormat getFilterAudioFormat(void);
  virtual int getWindowSizeUsec(void);
  virtual int getLatencyUSec(void);
  virtual int stateResourceConsumption(void);

protected:
  IPipe* getHeadPipe(bool bCreateInstance = false);
  IPipe* getTailPipe(bool bCreateInstance = false);
  IPipe* findPipe(IFilter* pFilter);
  std::vector<IPipe*> mPipes;
  std::vector<InterPipeBridge*> mInterPipeBridges;
  ISink* mpSink;
  ISource* mpSource;
  bool mSinkAttached;
  bool mSourceAttached;

  void createAndConnectPipesToTail(IPipe* pCurrentPipe);
  void createAndConnectPipesToHead(IPipe* pCurrentPipe);
  void ensureSourceSink(void);

  std::mutex mMutexThreads;
  std::mutex mMutexFilters;
};

#endif /* __PIPEMANAGER_HPP__ */