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

#ifndef __UTIL_HPP__
#define __UTIL_HPP__

#include <iostream>
#include <string>
#include "Buffer.hpp"

class Util
{
public:
  static void dumpBuffer(AudioBuffer& buf)
  {
    AudioFormat format = buf.getAudioFormat();
    std::cout << "sampling rate:" << format.getSamplingRate() <<
      " format:" << format.getEncodingString() <<
      " channels:" << (int)format.getNumberOfChannels() <<
      " samples:" << buf.getSamples() <<
      std::endl;

    ByteBuffer rawBuffer = buf.getRawBuffer();
    for(auto& aData : rawBuffer){
//      std::cout << (int)aData << ",";
      std::cout << std::hex << (int)aData << ",";
    }
    std::cout << std::endl;
  }
  static void dumpBuffer(std::string message, AudioBuffer& buf){
      std::cout << message << std::endl;
      dumpBuffer(buf);
  }
};

#endif /* __UTIL_HPP__ */