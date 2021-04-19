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

#ifndef __SINK_HPP__
#define __SINK_HPP__

#include <string>
#include <iostream>
#include "Buffer.hpp"
#include "Util.hpp"

class Sink
{
protected:
  ByteBuffer mBuf;

public:
  Sink(){};
  virtual ~Sink(){};
  virtual void write(ByteBuffer& buf){
    mBuf.reserve(buf.size() + mBuf.size() );
    std::copy( buf.begin(), buf.end(), std::back_inserter(mBuf) );
  };
  virtual std::string toString(void){return "Sink";}
  virtual void dump(void){
    Util::dumpBuffer("Dump Sink data", mBuf);
  }

  enum PRESENTATION {
    SPEAKER_MONO,
    SPEAKER_STEREO,
    SPEAKER_FL_FR_SL_SR,
    SPEAKER_FL_FR_C_SL_SR,
    SPEAKER_FL_FR_C_SL_SR_SW,
    SPEAKER_FL_FR_C_SL_SR_SW_EL_ER, // Atmos Enabled L/R
    HEADPHONE_MONO,
    HEADPHONE_STEREO,
    PRESENTATION_DEFAULT = SPEAKER_STEREO,
    UNKNOWN
  };
};

#endif /* __SINK_HPP__ */