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

#include "Testability.hpp"
#include "PowerManager.hpp"
#include "PowerManagerPrimitive.hpp"

IPowerManager::~IPowerManager()
{
  mCallbacks.clear();
}

IPowerManager::POWERSTATE IPowerManager::getPowerState(void)
{
  return mPowerState;

}

std::string IPowerManager::getPowerStateString(IPowerManager::POWERSTATE powerState)
{
  std::string result = "POWER_STATE_UNKNOWN";

  switch( powerState ){
    case IPowerManager::POWERSTATE::ACTIVE:
      result = "POWER_STATE_ACTIVE";
      break;
    case IPowerManager::POWERSTATE::IDLE:
      result = "POWER_STATE_IDLE";
      break;
    case IPowerManager::POWERSTATE::SUSPEND:
      result = "POWER_STATE_SUSPEND";
      break;
    case IPowerManager::POWERSTATE::OFF:
      result = "POWER_STATE_OFF";
      break;
  }

  return result;
}

int IPowerManager::registerCallback(CALLBACK callback)
{
  mCallbacks.push_back( callback );
  return mCallbacks.size() - 1;
}

void IPowerManager::unregisterCallback(int callbackId)
{
  if( callbackId < mCallbacks.size() ){
    mCallbacks.erase( mCallbacks.begin()+callbackId );
  }
}

void IPowerManager::registerListener(std::weak_ptr<IPowerManager::PowerChangeListener> listener)
{
  mListeners.push_back( listener );
}

bool IPowerManager::unregisterListener(std::weak_ptr<IPowerManager::PowerChangeListener> theListener)
{
  bool result = false;

  const auto pos = std::find_if(mListeners.begin(), mListeners.end(), [&theListener](const std::weak_ptr<IPowerManager::PowerChangeListener>& aListener) {
    return aListener.lock() == theListener.lock();
  });

  if( pos != mListeners.end() ){
    mListeners.erase(pos);
    result = true;
  }

  return result;
}

void IPowerManager::notifyStateChanged(POWERSTATE powerState)
{
  for(auto& aCallback : mCallbacks ){
    aCallback( powerState );
  }

 for(auto& aListener : mListeners ){
    std::shared_ptr<IPowerManager::PowerChangeListener> theListener = aListener.lock();
    if( theListener ){
      theListener->onPowerStateChanged( powerState );
    }
  }
}

void PowerManager::onInstantiate(void)
{
  mPowerManagerPrimitive = std::make_shared<PowerManagerPrimitive>();
  mPowerManagerPrimitive->initialize();
}

void PowerManager::onFinalize(void)
{
  if( mPowerManagerPrimitive ){
    mPowerManagerPrimitive->terminate();
    mPowerManagerPrimitive.reset();
  }
}

void PowerManager::setPowerState(IPowerManager::POWERSTATE powerState)
{
  mPowerState = powerState;
  notifyStateChanged( powerState );
}

#if __AFW_TEST__
std::weak_ptr<ITestable> PowerManager::getTestShim(void)
{
  return mPowerManagerPrimitive;
}
#endif /* __AFW_TEST__ */

