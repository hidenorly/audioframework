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


ParameterManager::ParameterManager()
{

}

ParameterManager::~ParameterManager()
{

}

void ParameterManager::setParameter(std::string key, std::string value)
{
  mParams.insert( std::make_pair(key, value) );
}

void ParameterManager::setParameterInt(std::string key, int value)
{
  mParams.insert( std::make_pair(key, std::to_string(value)) );
}

void ParameterManager::setParameterFloat(std::string key, float value)
{
  mParams.insert( std::make_pair(key, std::to_string(value)) );
}

void ParameterManager::setParameterBool(std::string key, bool value)
{
  mParams.insert( std::make_pair(key, value ? "true" : "false") );
}

void ParameterManager::setParameters(std::vector<ParameterManager::Param>& params)
{
  for(auto& aParam : params){
    mParams.insert( std::make_pair(aParam.key, aParam.value) );
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