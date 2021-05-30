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
#include "PlugInManager.hpp"
#include "ResourceManager.hpp"

class IFilter : public AudioBase, public IResourceConsumer
{
public:
  static inline const int DEFAULT_WINDOW_SIZE_USEC = 5000; // 5msec

  IFilter();
  virtual ~IFilter();
  virtual void process(AudioBuffer& inBuf, AudioBuffer& outBuf);
  // per-process() window size
  virtual int getRequiredWindowSizeUsec(void);
  // per-process() latency
  virtual int getLatencyUSec(void);
  // per-sample() processing time
  virtual int getExpectedProcessingUSec(void) = 0;
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
};

class FilterPlugIn : public Filter, public IPlugIn
{
public:
  FilterPlugIn();
  virtual ~FilterPlugIn();

  virtual void onLoad(void);
  virtual void onUnload(void);
  virtual std::string getId(void);
  virtual IPlugIn* newInstance(void);
};

typedef TPlugInManager<FilterPlugIn> FilterManager;

#endif /* __FILTER_HPP__ */