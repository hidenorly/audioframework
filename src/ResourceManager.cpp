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

IResourceManager::IResourceManager(int resource) : mResource(resource), mResourceCurrent(0), mId(0)
{

}

IResourceManager::~IResourceManager()
{

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

