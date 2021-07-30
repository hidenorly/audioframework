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

#ifndef __ENCODER_HPP__
#define __ENCODER_HPP__

#include "Media.hpp"
#include "Buffer.hpp"
#include "AudioFormat.hpp"
#include "Sink.hpp"
#include <memory>

class IEncoder : public IMediaCodec
{
protected:
  std::shared_ptr<ISink> mpSink;
  virtual void process(void);

public:
  IEncoder();
  virtual ~IEncoder();

  virtual std::shared_ptr<ISink> allocateSinkAdaptor(void); // as encoder's source
  virtual void releaseSinkAdaptor(std::shared_ptr<ISink> pSink);

  virtual std::shared_ptr<ISink> attachSink(std::shared_ptr<ISink> pSink);
  virtual std::shared_ptr<ISink> detachSink(void);

  virtual int getRequiredSamples(void) = 0;
  virtual bool isDecoder(void){ return false; };

  static std::shared_ptr<IMediaCodec> createByFormat(AudioFormat format, bool bDecoder = false){
    return IMediaCodec::createByFormat(format, bDecoder);
  }
};

class NullEncoder : public IEncoder
{
protected:
  AudioFormat mFormat;

public:
  NullEncoder( AudioFormat format = AudioFormat(AudioFormat::ENCODING::COMPRESSED) );
  ~NullEncoder();

  virtual void configure(MediaParam param);
  virtual int stateResourceConsumption(void);

  virtual int getEsChunkSize(void);
  virtual int getRequiredSamples(void);
  virtual void doProcess(IAudioBuffer& inBuf, IAudioBuffer& outBuf);
  virtual AudioFormat getFormat(void);
};

#endif /* __ENCODER_HPP__ */