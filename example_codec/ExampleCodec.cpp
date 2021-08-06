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

#include "Media.hpp"
#include "Decoder.hpp"
#include "AudioFormat.hpp"
#include "Buffer.hpp"
#include <string>
#include <iostream>

class CodecExampleNullDecoder;
class CodecExampleNullDecoder : public IDecoder
{
protected:
  AudioFormat mFormat;

public:
  CodecExampleNullDecoder( AudioFormat format = AudioFormat(AudioFormat::ENCODING::COMPRESSED) ):IDecoder(),mFormat(format){};
  ~CodecExampleNullDecoder(){};

  virtual void configure(MediaParam param){
    std::cout << param.key << ":=" << param.value << std::endl;
  };
  virtual int stateResourceConsumption(void){ return 0; };

  virtual int getEsChunkSize(void){ return 256; /* dummy size of ES chunk */ };
  virtual void doProcess(IAudioBuffer& inBuf, IAudioBuffer& outBuf){
    outBuf.setRawBuffer(inBuf.getRawBuffer());
  }
  virtual AudioFormat getFormat(void){ return mFormat; };
  virtual bool canHandle(AudioFormat format){ return format.isEncodingCompressed(); }; // since this is null decoder. actual codec shouldn't override this and should report correct supported format in getFormat()

  virtual std::string toString(void){ return "CodecExampleNullDecoder"; };

  /* @desc initialize at loading the codec plug-in shared object such as .so */
  virtual void onLoad(void){
    std::cout << "onLoad" << std::endl;
  }
  /* @desc uninitialize at unloading the codec plug-in shared object such as .so */
  virtual void onUnload(void){
    std::cout << "onUnload" << std::endl;
  }
  /* @desc report the codec plug-in's unique id
     @return unique plug-in id. may use uuid. */
  virtual std::string getId(void){
    return std::string("CodecExampleNullDecoder");
  }
  /* @desc this is expected to use by strategy
     @return new instance of this class result */
  virtual std::shared_ptr<IPlugIn> newInstance(void){
    return std::make_shared<CodecExampleNullDecoder>();
  }
};


extern "C"
{
void* getPlugInInstance(void)
{
  CodecExampleNullDecoder* pInstance = new CodecExampleNullDecoder();
  return reinterpret_cast<void*>(dynamic_cast<IPlugIn*>(pInstance));
}
};