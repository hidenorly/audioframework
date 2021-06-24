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

#include "Strategy.hpp"

Strategy::Strategy()
{

}

Strategy::~Strategy()
{
  mStrategies.clear();
}


bool Strategy::execute(std::shared_ptr<StrategyContext> context)
{
  bool result = false;
 
  for( auto& aStrategy : mStrategies){
    if( aStrategy->canHandle( context ) ){
      result = aStrategy->execute( context );
      break;
    }
  }
 
  return result;
}


void Strategy::registerStrategy(std::shared_ptr<IStrategy> aStrategy)
{
  mStrategies.push_back( aStrategy );
}

void Strategy::unregisterStrategy(std::shared_ptr<IStrategy> aStrategy)
{
  if( aStrategy ){
    std::erase( mStrategies, aStrategy );
  }
}