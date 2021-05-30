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

#ifndef __RESOURCEMANAGER_HPP__
#define __RESOURCEMANAGER_HPP__

#include <map>
#include <mutex>
#include <vector>

class CpuResource
{
protected:
  static int getComputingResource(void);
public:
  static int convertFromProcessingTimeToConsumptionResource(int processingTimeUsec);
  static int convertFromConsumptionResourceToProcessingTime(int consumptionResource);
};

class IResourceConsumer;

class IResourceManager
{
protected:
  std::map<int, int> mResources;
  std::vector<IResourceConsumer*> mResourceConsumers;
  std::mutex mMutexResource;
  int mId;
  int mResource;
  int mResourceCurrent;
  IResourceManager(int resource);
  virtual ~IResourceManager();

public:
  // acquire: return acquired resource id : -1 means fail.
  virtual int acquire(int requiredResource);
  virtual bool release(int nId);
  virtual bool acquire(IResourceConsumer& consumer);
  virtual bool acquire(IResourceConsumer* consumer);
  virtual bool release(IResourceConsumer& consumer);
  virtual bool release(IResourceConsumer* consumer);
};

class CpuResourceManager;

class CpuResourceManager : public IResourceManager
{
protected:
  CpuResourceManager(int resource);
  virtual ~CpuResourceManager();
  static inline CpuResourceManager* mpInstance = nullptr;
public:
  static IResourceManager* getInstance(void);

  static void admin_setResource(int resource);
  static void admin_terminate(void);
};

class IResourceConsumer
{
protected:
  int mResourceConsumptionId;
  IResourceManager* mpResourceManager;

  IResourceConsumer():mpResourceManager(nullptr), mResourceConsumptionId(-1){};
  virtual ~IResourceConsumer();

protected:
  friend IResourceManager;

  virtual int stateResourceConsumption(void) = 0;
  void storeResourceConsumptionId(int resourceId, IResourceManager* pResourceManager = nullptr);
  int restoreResourceConsumptionId(void);
  bool isResourceConsumed(void);
  void clearResourceManager(void);
};

#endif /* __RESOURCEMANAGER_HPP__ */
