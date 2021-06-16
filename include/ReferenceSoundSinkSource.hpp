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

#ifndef __REFERENCESOUND_SINK_SOURCE_HPP__
#define __REFERENCESOUND_SINK_SOURCE_HPP__

#include "InterPipeBridge.hpp"
#include "Sink.hpp"
#include <mutex>
#include <memory>

class ReferenceSoundSinkSource : public InterPipeBridge
{
protected:
  std::shared_ptr<ISink> mpSink;
  std::mutex mMutexSink;

protected:
  virtual void readPrimitive(IAudioBuffer& buf);
  virtual void writePrimitive(IAudioBuffer& buf);

public:
  ReferenceSoundSinkSource( std::shared_ptr<ISink> pSink );
  virtual ~ReferenceSoundSinkSource();

  std::shared_ptr<ISink> attachSink(std::shared_ptr<ISink> pSink);
  std::shared_ptr<ISink> detachSink(void);
  void clearBuffer(void);

  virtual bool setAudioFormat(AudioFormat audioFormat);
  virtual AudioFormat getAudioFormat(void);

  virtual int stateResourceConsumption(void);
};

#endif /* __REFERENCESOUND_SINK_SOURCE_HPP__ */