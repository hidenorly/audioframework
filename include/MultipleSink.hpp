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
#include "DelayFilter.hpp"
#include <vector>
#include <map>
#include <memory>


class MultipleSink : public ISink
{
protected:
  std::vector<std::shared_ptr<ISink>> mpSinks;
  std::map<std::shared_ptr<ISink>, AudioFormat::ChannelMapper> mChannelMaps;
  AudioFormat mFormat;
  std::map<std::shared_ptr<ISink>, std::shared_ptr<DelayFilter>> mpDelayFilters;
  int mMaxLatency;

  void ensureDelayFilters(bool bForceRecreate = false);
  std::vector<float> getPerSinkChannelVolumes(std::shared_ptr<ISink> pSink, Volume::CHANNEL_VOLUME perChannelVolumes);

public:
  MultipleSink(AudioFormat audioFormat = AudioFormat());
  virtual ~MultipleSink();
  virtual void attachSink(std::shared_ptr<ISink> pSink, AudioFormat::ChannelMapper& map);
  virtual bool detachSink(std::shared_ptr<ISink> pSink);
  virtual void clearSinks();

  virtual void writePrimitive(IAudioBuffer& buf);
  virtual std::string toString(void){ return "MultipleSink"; };

  virtual bool setAudioFormat(AudioFormat audioFormat);
  virtual AudioFormat getAudioFormat(void);

  virtual int getLatencyUSec(void);
  virtual float getVolume(void);
  virtual bool setVolume(float volumePercentage);
  virtual bool setVolume(Volume::CHANNEL_VOLUME perChannelVolumes);

  virtual void dump(void);
  virtual int stateResourceConsumption(void);
};

#endif /* __MULTIPLESINK_HPP__ */