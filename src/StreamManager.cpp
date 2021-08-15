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

#include "StreamManager.hpp"


std::shared_ptr<StreamInfo> StreamManager::get(int id)
{
  std::shared_ptr<StreamInfo> pStreamInfo = nullptr;
  for(auto& aStreamInfo : mStreamInfos){
    if( aStreamInfo->id == id){
      pStreamInfo = aStreamInfo;
      break;
    }
  }
  return pStreamInfo;
}

std::shared_ptr<StreamInfo> StreamManager::get(std::shared_ptr<IPipe> pPipe)
{
  std::shared_ptr<StreamInfo> pStreamInfo = nullptr;
  for(auto& aStreamInfo : mStreamInfos){
    if( aStreamInfo->pipe == pPipe){
      pStreamInfo = aStreamInfo;
      break;
    }
  }
  return pStreamInfo;
}

std::shared_ptr<StreamInfo> StreamManager::get(std::shared_ptr<StrategyContext> pContext)
{
  std::shared_ptr<StreamInfo> pStreamInfo = nullptr;
  for(auto& aStreamInfo : mStreamInfos){
    if( aStreamInfo->context == pContext){
      pStreamInfo = aStreamInfo;
      break;
    }
  }
  return pStreamInfo;
}

int StreamManager::add(std::shared_ptr<StrategyContext> pContext, std::shared_ptr<IPipe> pPipe)
{
  int resultId = -1;

  if( pContext && pPipe ){
    std::shared_ptr<StreamInfo> pStreamInfo = std::make_shared<StreamInfo>();
    resultId = pStreamInfo->id = mId;
    pStreamInfo->context = pContext;
    pStreamInfo->pipe = pPipe;
    mStreamInfos.push_back( pStreamInfo );
    mId++;
  }

  return resultId;
}

int StreamManager::add(std::shared_ptr<StreamInfo> pStreamInfo )
{
  int resultId = -1;
  if( pStreamInfo ){
    pStreamInfo->id = mId;
    mStreamInfos.push_back( pStreamInfo );
    mId++;
  }
  return resultId;
}

bool StreamManager::remove(int id)
{
  std::shared_ptr<StreamInfo> pStreamInfo = get(id);
  if( pStreamInfo ){
    std::erase( mStreamInfos, pStreamInfo );
    pStreamInfo.reset();
  }
  return (pStreamInfo!=nullptr);
}

bool StreamManager::remove(std::shared_ptr<IPipe> pPipe)
{
  std::shared_ptr<StreamInfo> pStreamInfo = get(pPipe);
  if( pStreamInfo ){
    std::erase( mStreamInfos, pStreamInfo );
    pStreamInfo.reset();
  }
  return (pStreamInfo!=nullptr);
}

bool StreamManager::remove(std::shared_ptr<StrategyContext> pContext)
{
  std::shared_ptr<StreamInfo> pStreamInfo = get(pContext);
  if( pStreamInfo ){
    std::erase( mStreamInfos, pStreamInfo );
    pStreamInfo.reset();
  }
  return (pStreamInfo!=nullptr);
}

bool StreamManager::remove(std::shared_ptr<StreamInfo> pStreamInfo)
{
  bool result = false;
  if( pStreamInfo ){
    int nPrevSize = mStreamInfos.size();
    std::erase( mStreamInfos, pStreamInfo );
    pStreamInfo.reset();
    result = ( nPrevSize != mStreamInfos.size() );
  }
  return result;
}

void StreamManager::clear(void)
{
  for(auto& aStreamInfo : mStreamInfos){
    aStreamInfo.reset();
  }
  mStreamInfos.clear();
}

int StreamManager::getId(std::shared_ptr<IPipe> pPipe)
{
  std::shared_ptr<StreamInfo> pStreamInfo = get( pPipe );

  return pStreamInfo ? pStreamInfo->id : -1;
}

int StreamManager::getId(std::shared_ptr<StrategyContext> pContext)
{
  std::shared_ptr<StreamInfo> pStreamInfo = get( pContext );

  return pStreamInfo ? pStreamInfo->id : -1;
}

std::shared_ptr<IPipe> StreamManager::getPipe(int id)
{
  std::shared_ptr<StreamInfo> pStreamInfo = get( id );

  return pStreamInfo ? pStreamInfo->pipe : nullptr;
}

std::shared_ptr<IPipe> StreamManager::getPipe(std::shared_ptr<StrategyContext> pContext)
{
  std::shared_ptr<StreamInfo> pStreamInfo = get( pContext );

  return pStreamInfo ? pStreamInfo->pipe : nullptr;
}

std::shared_ptr<StrategyContext> StreamManager::getContext(int id)
{
  std::shared_ptr<StreamInfo> pStreamInfo = get( id );

  return pStreamInfo ? pStreamInfo->context : nullptr;
}

std::shared_ptr<StrategyContext> StreamManager::getContext(std::shared_ptr<IPipe> pPipe)
{
  std::shared_ptr<StreamInfo> pStreamInfo = get( pPipe );

  return pStreamInfo ? pStreamInfo->context : nullptr;
}