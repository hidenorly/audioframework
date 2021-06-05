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

#ifndef __POWER_MANAGER_HPP__
#define __POWER_MANAGER_HPP__

#include <vector>
#include <functional>
#include <string>

class IPowerManager
{
public:
  enum POWERSTATE
  {
    ACTIVE,  // S0, D0 : ACTIVE
    IDLE,    // S1, D1 : INACTIVE - CLOCK GATE LEVEL
    SUSPEND, // S3, D3 : INACTIVE - POWER GATE LEVEL
    OFF,     // S5, D5 : INACTIVE - COMPLETELY POWER OFF
  };

  POWERSTATE getPowerState(void);
  std::string getPowerStateString(POWERSTATE powerState);
  typedef std::function<void(IPowerManager::POWERSTATE powerState)> CALLBACK;

protected:
  std::vector<CALLBACK> mCallbacks;
  POWERSTATE mPowerState;
  void notifyStateChanged(POWERSTATE powerState);

public:
  virtual ~IPowerManager();
  virtual int registerCallback(CALLBACK callback);
  virtual void unregisterCallback(int callbackId);
};

class IPowerManagerAdmin
{
public:
  virtual void setPowerState(IPowerManager::POWERSTATE power) = 0;
};

class PowerManager : public IPowerManager, public IPowerManagerAdmin
{
protected:
  static inline IPowerManager* mPowerManager = nullptr;

  PowerManager();
  virtual ~PowerManager();

public:
  static IPowerManager* getManager(void);
  virtual void setPowerState(IPowerManager::POWERSTATE power);
};

#endif /* __POWER_MANAGER_HPP__ */