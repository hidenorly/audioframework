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

ThreadBase::ThreadBase():mbIsRunning(false), mpThread(nullptr)
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
    mpThread = new std::thread(_execute, this);
    mbIsRunning = true;
  }
  mMutexThread.unlock();
}

void ThreadBase::stop(void)
{
  mMutexThread.lock();
  if( mpThread ){
    mbIsRunning = false;
    std::this_thread::sleep_for(std::chrono::microseconds(100));
    while( mpThread ){
      if( mpThread->joinable() ){
          mpThread->join();
          delete mpThread;
          mpThread = nullptr;
      }
    }
  }
  mMutexThread.unlock();
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
