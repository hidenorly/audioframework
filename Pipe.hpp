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

#ifndef __PIPE_HPP__
#define __PIPE_HPP__

#include "Filter.hpp"
#include "Source.hpp"
#include "Sink.hpp"
#include <vector>
#include <mutex>
#include <thread>


class IPipe
{
public:
  IPipe(){};
  virtual ~IPipe(){};

  virtual void addFilterToHead(Filter* pFilter) = 0;
  virtual void addFilterToTail(Filter* pFilter) = 0;

  virtual ISink* attachSink(ISink* pSink) = 0;
  virtual ISink* detachSink(void) = 0;
  virtual ISource* attachSource(ISource* pSource) = 0;
  virtual ISource* detachSource(void) = 0;

  virtual void run(void) = 0;
  virtual void stop(void) = 0;
  virtual bool isRunning(void) = 0;

  virtual void dump(void) = 0;
  virtual void clearFilers(void) = 0;
  virtual AudioFormat getFilterAudioFormat(void) = 0;
  virtual int getWindowSizeUsec(void) = 0;
};

class Pipe : public IPipe
{
public:
  Pipe();
  virtual ~Pipe();

  virtual void addFilterToHead(Filter* pFilter);
  virtual void addFilterToTail(Filter* pFilter);

  virtual ISink* attachSink(ISink* pSink);
  virtual ISink* detachSink(void);
  virtual ISource* attachSource(ISource* pSource);
  virtual ISource* detachSource(void);

  virtual void run(void);
  virtual void stop(void);
  virtual bool isRunning(void);

  virtual void dump(void);
  virtual void clearFilers(void);
  virtual AudioFormat getFilterAudioFormat(void);

protected:
  // Should override process() if you want to support different window size processing by several threads, etc.
  virtual void process(void);
  static void _execute(Pipe* pThis);
  // Should override getFilterAudioFormat() if you want to use different algorithm to choose using Audioformat
  int getCommonWindowSizeUsec(void);

  std::vector<Filter*> mFilters;
  ISink* mpSink;
  ISource* mpSource;

  std::atomic<bool> mbIsRunning;
  std::vector<std::thread> mThreads;
  std::mutex mMutexThreads;

public:
  virtual int getWindowSizeUsec(void){ return getCommonWindowSizeUsec(); };
};

#endif /* __PIPE_HPP__ */