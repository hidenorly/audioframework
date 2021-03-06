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

#ifndef __FIFOBUFFERREADREFERENCE_HPP__
#define __FIFOBUFFERREADREFERENCE_HPP__

#include "FifoBuffer.hpp"
#include "Buffer.hpp"
#include "AudioFormat.hpp"
#include <mutex>
#include <atomic>
#include <condition_variable>

class FifoBufferReadReference : public FifoBufferBase
{
public:
  FifoBufferReadReference(AudioFormat format = AudioFormat());
  virtual ~FifoBufferReadReference();

  bool readReference(IAudioBuffer& audioBuf);
  bool write(IAudioBuffer& audioBuf);
  virtual void unlock(void);
};

#endif /* __FIFOBUFFERREADREFERENCE_HPP__ */
