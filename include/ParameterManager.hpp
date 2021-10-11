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

#ifndef __PARAMETER_MANAGER_HPP__
#define __PARAMETER_MANAGER_HPP__

#include <map>
#include <vector>
#include <string>
#include <memory>
#include <functional>

#if USE_PARAMETERMANAGER_ADMINISTRATIVE_API
#include "Stream.hpp"
#endif /* USE_PARAMETERMANAGER_ADMINISTRATIVE_API */

class ParameterManager;

class ParameterManager
{
public:
  struct Param
  {
  public:
    std::string key;
    std::string value;
    Param(std::string key, std::string value):key(key),value(value){};
  };

  enum ParamType
  {
    TYPE_INT,
    TYPE_FLOAT,
    TYPE_BOOL,
    TYPE_STRING,
  };

  enum ParamRange
  {
    RANGE_ANY,
    RANGED,
    RANGE_ENUM,
  };

  class ParamRule
  {
  public:
    ParamType type;
    ParamRange range;
    float rangeMin;
    float rangeMax;
    std::vector<std::string> enumVals;
    ParamRule(): type(ParamType::TYPE_STRING), range(ParamRange::RANGE_ANY), rangeMin(0.0f), rangeMax(0.0f){};
    ParamRule(ParamType type): type(type), range(ParamRange::RANGE_ANY), rangeMin(0.0f), rangeMax(0.0f){};
    ParamRule(ParamType type, float rangeMin, float rangeMax): type(type), range(ParamRange::RANGED), rangeMin(rangeMin), rangeMax(rangeMax){};
    ParamRule(ParamType type, std::vector<std::string> enumVals): type(type), range(ParamRange::RANGE_ENUM), enumVals(enumVals){};
  };

protected:
  static inline std::shared_ptr<ParameterManager> mParamManager;
  ParameterManager();
  std::string trimParamString(std::string value);

public:
  virtual ~ParameterManager();
  // for all of ParameterManager users
  static std::weak_ptr<ParameterManager> getManager(void);
  bool setParameter(std::string key, std::string value);
  bool setParameterInt(std::string key, int value);
  bool setParameterFloat(std::string key, float value);
  bool setParameterBool(std::string key, bool value);
  bool setParameters(std::vector<ParameterManager::Param>& params);

  void setParameterRule(std::string key, ParamRule rule);
  ParamRule getParameterRule(std::string key);

  std::string getParameter(std::string key, std::string defaultValue = "");
  int getParameterInt(std::string key, int defaultValue = 0);
  float getParameterFloat(std::string key, float defaultValue = 0.0f);
  bool getParameterBool(std::string key, bool defaultValue = false);
  // get all parameters if no keys specified
  std::vector<ParameterManager::Param> getParameters(std::string wildcardKeys);
  std::vector<ParameterManager::Param> getParameters(std::vector<std::string> keys = std::vector<std::string>{});

  typedef std::function<void(std::string key, std::string value)> CALLBACK;
  int registerCallback(std::string key, CALLBACK callback);
  void unregisterCallback(int callbackId);

#if USE_PARAMETERMANAGER_ADMINISTRATIVE_API
  // administrative API
  bool storeToStream(std::shared_ptr<IStream> pStream);
  bool restoreFromStream(std::shared_ptr<IStream> pStream, bool bOverride = true);
  void resetAllOfParams(void);
#endif /* USE_PARAMETERMANAGER_ADMINISTRATIVE_API */

protected:
  int mListnerId;
  std::map<std::string, std::string> mParams;
  struct LISTENER
  {
  public:
    int listenerId;
    CALLBACK callback;
    LISTENER(int listenerId, CALLBACK callback): listenerId(listenerId), callback(callback){};
  };
  std::map<std::string, std::vector<LISTENER>> mListeners;
  std::map<std::string, std::vector<LISTENER>> mWildCardListeners;
  std::map<int, std::string> mListenerIdReverse;
  std::map<std::string, ParamRule> mParamRules;

  void removeListenerWithListenerId(std::vector<LISTENER>& listeners, int listenerId);
  std::string getKeyFromListernerId(int listenerId);
  void executeNotify(std::string key, std::string value, std::vector<LISTENER> listeners);
  bool filterValueWithRule(std::string key, std::string& value);
};

#endif /* __PARAMETER_MANAGER_HPP__ */
