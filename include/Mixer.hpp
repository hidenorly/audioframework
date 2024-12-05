/* 
  Copyright (C) 2021, 2024 hidenorly

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

#ifndef __MIXER_HPP__
#define __MIXER_HPP__

#include "Buffer.hpp"
#include <vector>

class Mixer
{
public:
  static bool process( std::vector<std::shared_ptr<AudioBuffer>> pInBuffers, std::shared_ptr<AudioBuffer> pOutBuffer );
protected:
  static bool doMix( std::vector<std::shared_ptr<AudioBuffer>> pInBuffers, std::shared_ptr<AudioBuffer> pOutBuffer );
  static bool doMixPrimitive( std::shared_ptr<AudioBuffer> pInBuffer1, std::shared_ptr<AudioBuffer> pInBuffer2, std::shared_ptr<AudioBuffer> pOutBuffer );
};

#endif /* __MIXER_HPP__ */