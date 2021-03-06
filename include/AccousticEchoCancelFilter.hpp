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

#ifndef __AEC_FILTER_HPP__
#define __AEC_FILTER_HPP__

#include "Filter.hpp"
#include "Source.hpp"

class AccousticEchoCancelFilter : public Filter
{
public:
  AccousticEchoCancelFilter();
  ~AccousticEchoCancelFilter();
  virtual void process(AudioBuffer& inBuf, AudioBuffer& refBuf);
};

#endif /* __AEC_FILTER_HPP__ */