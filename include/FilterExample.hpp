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

#ifndef __FILTER_EXAMPLES_HPP__
#define __FILTER_EXAMPLES_HPP__

#include "Filter.hpp"

class FilterIncrement : public Filter
{
protected:
  int mWindowSize;

public:
  FilterIncrement(int windowSize = DEFAULT_WINDOW_SIZE_USEC) : mWindowSize(windowSize){};
  virtual ~FilterIncrement(){};
  virtual void process(AudioBuffer& inBuf, AudioBuffer& outBuf);
  virtual int getRequiredWindowSizeUsec(void){ return mWindowSize; };
  virtual std::string toString(void){ return "FilterIncrement"; };
};


#endif /* __FILTER_EXAMPLES_HPP__ */