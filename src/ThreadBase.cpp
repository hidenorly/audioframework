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

#include "ThreadBase.hpp"

ThreadBase::ThreadBase():mpThread(nullptr), mbIsRunning(false), mIsPreviousRunning(false)
{

}

ThreadBase::~ThreadBase()
{
  stop();
}

void ThreadBase::run(void)
{
  mMutexThread.lock();
  if( !mbIsRunning && !mpThread ){
    mbIsRunning = true;
    mpThread = new std::thread(_execute, this);
  }
  mMutexThread.unlock();
  notifyRunnerStatusChanged();
}

void ThreadBase::unlockToStop(void)
{
  // DO UNLOCK THE BLOCKING FOR STOPPING YOUR PROCESS
}

void ThreadBase::stop(void)
{
  mMutexThread.lock();
  if( mbIsRunning ){
    mbIsRunning = false;
    while( mpThread ){
      unlockToStop();
      std::this_thread::sleep_for(std::chrono::microseconds(100));
      if( mpThread->joinable() ){
        mpThread->join();
        delete mpThread;
        mpThread = nullptr;
      }
    }
  }
  mMutexThread.unlock();

  notifyRunnerStatusChanged();
}

bool ThreadBase::isRunning(void)
{
  return mbIsRunning;
}

void ThreadBase::process(void)
{
}

void ThreadBase::_execute(ThreadBase* pThis)
{
  pThis->process();
  pThis->mbIsRunning = false;
}


void ThreadBase::notifyRunnerStatusChanged(void)
{
  if( mbIsRunning != mIsPreviousRunning ){
    mIsPreviousRunning = mbIsRunning;
    for(auto& aListener : mRunnerListerners ){
      std::shared_ptr<ThreadBase::RunnerListener> theListener = aListener.lock();
      if( theListener ){
        theListener->onRunnerStatusChanged( mbIsRunning );
      }
    }
  }
}

void ThreadBase::registerRunnerStatusListener(std::shared_ptr<ThreadBase::RunnerListener> listener)
{
  std::weak_ptr<ThreadBase::RunnerListener> theListener(listener);
  mRunnerListerners.push_back( theListener );
}

void ThreadBase::unregisterRunnerStatusListener(std::shared_ptr<ThreadBase::RunnerListener> listener)
{
  std::weak_ptr<ThreadBase::RunnerListener> theListener(listener);

  const auto pos = std::find_if(mRunnerListerners.begin(), mRunnerListerners.end(), [&theListener](const std::weak_ptr<ThreadBase::RunnerListener>& aListener) {
          return aListener.lock() == theListener.lock();
      });

  if (pos != mRunnerListerners.end()){
    mRunnerListerners.erase(pos);
  }
}