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

}

void PowerManagerPrimitive::terminate(void)
{

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

}

void PowerManagerPrimitive::terminate(void)
{

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

}

void PowerManagerPrimitive::terminate(void)
{

}
#endif