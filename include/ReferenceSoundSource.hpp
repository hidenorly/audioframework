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

#ifndef __REFERENCESOUND_SOURCE_HPP__
#define __REFERENCESOUND_SOURCE_HPP__

#include "InterPipeBridge.hpp"
#include "Sink.hpp"
#include <mutex>

class ReferenceSoundSource : public InterPipeBridge
{
protected:
  ISink* mpSink;
  std::mutex mMutexSink;

protected:
  virtual void writePrimitive(IAudioBuffer& buf);

public:
  ReferenceSoundSource( ISink* pSink );
  virtual ~ReferenceSoundSource();

  ISink* attachSink(ISink* pSink);
  ISink* detachSink(void);
  void clearBuffer(void);
};

#endif /* __REFERENCESOUND_SOURCE_HPP__ */