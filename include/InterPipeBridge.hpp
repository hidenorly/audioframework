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

class InterPipeBridge : public ISink, public ISource, public AudioBase
{
protected:
  FifoBuffer mFifoBuffer;

public:
  InterPipeBridge(AudioFormat format = AudioFormat());
  virtual ~InterPipeBridge(){ mFifoBuffer.unlock(); };

  virtual void readPrimitive(AudioBuffer& buf);
  virtual void writePrimitive(AudioBuffer& buf);

  virtual void dump(void){};
  virtual std::string toString(void){return "InterPipeBridge";};

  virtual bool setAudioFormat(AudioFormat audioFormat);
  virtual AudioFormat getAudioFormat(void);

  void unlock(void){ mFifoBuffer.unlock(); };
};

#endif /* __INTERPIPEBRIDGE_HPP__ */