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

#ifndef __STREAM_MANAGER_HPP__
#define __STREAM_MANAGER_HPP__

#include <vector>
#include <memory>
#include <map>
#include "Pipe.hpp"
#include "Sink.hpp"
#include "Source.hpp"
#include "Strategy.hpp"
#include "Singleton.hpp"


class StreamInfo
{
public:
  int id;
  std::shared_ptr<StrategyContext> context;
  std::shared_ptr<IPipe> pipe;
  StreamInfo(){};
  virtual ~StreamInfo(){};
};

class StreamManager;

class StreamManager : public SingletonBase<StreamManager>
{
protected:
  std::vector<std::shared_ptr<StreamInfo>> mStreamInfos;
  int mId;
  virtual void onInstantiate(void){ mId = 0; };
  virtual void onFinalize(void){ clear(); };

public:
  std::shared_ptr<StreamInfo> get(int id);
  std::shared_ptr<StreamInfo> get(std::shared_ptr<IPipe> pPipe);
  std::shared_ptr<StreamInfo> get(std::shared_ptr<StrategyContext> pContext);

  int add(std::shared_ptr<StrategyContext> pContext, std::shared_ptr<IPipe> pPipe);
  int add(std::shared_ptr<StreamInfo> pStreamInfo );

  bool remove(int id);
  bool remove(std::shared_ptr<IPipe> pPipe);
  bool remove(std::shared_ptr<StrategyContext> pContext);
  bool remove(std::shared_ptr<StreamInfo> pStreamInfo);

  void clear(void);

  int getId(std::shared_ptr<IPipe> pPipe);
  int getId(std::shared_ptr<StrategyContext> pContext);

  std::shared_ptr<IPipe> getPipe(int id);
  std::shared_ptr<IPipe> getPipe(std::shared_ptr<StrategyContext> pContext);

  std::shared_ptr<StrategyContext> getContext(int id);
  std::shared_ptr<StrategyContext> getContext(std::shared_ptr<IPipe> pPipe);
};

#endif /* __STREAM_MANAGER_HPP__ */