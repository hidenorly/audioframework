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

class DecoderParam
{
public:
  std::string key;
  std::string value;

  void set(std::string arg){ value = arg; };
  void setInt(int arg){ value = std::to_string( arg ); };
  void setBool(bool arg){ value = arg ? "true" : "false"; };

  std::string get(void){ return value; };
  int getInt(void){ return std::stoi( value ); };
  bool getBool(void){ return (value == "true") ? true : false; };

  DecoderParam(std::string key, std::string value):key(key){ set(value); };
  DecoderParam(std::string key, int value):key(key){ setInt(value); };
  DecoderParam(std::string key, bool value):key(key){ setBool(value); };
};

class IDecoder : public ThreadBase
{
protected:
  ISource* mpSource;
  std::vector<InterPipeBridge*> mpInterPipeBridges;

public:
  IDecoder();
  virtual ~IDecoder();

  virtual void configure(DecoderParam param);
  virtual void configure(std::vector<DecoderParam> params);
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

  virtual void configure(DecoderParam param);

protected:
  virtual void process(void);
};

#endif /* __DECODER_HPP__ */