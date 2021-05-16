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

#include "Stream.hpp"

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

protected:
  static ParameterManager mParamManager;
  ParameterManager();
  virtual ~ParameterManager();

public:
  static ParameterManager* getManager(void);
  void setParameter(std::string key, std::string value);
  void setParameterInt(std::string key, int value);
  void setParameterFloat(std::string key, float value);
  void setParameterBool(std::string key, bool value);
  void setParameters(std::vector<ParameterManager::Param>& params);

  std::string getParameter(std::string key, std::string defaultValue = "");
  int getParameterInt(std::string key, int defaultValue = 0);
  float getParameterFloat(std::string key, float defaultValue = 0.0f);
  bool getParameterBool(std::string key, bool defaultValue = false);
  // get all parameters if no keys specified
  std::vector<ParameterManager::Param> getParameters(std::vector<std::string> keys = std::vector<std::string>{});

  typedef std::function<void(std::string key, std::string value)> CALLBACK;
  int registerCallback(std::string key, CALLBACK callback);
  void unregisterCallback(int callbackId);

  bool storeToStream(IStream* pStream);
  bool restoreFromStream(IStream* pStream);

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

  void removeListenerWithListenerId(std::vector<LISTENER>& listeners, int listenerId);
  std::string getKeyFromListernerId(int listenerId);
  void executeNotify(std::string key, std::string value, std::vector<LISTENER> listeners);
};

#endif /* __PARAMETER_MANAGER_HPP__ */