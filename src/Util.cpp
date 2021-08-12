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

#include "Util.hpp"

void Util::dumpBuffer(IAudioBuffer* pBuf)
{
  if(pBuf){
    std::lock_guard<std::mutex> lock(Util::mPrintMutex);
    AudioFormat format = pBuf->getAudioFormat();
    AudioBuffer* pAudioBuf = dynamic_cast<AudioBuffer*>(pBuf);
    if( pAudioBuf ){
      std::cout << "sampling rate:" << (int)format.getSamplingRate() <<
        " format:" << format.getEncodingString() <<
        " channels:" << (int)format.getNumberOfChannels() <<
        " samples:" << (int)pAudioBuf->getNumberOfSamples() <<
        std::endl;
      } else {
      std::cout << " format:" << format.getEncodingString() <<
        " data size:" << (int)pBuf->getRawBufferSize() <<
        std::endl;
      }

    std::cout << std::hex;
    ByteBuffer rawBuffer = pBuf->getRawBuffer();
    for(auto& aData : rawBuffer){
      std::cout << (int)aData << ",";
    }
    std::cout << std::dec << std::endl;
  }
}

void Util::dumpBuffer(std::shared_ptr<IAudioBuffer> pBuf)
{
  dumpBuffer( pBuf.get() );
}


void Util::dumpBuffer(IAudioBuffer& buf)
{
  dumpBuffer(&buf);
}

void Util::dumpBuffer(std::string message, IAudioBuffer* buf)
{
  std::cout << message << std::endl;
  dumpBuffer(buf);
}

void Util::dumpBuffer(std::string message, IAudioBuffer& buf)
{
  dumpBuffer(message, &buf);
}

void Util::dumpBuffer(std::string message, std::shared_ptr<IAudioBuffer> buf)
{
  dumpBuffer(message, buf.get());
}
