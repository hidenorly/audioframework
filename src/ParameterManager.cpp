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
#include "StringTokenizer.hpp"
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

  key = trimParamString(key);
  value = trimParamString(value);

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

void ParameterManager::setParameterRule(std::string key, ParamRule rule)
{
  mParamRules.insert_or_assign( key, rule );
}

ParameterManager::ParamRule ParameterManager::getParameterRule(std::string key)
{
  return mParamRules.contains( key ) ? mParamRules[key] : ParameterManager::ParamRule(ParamType::TYPE_STRING);
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

std::vector<ParameterManager::Param> ParameterManager::getParameters(std::string wildcardKeys)
{
  std::vector<ParameterManager::Param> result;
  if( wildcardKeys.ends_with("*") ){
    wildcardKeys = wildcardKeys.substr( 0, wildcardKeys.length()-1 );

    for( auto& [aKey, value] : mParams ){
      if( aKey.starts_with(wildcardKeys) ){
        result.push_back( Param(aKey, value) );
      }
    }
  }

  return result;
}

std::vector<ParameterManager::Param> ParameterManager::getParameters(std::vector<std::string> keys)
{
  std::vector<Param> result;

  if( !keys.empty() ){
    for( auto& aKey : keys) {
      if( !aKey.ends_with("*") ){
        std::string value = getParameter( aKey );
        result.push_back( Param(aKey, value) );
      } else {
        const auto& values = getParameters( aKey );
        result.insert( result.end(), values.begin(), values.end() );
      }
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


bool ParameterManager::storeToStream(IStream* pStream)
{
  bool result = false;
  if( pStream ){
    for( auto& [aKey, value] : mParams ){
      std::string buf = "\"" + aKey + "\":\"" + value + "\"";
      pStream->writeLine( buf );
      result = true;
    }
  }
  return result;
}

std::string ParameterManager::trimParamString(std::string value)
{
  const static std::string trimString = " \r\n\"";

  int nPos = value.find_last_not_of( trimString );
  if( nPos != std::string::npos ){
    value = value.substr( 0, nPos+1 );
  }

  nPos = value.find_first_not_of( trimString );
  if( nPos != std::string::npos ){
    value = value.substr( nPos );
  }

  return value;
}


bool ParameterManager::restoreFromStream(IStream* pStream, bool bOverride)
{
  bool result = false;

  if( pStream ){
    std::string aLine;
    while( !pStream->isEndOfStream() ){
      if( pStream->readLine( aLine ) ){
        StringTokenizer tok( aLine, "\":\"");
        if( tok.hasNext() ){
          result = true;
          std::string key = trimParamString( tok.getNext() );
          std::string value = trimParamString( tok.getNext() );
          if( bOverride || !mParams.contains( key ) ){
            setParameter( key, value );
          }
        }
      }
    }
  }

  return result;
}

void ParameterManager::resetAllOfParams(void)
{
  mParams.clear();
}

