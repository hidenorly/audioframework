/* 
  Copyright (C) 2021, 2023, 2024 hidenorly

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

#ifndef __THREAD_BASE_HPP__
#define __THREAD_BASE_HPP__

#include <memory>
#include <vector>
#include <mutex>
#include <thread>
#include <atomic>

class ThreadBase
{
protected:
  std::mutex mMutexThread;
  std::shared_ptr<std::thread> mpThread;
  std::atomic<bool> mbIsRunning;
  bool mIsPreviousRunning;

public:
  ThreadBase();
  virtual ~ThreadBase();

  virtual void run(void);
  virtual void stop(void);
  virtual bool isRunning(void);

protected:
  virtual void process(void);
  static void _execute(ThreadBase* pThis);
  virtual void unlockToStop(void);

public:
  class RunnerListener
  {
  public:
    RunnerListener(){};
    virtual ~RunnerListener(){};
    virtual void onRunnerStatusChanged(bool bRunning){};
  };

  void registerRunnerStatusListener(std::shared_ptr<RunnerListener> listener);
  void unregisterRunnerStatusListener(std::shared_ptr<RunnerListener> listener);
  void unregisterRunnerStatusListener(RunnerListener* listener);

protected:
  std::vector<std::weak_ptr<RunnerListener>> mRunnerListerners;
  void notifyRunnerStatusChanged(void);
};

#endif /* __THREAD_BASE_HPP__ */
