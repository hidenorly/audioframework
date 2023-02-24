/*
  Copyright (C) 2021, 2023 hidenorly

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

#include "ResourceManager.hpp"
#include <algorithm>
#include <memory>

#ifndef __USE_DUMMY_CPU_RESOURCE_IMPL_
  #define __USE_DUMMY_CPU_RESOURCE_IMPL_ 1
#endif /* __USE_DUMMY_CPU_RESOURCE_IMPL_ */

#if __USE_DUMMY_CPU_RESOURCE_IMPL_
#ifndef CPU_RESOURCE_VALUE
#define CPU_RESOURCE_VALUE 1000000 // 1000DMIPS * 1000
#endif /* CPU_RESOURCE_VALUE */

int CpuResource::getComputingResource(void)
{
  return CPU_RESOURCE_VALUE;
}
#endif /* __USE_DUMMY_CPU_RESOURCE_IMPL_ */

int CpuResource::convertFromProcessingTimeToConsumptionResource(int processingTimeUsec)
{
  return (int)((float)getComputingResource() * (float)processingTimeUsec / 1000000.0f);
}

int CpuResource::convertFromConsumptionResourceToProcessingTime(int consumptionResource)
{
  return (int)( 1000000.0f * (float)consumptionResource / (float)getComputingResource() );
}

void IResourceConsumer::storeResourceConsumptionId(int resourceId, std::weak_ptr<IResourceManager> pResourceManager)
{
  mResourceConsumptionId = resourceId;
  mpResourceManager = pResourceManager;
}

int IResourceConsumer::restoreResourceConsumptionId(void)
{
  int nResourceConsumptionId = mResourceConsumptionId;
  mResourceConsumptionId = -1;
  return nResourceConsumptionId;
}

IResourceConsumer::~IResourceConsumer()
{
  std::shared_ptr<IResourceManager> pResourceManager = mpResourceManager.lock();
  if( pResourceManager && (-1 != mResourceConsumptionId) ){
    pResourceManager->release( this );
  }
}

bool IResourceConsumer::isResourceConsumed(void)
{
  return ( -1 != mResourceConsumptionId );
}

void IResourceConsumer::clearResourceManager(void)
{
  mpResourceManager.reset();
}


IResourceManager::IResourceManager(int resource) : mResource(resource), mResourceCurrent(0), mId(0)
{

}

IResourceManager::~IResourceManager()
{
  for( auto& aConsumer : mResourceConsumers ){
    auto&& pConsumer = aConsumer.lock();
    if( pConsumer ){
      pConsumer->clearResourceManager();
    }
  }
}

int IResourceManager::acquire(int requiredResource)
{
  int result = -1;

  mMutexResource.lock();
  {
    if( (requiredResource+mResourceCurrent) <= mResource ){
      mResourceCurrent += requiredResource;
      result = mId++;
      mResources[result] = requiredResource;
    }
  }
  mMutexResource.unlock();

  return result;
}

bool IResourceManager::release(int nId)
{
  bool result = false;

  mMutexResource.lock();
  {
    if( mResources.contains(nId) ){
      mResourceCurrent -= mResources[nId];
      mResources.erase( nId );
      result = true;
    }
  }
  mMutexResource.unlock();

  return result;
}

bool IResourceManager::acquire(std::weak_ptr<IResourceConsumer> consumer)
{
  std::shared_ptr<IResourceConsumer> pConsumer = consumer.lock();
  if( pConsumer && !pConsumer->isResourceConsumed() ){
    int nResourceConsumptionId = acquire( pConsumer->stateResourceConsumption() ) ;
    pConsumer->storeResourceConsumptionId( nResourceConsumptionId, mpInstance );
    mResourceConsumers.push_back( consumer );
    return (  nResourceConsumptionId != -1 );
  } else {
    return false;
  }
}

bool IResourceManager::release(std::weak_ptr<IResourceConsumer> consumer)
{
  std::shared_ptr<IResourceConsumer> pConsumer = consumer.lock();
  return release( pConsumer.get() );
}

bool IResourceManager::release(IResourceConsumer* consumer)
{
  if( consumer ){
    const auto pos = std::find_if(mResourceConsumers.begin(), mResourceConsumers.end(), [&consumer](const std::weak_ptr<IResourceConsumer>& aConsumer) {
            return aConsumer.lock().get() == consumer;
        });

    if (pos != mResourceConsumers.end()){
      mResourceConsumers.erase(pos);
    }
    return release( consumer->restoreResourceConsumptionId() );
  }
  return false;
}

CpuResourceManager::CpuResourceManager(int resource):IResourceManager(resource)
{

}

CpuResourceManager::~CpuResourceManager()
{
}

std::weak_ptr<IResourceManager> CpuResourceManager::getInstance(void)
{
  return mpInstance;
};


void CpuResourceManager::admin_setResource(int resource)
{
  if( mpInstance ){
    mpInstance.reset();
  }
  mpInstance = std::shared_ptr<CpuResourceManager>( new CpuResourceManager( resource ) );
}

void CpuResourceManager::admin_terminate(void)
{
  mpInstance.reset();
}
