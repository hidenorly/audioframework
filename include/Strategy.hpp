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

#ifndef __STRATEGY_HPP__
#define __STRATEGY_HPP__

#include <vector>
#include <memory>

class StrategyContext
{
public:
  StrategyContext(){};
  virtual ~StrategyContext(){};
};

class IStrategy
{
public:
  virtual ~IStrategy(){};

  virtual bool canHandle(std::shared_ptr<StrategyContext> context) = 0;
  virtual bool execute(std::shared_ptr<StrategyContext> context) = 0;
};

class Strategy
{
protected:
  std::vector<std::shared_ptr<IStrategy>> mStrategies;
public:
  Strategy();
  virtual ~Strategy();

  bool execute(std::shared_ptr<StrategyContext> context);
  void registerStrategy(std::shared_ptr<IStrategy> aStrategy);
  void unregisterStrategy(std::shared_ptr<IStrategy> aStrategy);
};

#endif /* __STRATEGY_HPP__ */