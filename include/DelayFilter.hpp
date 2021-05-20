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

#ifndef __DELAYFILTER_HPP__
#define __DELAYFILTER_HPP__

#include "Filter.hpp"
#include "AudioFormat.hpp"
#include "FifoBuffer.hpp"
#include <map>
#include <mutex>

class DelayFilter : public IFilter
{
public:
  typedef std::map<AudioFormat::CH, int> ChannelDelay;

protected:
  int mProessSize;
  AudioFormat mAudioFormat;

  static const int DEFAULT_PROCESSING_TIME_USEC = 100; // 0.1msec

  ChannelDelay mChannelDelay;
  std::map<AudioFormat::CH, FifoBuffer*> mDelayBuf;

public:
  DelayFilter(AudioFormat audioFormat, ChannelDelay channelDelay);
  virtual ~DelayFilter();
  virtual std::vector<AudioFormat> getSupportedAudioFormats(void);

  virtual void process(AudioBuffer& inBuf, AudioBuffer& outBuf);

  virtual int getExpectedProcessingUSec(void);
  virtual int getRequiredWindowSizeUsec(void);
};


#endif /* __DELAYFILTER_HPP__ */