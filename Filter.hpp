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
#include <iostream>
#include <vector>

class Filter : public AudioBase
{
protected:
  const float DEFAULT_WINDOW_SIZE_USEC = 5000; // 5msec

public:
  Filter(){};
  virtual ~Filter(){};
  virtual void process(AudioBuffer& inBuf, AudioBuffer& outBuf){ outBuf = inBuf; };
  virtual int getRequiredWindowSizeUsec(void){ return DEFAULT_WINDOW_SIZE_USEC; };
  virtual std::vector<AudioFormat> getSupportedAudioFormats(void){
    std::vector<AudioFormat> audioFormats;
    audioFormats.push_back( AudioFormat() );
    return audioFormats;
  };
};

#endif /* __FILTER_HPP__ */