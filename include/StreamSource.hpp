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

#ifndef __STREAMSOURCE_HPP__
#define __STREAMSOURCE_HPP__

#include "Source.hpp"
#include "Stream.hpp"
#include "AudioFormat.hpp"
#include <memory>

class StreamSource : public ISource
{
protected:
  AudioFormat mFormat;
  std::shared_ptr<IStream> mpStream;

protected:
  virtual void readPrimitive(IAudioBuffer& buf);
  virtual void setAudioFormatPrimitive(AudioFormat audioFormat);

public:
  StreamSource(AudioFormat format = AudioFormat(), std::shared_ptr<IStream> pStream = nullptr);
  virtual ~StreamSource();
  virtual void close(void);
  virtual std::string toString(void){return "StreamSource";};

  virtual void parse(ByteBuffer& inStreamBuf, IAudioBuffer& dstAudioBuf);
  virtual AudioFormat getAudioFormat(void);
};

#endif /* __STREAMSOURCE_HPP__ */