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

#include "Source.hpp"
#include "Buffer.hpp"
#include <cmath>
#include <iostream>

class SourceExampleSin;
class SourceExampleSin : public SourcePlugIn
{
protected:
  std::vector<AudioFormat> mSupportedFormats;

public:
  SourceExampleSin(){
    for(int anEncoding = AudioFormat::ENCODING::PCM_8BIT; anEncoding < AudioFormat::ENCODING::COMPRESSED_UNKNOWN; anEncoding++){
      for( int aChannel = AudioFormat::CHANNEL::CHANNEL_MONO; aChannel < AudioFormat::CHANNEL::CHANNEL_UNKNOWN; aChannel++){
        mSupportedFormats.push_back( AudioFormat((AudioFormat::ENCODING)anEncoding, 48000, (AudioFormat::CHANNEL)aChannel) );
        mSupportedFormats.push_back( AudioFormat((AudioFormat::ENCODING)anEncoding, 96000, (AudioFormat::CHANNEL)aChannel) );
      }
    }
  };
  virtual ~SourceExampleSin(){};
  virtual std::vector<AudioFormat> getSupportedAudioFormats(void){ return mSupportedFormats; };
  virtual bool isAvailableFormat(AudioFormat format){ return format.isEncodingPcm(); };

protected:
  virtual void readPrimitive(IAudioBuffer& buf){
    ByteBuffer esRawBuf( buf.getRawBuffer().size(), 0 );
    AudioBuffer* pBuf = dynamic_cast<AudioBuffer*>(&buf);
    static const float pi = 3.141592653589793f;
    if( pBuf ){
      AudioFormat format = pBuf->getAudioFormat();
      int nChannels = format.getNumberOfChannels();
      int nSamples = pBuf->getNumberOfSamples();

      int8_t* pVal8 = reinterpret_cast<int8_t*>(esRawBuf.data());
      int16_t* pVal16 = reinterpret_cast<int16_t*>(pVal8);
      int32_t* pVal32 = reinterpret_cast<int32_t*>(pVal8);
      float* pValFloat = reinterpret_cast<float*>(pVal8);

      for(int n = 0; n < nSamples; n++ ){
        float val = std::sin( 2 * pi * n / nSamples );
        int offset = n * nChannels;
        for( int c = 0; c < nChannels; c++ ){
          switch( format.getEncoding() ){
            case AudioFormat::ENCODING::PCM_8BIT:
              * (pVal8+offset+c) = (int8_t)( (float)INT8_MAX * val );
              break;
            case AudioFormat::ENCODING::PCM_16BIT:
              * (pVal16+offset+c) = (int8_t)( (float)INT16_MAX * val );
              break;
            case AudioFormat::ENCODING::PCM_32BIT:
              * (pVal32+offset+c) = (int32_t)( (float)INT32_MAX * val );
              break;
            case AudioFormat::ENCODING::PCM_24BIT_PACKED:
              {
                int32_t tmp = (int32_t)( (float)INT32_MAX * val );
                *(pVal8+offset+c+0) = (uint8_t)((tmp & 0x0000FF00) >> 8);
                *(pVal8+offset+c+1) = (uint8_t)((tmp & 0x00FF0000) >> 16);
                *(pVal8+offset+c+2) = (uint8_t)((tmp & 0xFF000000) >> 24);
              }
              break;
            case AudioFormat::ENCODING::PCM_FLOAT:
              *(pValFloat+offset+c) = val;
              break;
            case AudioFormat::ENCODING::PCM_UNKNOWN:
            default:
              offset = 0;
              break;
          }
        }
      }
    }
    buf.setRawBuffer( esRawBuf );
    buf.setAudioFormat( mFormat );
  }

public:
  virtual std::string toString(void){ return "SourceExampleSin"; };

  /* @desc initialize at loading the source plug-in shared object such as .so */
  virtual void onLoad(void){
    std::cout << "onLoad" << std::endl;
  }
  /* @desc uninitialize at unloading the source plug-in shared object such as .so */
  virtual void onUnload(void){
    std::cout << "onUnload" << std::endl;
  }
  /* @desc report the source plug-in's unique id
     @return unique plug-in id. may use uuid. */
  virtual std::string getId(void){
    return std::string("SourceExampleSin");
  }
  /* @desc this is expected to use by strategy
     @return new instance of this class's result */
  virtual std::shared_ptr<IPlugIn> newInstance(void){
    return std::make_shared<SourceExampleSin>();
  }
};


extern "C"
{
void* getPlugInInstance(void)
{
  SourceExampleSin* pInstance = new SourceExampleSin();
  return reinterpret_cast<void*>(dynamic_cast<IPlugIn*>(pInstance));
}
};
