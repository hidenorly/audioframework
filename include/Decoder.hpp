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
#include "Source.hpp"
#include "InterPipeBridge.hpp"
#include "ThreadBase.hpp"
#include <string>
#include <vector>
#include <thread>
#include "Media.hpp"
#include "ResourceManager.hpp"

class IDecoder : public ThreadBase, public IResourceConsumer
{
protected:
  ISource* mpSource;
  std::vector<InterPipeBridge*> mpInterPipeBridges;
  virtual void unlockToStop(void);

public:
  IDecoder();
  virtual ~IDecoder();

  virtual void configure(MediaParam param);
  virtual void configure(std::vector<MediaParam> params);
  virtual ISource* attachSource(ISource* pSource);
  virtual ISource* detachSource(void);
  virtual ISource* allocateSourceAdaptor(void);
  virtual void releaseSourceAdaptor(ISource* pSource);
  virtual void seek(int64_t position);
  virtual int64_t getPosition(void);
};

class NullDecoder : public IDecoder
{
public:
  NullDecoder();
  ~NullDecoder();

  virtual void configure(MediaParam param);
  virtual int stateResourceConsumption(void);

protected:
  virtual void process(void);
};

#endif /* __DECODER_HPP__ */