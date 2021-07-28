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

#ifndef __INTERPIPEBRIDGE_HPP__
#define __INTERPIPEBRIDGE_HPP__

#include "Buffer.hpp"
#include "FifoBuffer.hpp"
#include "AudioFormat.hpp"
#include "Sink.hpp"
#include "Source.hpp"
#include <string>

class InterPipeBridge : public ISource, public ISink, public IUnlockable
{
protected:
  FifoBuffer mFifoBuffer;
  int mRequiredResource;

protected:
  virtual void readPrimitive(IAudioBuffer& buf);
  virtual void writePrimitive(IAudioBuffer& buf);
  virtual void setAudioFormatPrimitive(AudioFormat audioFormat);

public:
  InterPipeBridge(AudioFormat format = AudioFormat());
  virtual ~InterPipeBridge(){ mFifoBuffer.unlock(); };
  virtual bool isAvailableFormat(AudioFormat format){ return true; };

  virtual void dump(void){};
  virtual std::string toString(void){return "InterPipeBridge";};

  virtual AudioFormat getAudioFormat(void);

  virtual void unlock(void){ mFifoBuffer.unlock(); };
  virtual int stateResourceConsumption(void);
  virtual void setRequiredResourceConsumption(int nRequiredResource);
};

#endif /* __INTERPIPEBRIDGE_HPP__ */