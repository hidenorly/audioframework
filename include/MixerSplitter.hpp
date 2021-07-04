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

#ifndef __MIXERSPLITTER_HPP__
#define __MIXERSPLITTER_HPP__

#include "Sink.hpp"
#include "Pipe.hpp"
#include "InterPipeBridge.hpp"
#include "AudioFormat.hpp"
#include "ThreadBase.hpp"
#include "PipeMixer.hpp"
#include <vector>
#include <mutex>
#include <atomic>
#include <thread>
#include <map>
#include <memory>

class MixerSplitter : public ThreadBase
{
public:
  class MapCondition
  {
  public:
    MapCondition(){};
    virtual ~MapCondition(){};
    virtual bool canHandle(AudioFormat srcFormat)=0;
  };
  class MapAnyCondition : public MapCondition
  {
  public:
    MapAnyCondition():MapCondition(){};
    virtual ~MapAnyCondition(){};
    virtual bool canHandle(AudioFormat srcFormat){return true;};
  };
  class MapAnyCompressedCondition : public MapCondition
  {
  public:
    MapAnyCompressedCondition():MapCondition(){};
    virtual ~MapAnyCompressedCondition(){};
    virtual bool canHandle(AudioFormat srcFormat){return srcFormat.isEncodingCompressed();};
  };
  class MapAnyPcmCondition : public MapCondition
  {
  public:
    MapAnyPcmCondition():MapCondition(){};
    virtual ~MapAnyPcmCondition(){};
    virtual bool canHandle(AudioFormat srcFormat){return srcFormat.isEncodingPcm();};
  };

protected:
  class SourceSinkMapper
  {
  public:
    std::shared_ptr<ISink> source;
    std::shared_ptr<ISink> sink;
  public:
    SourceSinkMapper(){};
    SourceSinkMapper(std::shared_ptr<SourceSinkMapper> mapper):source(mapper->source),sink(mapper->sink){};
    SourceSinkMapper(SourceSinkMapper& mapper):source(mapper.source),sink(mapper.sink){};
    SourceSinkMapper(std::shared_ptr<ISink> srcSink, std::shared_ptr<ISink> dstSink):source(srcSink),sink(dstSink){};
    virtual ~SourceSinkMapper(){};
  };
  class SourceSinkConditionMapper : public SourceSinkMapper
  {
  public:
    std::shared_ptr<MapCondition> condition;
  public:
    SourceSinkConditionMapper():SourceSinkMapper(){};
    SourceSinkConditionMapper(std::shared_ptr<SourceSinkConditionMapper> mapper){
      SourceSinkConditionMapper( mapper->source, mapper->sink, mapper->condition );
    };
    SourceSinkConditionMapper(SourceSinkConditionMapper& mapper){
      SourceSinkConditionMapper( mapper.source, mapper.sink, mapper.condition );
    };
    SourceSinkConditionMapper(std::shared_ptr<ISink> srcSink, std::shared_ptr<ISink> dstSink, std::shared_ptr<MapCondition> argCondition):SourceSinkMapper(srcSink, dstSink), condition(argCondition){};
    virtual ~SourceSinkConditionMapper(){};
  };

protected:
  std::mutex mMutexSourceSink;
  std::vector<std::shared_ptr<ISink>> mpSinks;
  std::vector<std::shared_ptr<ISink>> mpSources;
  std::map<std::shared_ptr<ISink>, std::shared_ptr<AudioFormat>> mpSourceAudioFormats;
  std::map<std::shared_ptr<ISink>, std::weak_ptr<IPipe>> mpSourcePipes;
  std::vector<std::shared_ptr<SourceSinkConditionMapper>> mSourceSinkMapper;
  std::map<std::shared_ptr<ISink>, std::shared_ptr<PipeMixer>> mpMixers;
  std::atomic<bool> mbOnChanged;

protected:
  virtual void process(void);
  virtual void unlockToStop(void);
  bool isSinkAvailableLocked(std::shared_ptr<ISink> pSink);
  bool isSourceAvailableLocked(std::shared_ptr<ISink> pSink);
  std::shared_ptr<SourceSinkConditionMapper> getSourceSinkMapperLocked(std::shared_ptr<ISink> pSource, std::shared_ptr<ISink> pSink);
  std::shared_ptr<SourceSinkMapper> getSourceSinkMapperLocked(std::shared_ptr<ISink> pSource);
  bool removeMapperLocked(std::shared_ptr<ISink> srcSink);
  bool isPipeRunningOrNotRegistered(std::shared_ptr<ISink> srcSink);
  bool isSituationChanged(void);

public:
  MixerSplitter();
  virtual ~MixerSplitter();

  virtual std::vector<std::shared_ptr<ISink>> getAllOfSinks(void);
  virtual void addSink(std::shared_ptr<ISink> pSink);
  virtual bool removeSink(std::shared_ptr<ISink> pSink);

  virtual std::vector<std::shared_ptr<ISink>> getAllOfSinkAdaptors(void);
  virtual std::shared_ptr<ISink> allocateSinkAdaptor(AudioFormat format = AudioFormat(), std::shared_ptr<IPipe> pPipe = nullptr);
  virtual bool releaseSinkAdaptor(std::shared_ptr<ISink> pSink);

  virtual bool conditionalMap(std::shared_ptr<ISink> srcSink, std::shared_ptr<ISink> dstSink, std::shared_ptr<MapCondition> conditions);
  virtual bool map(std::shared_ptr<ISink> srcSink, std::shared_ptr<ISink> dstSink);
  virtual bool unmap(std::shared_ptr<ISink> srcSink);

  virtual void dump(void);
};

#endif /* __MIXSPLITTER_HPP__ */