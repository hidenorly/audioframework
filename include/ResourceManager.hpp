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
#include <memory>

class CpuResource
{
public:
  /* CPU resource DKIPS (DMIPS*1000)
     @return DMIPS * 1000 */
  static int getComputingResource(void);
  /* Convert processing time to CPU resource DKIPS (DMIPS*1000)
     @return DMIPS * 1000 */
  static int convertFromProcessingTimeToConsumptionResource(int processingTimeUsec);
  /* Convert CPU resource DKIPS (DMIPS*1000) to processing time
     @return processing time [USec] */
  static int convertFromConsumptionResourceToProcessingTime(int consumptionResource);
};

class IResourceConsumer;

class IResourceManager
{
protected:
  std::map<int, int> mResources;
  std::vector<std::weak_ptr<IResourceConsumer>> mResourceConsumers;
  std::mutex mMutexResource;
  int mResource;
  int mResourceCurrent;
  int mId;
  static inline std::shared_ptr<IResourceManager> mpInstance;
  IResourceManager(int resource);

public:
  virtual ~IResourceManager();
  /* @desc acquire computing resource
     @return acquired resource id : -1 means fail. */
  virtual int acquire(int requiredResource);
  /* @desc release acquired resource
     @arg nId : acquired resource Id which is returned by acquire()
     @return acquired resource id : -1 means fail. */
  virtual bool release(int nId);
  /* @desc acquire computing resource
     @arg IResourceConsumer which is implemented by ISink, ISource, IFilter instance
     @return true: success to acquire, false: fail to acquire */
  virtual bool acquire(std::weak_ptr<IResourceConsumer> consumer);
  /* @desc release acquired resource
     @arg IResourceConsumer which is implemented by ISink, ISource, IFilter instance
     @return true: success to release, false: fail to release */
  virtual bool release(std::weak_ptr<IResourceConsumer> consumer);
  virtual bool release(IResourceConsumer* consumer);
};

class CpuResourceManager;

class CpuResourceManager : public IResourceManager
{
protected:
  // Computing power as per-second (DMIPS)
  CpuResourceManager(int resource);
public:
  virtual ~CpuResourceManager();
  /* @desc get CPU Resource Manager
     @return instance of CpuResourceManager */
  static std::weak_ptr<IResourceManager> getInstance(void);

  /* @desc set CPU Resource Manager's resource
     @arg specify the resource value (DMIPS*1000) */
  static void admin_setResource(int resource);
  /* @desc dispose the CpuResource instance */
  static void admin_terminate(void);
};

/* This should be implemented in ISink, ISource, IFilter delived classes */
class IResourceConsumer
{
protected:
  std::weak_ptr<IResourceManager> mpResourceManager;
  int mResourceConsumptionId;

  IResourceConsumer():mResourceConsumptionId(-1){};

protected:
  friend IResourceManager;

  // state required computing power as per-second (DMIPS*1000) (in case of CpuResource)
  void storeResourceConsumptionId(int resourceId, std::weak_ptr<IResourceManager> pResourceManager);
  int restoreResourceConsumptionId(void);
  bool isResourceConsumed(void);
  void clearResourceManager(void);

public:
  virtual ~IResourceConsumer();
  /* this should be implemented in the delived class to report consuming resource for IResourceManager delived class such as CpuResourceManager */
  virtual int stateResourceConsumption(void) = 0;
};

#endif /* __RESOURCEMANAGER_HPP__ */