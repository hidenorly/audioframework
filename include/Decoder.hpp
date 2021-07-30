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

#ifndef __DECODER_HPP__
#define __DECODER_HPP__

#include "Media.hpp"
#include "Buffer.hpp"
#include "AudioFormat.hpp"
#include "Source.hpp"
#include <memory>

class IDecoder : public IMediaCodec
{
protected:
  std::shared_ptr<ISource> mpSource;
  virtual void process(void);

public:
  IDecoder();
  virtual ~IDecoder();

  virtual std::shared_ptr<ISource> attachSource(std::shared_ptr<ISource> pSource);
  virtual std::shared_ptr<ISource> detachSource(void);
  virtual std::shared_ptr<ISource> allocateSourceAdaptor(void);
  virtual void releaseSourceAdaptor(std::shared_ptr<ISource> pSource);

  virtual bool isDecoder(void){ return true; };

  static std::shared_ptr<IMediaCodec> createByFormat(AudioFormat format, bool bDecoder = true){
    return IMediaCodec::createByFormat(format, bDecoder);
  }
};

class NullDecoder : public IDecoder
{
protected:
  AudioFormat mFormat;

public:
  NullDecoder( AudioFormat format = AudioFormat(AudioFormat::ENCODING::COMPRESSED) );
  ~NullDecoder();

  virtual void configure(MediaParam param);
  virtual int stateResourceConsumption(void);

  virtual int getEsChunkSize(void);
  virtual void doProcess(IAudioBuffer& inBuf, IAudioBuffer& outBuf);
  virtual AudioFormat getFormat(void);
};

#endif /* __DECODER_HPP__ */