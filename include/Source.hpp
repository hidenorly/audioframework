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

#ifndef __SOURCE_HPP__
#define __SOURCE_HPP__

#include <string>
#include "Buffer.hpp"
#include "AudioFormat.hpp"
#include "PipeAndFilterCommon.hpp"

class ISource : public ISourceSinkCommon
{
public:
  virtual void read(AudioBuffer& buf) = 0;
  virtual ~ISource(){};
};

class Source : public AudioBase, public ISource
{
public:
  Source(){};
  virtual ~Source(){};
  virtual void read(AudioBuffer& buf){
    ByteBuffer rawBuffer = buf.getRawBuffer();
    ByteBuffer bufZero(rawBuffer.size(), 128);
    uint8_t* ptr = bufZero.data();
    for(int i=0; i<bufZero.size(); i++){
      *ptr++ = i % 256;
    }
    rawBuffer = bufZero;
    buf.setRawBuffer( rawBuffer );
  };
  virtual std::string toString(void){return "Source";};
};

#endif /* __SOURCE_HPP__ */