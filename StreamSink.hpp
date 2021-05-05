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

#ifndef __STREAMSINK_HPP__
#define __STREAMSINK_HPP__

#include "Buffer.hpp"
#include "AudioFormat.hpp"
#include "Sink.hpp"
#include "Stream.hpp"
#include <string>

class StreamSink : public ISink, public AudioBase
{
protected:
  IStream*  mpStream;
  AudioFormat mFormat;

public:
  StreamSink(AudioFormat format, IStream* pStream);
  virtual ~StreamSink();

  virtual void serialize(AudioBuffer& srcAudioBuf, ByteBuffer& outStreamBuf);
  virtual void write(AudioBuffer& buf);
  virtual void close(void);

  virtual void dump(void){};
  virtual std::string toString(void){return "StreamSink";};

  virtual bool setAudioFormat(AudioFormat audioFormat);
  virtual AudioFormat getAudioFormat(void);
};

#endif /* __STREAMSINK_HPP__ */