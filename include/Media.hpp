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

#ifndef __MEDIACODEC_HPP__
#define __MEDIACODEC_HPP__

#include <vector>
#include <string>
#include <memory>
#include "AudioFormat.hpp"
#include "InterPipeBridge.hpp"
#include "ThreadBase.hpp"
#include "ResourceManager.hpp"

class MediaParam
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

  MediaParam(std::string key, std::string value):key(key){ set(value); };
  MediaParam(std::string key, int value):key(key){ setInt(value); };
  MediaParam(std::string key, bool value):key(key){ setBool(value); };
};

class IMediaCodec;

class IMediaCodec : public ThreadBase, public IResourceConsumer
{
protected:
  std::vector<std::shared_ptr<InterPipeBridge>> mpInterPipeBridges;
  virtual void unlockToStop(void);

public:
  IMediaCodec();
  virtual ~IMediaCodec();

  virtual void configure(MediaParam param) = 0;
  virtual void configure(std::vector<MediaParam> params);

  virtual void seek(int64_t position);
  virtual int64_t getPosition(void);
  virtual int getEsChunkSize(void) = 0;
  virtual void doProcess(IAudioBuffer& inBuf, IAudioBuffer& outBuf) = 0;
  virtual AudioFormat getFormat(void) = 0;

  static std::shared_ptr<IMediaCodec> createByFormat(AudioFormat format, bool bDecoder = true);
};

#endif /* __MEDIA_HPP__ */