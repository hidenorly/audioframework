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

#include "Testability.hpp"
#include "PowerManager.hpp"
#include "PowerManagerPrimitive.hpp"
#include <string>
#include <thread>
#include <iostream>
#include <fstream>
#include <filesystem>

#if __AFW_TEST__
// for test environement
void PowerManagerPrimitive::setPowerState(IPowerManager::POWERSTATE powerState )
{
  IPowerManagerAdmin* pManager = dynamic_cast<IPowerManagerAdmin*>( PowerManager::getManager() );
  if( pManager ){
    pManager->setPowerState( powerState );
  }
}

ITestable* PowerManagerPrimitive::getTestShim(void)
{
 return this; 
}

void PowerManagerPrimitive::process(void)
{
}
#endif /* __AFW_TEST__ */


#if __linux__
// for Linux
PowerManagerPrimitive::PowerManagerPrimitive()
{

}

PowerManagerPrimitive::~PowerManagerPrimitive()
{
  terminate();
}

void PowerManagerPrimitive::initialize(void)
{
  run();
}

void PowerManagerPrimitive::terminate(void)
{
  stop();
}

static constexpr char sys_power_wakeup_count[] = "/sys/power/wakeup_count";
static constexpr int RESUME_CHECK_DURATION = 100000;

void PowerManagerPrimitive::process(void)
{
  // TODO: For implementer
  //       Before echo "mem" > /sys/power/state, setPowerState( IPowerManager::POWERSTATE::SUSPEND );
  // Monitor resume from suspend by reading /sys/power/wakeup_count
  if( std::filesystem::exists( sys_power_wakeup_count ) ){
    std::fstream stream( sys_power_wakeup_count, std::ios::in );
    std::string count;
    std::getline( stream, count );
    int lastWakeupCount = std::stoi( count );

    while( isRunning() ){
      stream.seekp( 0 );
      std::getline( stream, count );
      int nCount = std::stoi( count );
      if( (nCount!=lastWakeupCount) ){
        setPowerState( IPowerManager::POWERSTATE::ACTIVE ); // resume from suspend
        lastWakeupCount = nCount;
      }
      std::this_thread::sleep_for(std::chrono::microseconds(RESUME_CHECK_DURATION));
    }

    stream.close();
  }
}
/* end of __linux */
#elif __APPLE__
// for MacOS
PowerManagerPrimitive::PowerManagerPrimitive()
{

}

PowerManagerPrimitive::~PowerManagerPrimitive()
{
  terminate();
}

void PowerManagerPrimitive::initialize(void)
{
  run();
}

void PowerManagerPrimitive::terminate(void)
{
  stop();
}
/* end of __APPLE__ */
#else
// for the others
PowerManagerPrimitive::PowerManagerPrimitive()
{

}

PowerManagerPrimitive::~PowerManagerPrimitive()
{
  terminate();
}

void PowerManagerPrimitive::initialize(void)
{
  run();
}

void PowerManagerPrimitive::terminate(void)
{
  stop();
}
#endif