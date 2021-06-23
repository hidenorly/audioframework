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

#include "Buffer.hpp"
#include "AudioFormat.hpp"
#include "Source.hpp"
#include "InterPipeBridge.hpp"
#include "ThreadBase.hpp"
#include <string>
#include <vector>
#include <thread>
#include <memory>
#include "Media.hpp"
#include "ResourceManager.hpp"

class IDecoder;

class IDecoder : public ThreadBase, public IResourceConsumer
{
protected:
  std::shared_ptr<ISource> mpSource;
  std::vector<std::shared_ptr<InterPipeBridge>> mpInterPipeBridges;
  virtual void unlockToStop(void);
  virtual void process(void);

public:
  IDecoder();
  virtual ~IDecoder();

  virtual void configure(MediaParam param);
  virtual void configure(std::vector<MediaParam> params);
  virtual std::shared_ptr<ISource> attachSource(std::shared_ptr<ISource> pSource);
  virtual std::shared_ptr<ISource> detachSource(void);
  virtual std::shared_ptr<ISource> allocateSourceAdaptor(void);
  virtual void releaseSourceAdaptor(std::shared_ptr<ISource> pSource);
  virtual void seek(int64_t position);
  virtual int64_t getPosition(void);

  virtual int getEsChunkSize(void) = 0;
  virtual void doProcess(IAudioBuffer& inBuf, IAudioBuffer& outBuf) = 0;
  virtual AudioFormat getFormat(void) = 0;

  static std::shared_ptr<IDecoder> createByFormat(AudioFormat format);
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