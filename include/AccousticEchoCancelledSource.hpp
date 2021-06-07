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

#include "Source.hpp"
#include "Sink.hpp"
#include "DelayFilter.hpp"

class AccousticEchoCancelledSource : public ISource
{
protected:
  ISource* mpSource;
  ISink* mpReferenceSound;
  DelayFilter* mpDelay;

protected:
  virtual void readPrimitive(IAudioBuffer& buf);

public:
  AccousticEchoCancelledSource(ISource* pSource, ISink* pReferenceSound = nullptr);
  virtual ~AccousticEchoCancelledSource();
  virtual std::string toString(void){return "AccousticEchoCanceledSource";};
};


#endif /* __AEC_FILTER_HPP__ */