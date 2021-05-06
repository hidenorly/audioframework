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

void ParameterManager::executeNotify(std::string key, std::string value, std::vector<LISTENER> listeners)
{
  for( auto& aListener : listeners ){
    aListener.callback( key, value );
  }
}

void ParameterManager::setParameter(std::string key, std::string value)
{
  bool bChanged = true;

  if( mParams.contains( key ) ){
    // check ro.* (=read only)
    if( 0 == key.find( "ro." ) ) return;

    bChanged = ( mParams[ key ] != value );
  }

  mParams.insert_or_assign( key, value );

  if( bChanged ) {
    for( auto& [aKey, listeners] : mWildCardListeners ){
      if( key.starts_with( aKey ) ){
        executeNotify( key, value, listeners );
      }
    }
    if( mListeners.contains( key ) ){
      auto listeners = mListeners[ key ];
      executeNotify( key, value, listeners );
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
  for( auto& aParam : params ){
    setParameter( aParam.key, aParam.value );
  }
}

std::string ParameterManager::getParameter(std::string key, std::string defaultValue)
{
  return mParams.contains( key ) ? mParams[key] : defaultValue;
}

int ParameterManager::getParameterInt(std::string key, int defaultValue)
{
  return mParams.contains( key ) ? std::stoi(mParams[key]) : defaultValue;
}

float ParameterManager::getParameterFloat(std::string key, float defaultValue)
{
  return mParams.contains( key ) ? std::stof(mParams[key]) : defaultValue;
}

bool ParameterManager::getParameterBool(std::string key, bool defaultValue)
{
  return mParams.contains( key ) ? (mParams[key] == "true" ? true : false) : defaultValue;
}

std::vector<ParameterManager::Param> ParameterManager::getParameters(std::vector<std::string> keys)
{
  std::vector<Param> result;

  if( !keys.empty() ){
    for( auto& aKey : keys) {
      std::string value = getParameter( aKey );
      result.push_back( Param(aKey, value) );
    }
  } else {
    // no specifying means all of parameters
    for( auto& [aKey, value] : mParams ){
      result.push_back( Param(aKey, value) );
    }
  }

  return result;
}

int ParameterManager::registerCallback(std::string key, CALLBACK callback)
{
  int listenerId = mListnerId++;
  if( key.ends_with("*") ){
    // wild card case
    std::string _key = key.substr( 0, key.length() -1 );
    if( !mWildCardListeners.contains( _key ) ){
      std::vector<LISTENER> listeners;
      mWildCardListeners.insert_or_assign( _key, listeners );
    }
    std::vector<LISTENER> listeners = mWildCardListeners[ _key ];
    listeners.push_back( LISTENER(listenerId, callback) );

    mWildCardListeners.insert_or_assign( _key, listeners );
    mListenerIdReverse.insert_or_assign( listenerId, key );
  } else {
    // complete match case
    if( !mListeners.contains( key ) ){
      std::vector<LISTENER> listeners;
      mListeners.insert_or_assign( key, listeners );
    }
    std::vector<LISTENER> listeners = mListeners[ key ];
    listeners.push_back( LISTENER(listenerId, callback) );

    mListeners.insert_or_assign( key, listeners );
    mListenerIdReverse.insert_or_assign( listenerId, key );
  }

  return listenerId;
}

std::string ParameterManager::getKeyFromListernerId(int listenerId)
{
  std::string result;

  decltype( mListenerIdReverse )::iterator it = mListenerIdReverse.find( listenerId );
  if( it != mListenerIdReverse.end() ){
    result = it->second;
  }
  return result;
}

void ParameterManager::removeListenerWithListenerId(std::vector<LISTENER>& listeners, int listenerId)
{
  for(auto it = listeners.begin(); it!=listeners.end(); it++){
    if( it->listenerId == listenerId ){
      listeners.erase( it );
      break;
    }
  }
}

void ParameterManager::unregisterCallback(int callbackId)
{
  int listenerId = callbackId;
  std::string key = getKeyFromListernerId( listenerId );
  if( !key.empty() ){
    if( key.ends_with("*") ){
      key = key.substr( 0, key.length()-1 );
      std::vector<LISTENER> listeners = mWildCardListeners[ key ];
      removeListenerWithListenerId( listeners, listenerId );
      if( !listeners.empty() ){
        mWildCardListeners.insert_or_assign( key, listeners );
      } else {
        mWildCardListeners.erase( key );
      }
    } else {
      std::vector<LISTENER> listeners = mListeners[ key ];
      removeListenerWithListenerId( listeners, listenerId );
      if( !listeners.empty() ){
        mListeners.insert_or_assign( key, listeners );
      } else {
        mListeners.erase( key );
      }
    }
  }
  mListenerIdReverse.erase( listenerId );
}