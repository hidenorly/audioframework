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

#ifndef __FIFOBUFFER_HPP__
#define __FIFOBUFFER_HPP__

#include "Buffer.hpp"
#include "AudioFormat.hpp"
#include <mutex>
#include <condition_variable>

class FifoBuffer
{
protected:
  AudioFormat mFormat;
  ByteBuffer mBuf;
  std::mutex mMutex;
  std::condition_variable mEvent;
  std::mutex mEventMutex;

public:
  FifoBuffer(AudioFormat& format);
  FifoBuffer(){ FifoBuffer(AudioFormat()); };
  virtual ~FifoBuffer();

  bool read(AudioBuffer& audioBuf);
  bool write(AudioBuffer& audioBuf);

  int getBufferedSamples(void);
  AudioFormat getAudioFormat(void){ return mFormat; };
};

#endif /* __FIFOBUFFER_HPP__ */