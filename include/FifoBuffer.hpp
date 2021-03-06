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

#include "PipeAndFilterCommon.hpp"
#include "Buffer.hpp"
#include "AudioFormat.hpp"
#include <mutex>
#include <atomic>
#include <condition_variable>

class FifoBufferBase : public IUnlockable, public AudioBase
{
protected:
  AudioFormat mFormat;
  ByteBuffer mBuf;
  int mFifoSizeLimit;
  std::mutex mBufMutex;
  std::condition_variable mReadBlockEvent;
  std::mutex mReadBlockEventMutex;
  std::atomic<bool> mReadBlocked;
  std::atomic<bool> mUnlockReadBlock;

protected:
  FifoBufferBase(AudioFormat format = AudioFormat());
  virtual ~FifoBufferBase();
  void setAudioFormatPrimitive(AudioFormat format);

public:
  int getBufferedSamples(void);
  int getBufferedBytes(void);
  void setFifoSizeLimit(int nSampleLimit);
  AudioFormat getAudioFormat(void){ return mFormat; };
  void clearBuffer(void);
  virtual bool isAvailableFormat(AudioFormat format){ return true; };
};

class FifoBuffer : public FifoBufferBase
{
protected:
  std::condition_variable mWriteBlockEvent;
  std::mutex mWriteBlockEventMutex;
  std::atomic<bool> mWriteBlocked;
  std::atomic<bool> mUnlockWriteBlock;

public:
  FifoBuffer(AudioFormat format = AudioFormat());
  virtual ~FifoBuffer();

  bool read(IAudioBuffer& audioBuf);
  bool write(IAudioBuffer& audioBuf);
  virtual void unlock(void);
};

#endif /* __FIFOBUFFER_HPP__ */