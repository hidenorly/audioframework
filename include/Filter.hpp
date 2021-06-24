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

#ifndef __FILTER_HPP__
#define __FILTER_HPP__

#include "Buffer.hpp"
#include "AudioFormat.hpp"
#include <vector>
#include <memory>
#include <string>
#include "PlugInManager.hpp"
#include "ResourceManager.hpp"

class IFilter : public AudioBase, public IResourceConsumer
{
public:
  static inline const int DEFAULT_WINDOW_SIZE_USEC = 5000; // 5msec

  IFilter();
  virtual ~IFilter();
  /* @desc this method should do filtering processing using inBuf and the result should be written to outBuf */
  virtual void process(AudioBuffer& inBuf, AudioBuffer& outBuf);
  /* @desc this method should return per-process() window size
     @return minimum window size usec
     If 5000 is returned, the in & out buffer's samples will be 240 at 48KHz ideal case.
     Note that Pipe implementation may pass n * getRequiredWindowSizeUsec()'s buffer (n: natural number) to adjust the other filters' window size requirement.
     In this case, the process() should handle it. */
  virtual int getRequiredWindowSizeUsec(void);
  /* @desc report per-process() latency
           report the latency in case of the reported getRequiredWindowSizeUsec()
    Note that in Pipe implementation, actual pipe total latency will be common window size of the filters + Sink latency + Source's latency.
     */
  virtual int getLatencyUSec(void);
  /* @desc report per-process() processing time
           report the latency in case of the reported getRequiredWindowSizeUsec() */
  virtual int getExpectedProcessingUSec(void) = 0;
  virtual std::string toString(void){ return "IFilter"; };
};

class Filter : public IFilter
{
public:
  static const int DEFAULT_REQUIRED_PROCESSING_RESOURCE = 100; // 0.1DMIPS

  Filter();
  virtual ~Filter();
  virtual std::vector<AudioFormat> getSupportedAudioFormats(void);
  // per-second processing resource (DMIPS*1000)
  virtual int stateResourceConsumption(void);
  virtual int getExpectedProcessingUSec(void);
  virtual std::string toString(void){ return "Filter"; };
};

class FilterPlugIn : public Filter, public IPlugIn
{
public:
  FilterPlugIn();
  virtual ~FilterPlugIn();

  /* @desc initialize at loading the filter plug-in shared object such as .so */
  virtual void onLoad(void);
  /* @desc uninitialize at unloading the filter plug-in shared object such as .so */
  virtual void onUnload(void);
  /* @desc report your filter plug-in's unique id
     @return unique plug-in id. may use uuid. */
  virtual std::string getId(void);
  /* @desc this is expected to use by strategy
     @return new YourFilter()'s result */
  virtual std::shared_ptr<IPlugIn> newInstance(void);
  virtual std::string toString(void){ return "FilterPlugIn"; };
};

typedef TPlugInManager<FilterPlugIn> FilterManager;

#endif /* __FILTER_HPP__ */