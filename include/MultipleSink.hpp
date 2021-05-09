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

#ifndef __MULTIPLESINK_HPP__
#define __MULTIPLESINK_HPP__

#include "Sink.hpp"
#include "AudioFormat.hpp"
#include "Buffer.hpp"
#include <vector>
#include <map>


class MultipleSink : public ISink
{
protected:
  std::vector<ISink*> mpSinks;
  std::map<ISink*, AudioFormat::ChannelMapper> mChannelMaps;
  AudioFormat mFormat;

public:
  MultipleSink(AudioFormat audioFormat = AudioFormat());
  virtual ~MultipleSink();
  virtual void addSink(ISink* pSink, AudioFormat::ChannelMapper& map);
  virtual void clearSinks(void);

  virtual void write(AudioBuffer& buf);
  virtual std::string toString(void){ return "MultipleSink"; };

  virtual bool setAudioFormat(AudioFormat audioFormat);
  virtual AudioFormat getAudioFormat(void);

  virtual void dump(void);
};

#endif /* __MULTIPLESINK_HPP__ */