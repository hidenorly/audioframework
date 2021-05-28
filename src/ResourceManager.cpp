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

#include "ResourceManager.hpp"

void IResourceConsumer::storeResourceConsumptionId(int resourceId, IResourceManager* pResourceManager)
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
  if( mpResourceManager && (-1 != mResourceConsumptionId) ){
    mpResourceManager->release( *this );
  }
}

bool IResourceConsumer::isResourceConsumed(void)
{
  return ( -1 != mResourceConsumptionId );
}

void IResourceConsumer::clearResourceManager(void)
{
  mpResourceManager = nullptr;
}


IResourceManager::IResourceManager(int resource) : mResource(resource), mResourceCurrent(0), mId(0)
{

}

IResourceManager::~IResourceManager()
{
  for( auto& aConsumer : mResourceConsumers ){
    aConsumer->clearResourceManager();
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

bool IResourceManager::acquire(IResourceConsumer& consumer)
{
  if( !consumer.isResourceConsumed() ){
    int nResourceConsumptionId = acquire( consumer.stateResourceConsumption() ) ;
    consumer.storeResourceConsumptionId( nResourceConsumptionId, this );
    mResourceConsumers.push_back( &consumer );
    return (  nResourceConsumptionId != -1 );
  } else {
    return false;
  }
}

bool IResourceManager::release(IResourceConsumer& consumer)
{
  std::erase( mResourceConsumers, &consumer );
  return release( consumer.restoreResourceConsumptionId() );
}

bool IResourceManager::acquire(IResourceConsumer* consumer)
{
  return consumer ? acquire(*consumer) : false;
}

bool IResourceManager::release(IResourceConsumer* consumer)
{
  return consumer ? release(*consumer) : false;
}

CpuResourceManager::CpuResourceManager(int resource):IResourceManager(resource)
{

}

CpuResourceManager::~CpuResourceManager()
{
}

IResourceManager* CpuResourceManager::getInstance(void)
{
  return mpInstance;
};


void CpuResourceManager::admin_setResource(int resource)
{
  if( mpInstance ){
    delete mpInstance;
    mpInstance = nullptr;
  }
  mpInstance = new CpuResourceManager( resource );
}

void CpuResourceManager::admin_terminate(void)
{
  delete mpInstance;
  mpInstance = nullptr;
}

