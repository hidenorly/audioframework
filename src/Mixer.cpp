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

#include "Mixer.hpp"
#include <vector>

#ifndef USE_TINY_MIXER_IMPL
  #define USE_TINY_MIXER_IMPL 1
#endif /* USE_TINY_MIXER_IMPL */

#if USE_TINY_MIXER_IMPL

bool Mixer::process( std::vector<AudioBuffer*> pInBuffers, AudioBuffer* pOutBuffer )
{
  return true;
}

#endif /* USE_TINY_MIXER_IMPL */