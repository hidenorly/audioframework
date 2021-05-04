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

#include "ParameterManager.hpp"
#include <map>
#include <vector>
#include <string>


ParameterManager ParameterManager::mParamManager;

ParameterManager* ParameterManager::getManager(void)
{
  return &mParamManager;
}


ParameterManager::ParameterManager():mListnerId(0)
{

}

ParameterManager::~ParameterManager()
{

}

void ParameterManager::setParameter(std::string key, std::string value)
{
  bool bChanged = false;

  if( contains(key) ){
    // check ro.* (=read only)
    if( 0 == key.find("ro.") ) return;

    bChanged =( mParams[key] != value );
  }

  mParams.insert_or_assign( key, value );

  if( bChanged && callbackKeyContains(key) ){
    auto listeners = mListeners[key];
    for(auto& aListener : listeners){
      aListener.callback(key, value);
    }
  }
}

void ParameterManager::setParameterInt(std::string key, int value)
{
  setParameter( key, std::to_string(value) );
}

void ParameterManager::setParameterFloat(std::string key, float value)
{
  setParameter( key, std::to_string(value) );
}

void ParameterManager::setParameterBool(std::string key, bool value)
{
  setParameter( key, value ? "true" : "false" );
}

void ParameterManager::setParameters(std::vector<ParameterManager::Param>& params)
{
  for(auto& aParam : params){
    setParameter( aParam.key, aParam.value );
  }
}

bool ParameterManager::contains(std::string key)
{
  decltype(mParams)::iterator it = mParams.find(key);
  return ( it == mParams.end() ) ? false : true;
}


std::string ParameterManager::getParameter(std::string key, std::string defaultValue)
{
  return contains(key) ? mParams[key] : defaultValue;
}

int ParameterManager::getParameterInt(std::string key, int defaultValue)
{
  return contains(key) ? std::stoi(mParams[key]) : defaultValue;
}

float ParameterManager::getParameterFloat(std::string key, float defaultValue)
{
  return contains(key) ? std::stof(mParams[key]) : defaultValue;
}

bool ParameterManager::getParameterBool(std::string key, bool defaultValue)
{
  return contains(key) ? (mParams[key] == "true" ? true : defaultValue) : defaultValue;
}

std::vector<ParameterManager::Param> ParameterManager::getParameters(std::vector<std::string> keys)
{
  std::vector<Param> result;

  if( !keys.empty() ){
    for(auto& aKey : keys){
      std::string value = getParameter( aKey );
      result.push_back( Param(aKey, value) );
    }
  } else {
    for(auto& [aKey, value] : mParams){
      result.push_back( Param(aKey, value) );
    }
  }

  return result;
}

bool ParameterManager::callbackKeyContains(std::string key)
{
  decltype(mListeners)::iterator it = mListeners.find(key);
  return ( it == mListeners.end() ) ? false : true;
}

void ParameterManager::ensureCallbacks(std::string key)
{
  if( !callbackKeyContains(key) ){
    std::vector<LISTENER> listeners;
    mListeners.insert_or_assign( key, listeners );
  }
}

int ParameterManager::registerCallback(std::string key, CALLBACK callback)
{
  ensureCallbacks( key );

  std::vector<LISTENER> listeners = mListeners[ key ];
  int listenerId = mListnerId++;
  listeners.push_back( LISTENER(listenerId, callback) );

  mListeners.insert_or_assign( key, listeners );
  mListenerIdReverse.insert_or_assign( listenerId, key );

  return listenerId;
}

std::string ParameterManager::getKeyFromListernerId(int listenerId)
{
  std::string result;

  decltype(mListenerIdReverse)::iterator it = mListenerIdReverse.find(listenerId);
  if( it != mListenerIdReverse.end() ){
    result = it->second;
  }
  return result;
}

void ParameterManager::unregisterCallback(int callbackId)
{
  int listenerId = callbackId;
  std::string key = getKeyFromListernerId(listenerId);
  if( !key.empty() ){
    std::vector<LISTENER> listeners = mListeners[ key ];
    for(auto it=listeners.begin(); it!=listeners.end(); it++){
      if( it->listenerId == listenerId ){
        listeners.erase( it );
        break;
      }
    }
    if( !listeners.empty() ){
      mListeners.insert_or_assign( key, listeners );
    } else {
      auto it = mListeners.find( key );
      mListeners.erase( it );
    }
  }
  auto it = mListenerIdReverse.find(listenerId);
  if( it!=mListenerIdReverse.end() ) {
    // found
    mListenerIdReverse.erase(it);
  }
}