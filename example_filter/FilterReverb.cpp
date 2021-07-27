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

#include "Filter.hpp"
#include "ParameterManager.hpp"
#include "Buffer.hpp"
#include "AudioFormatAdaptor.hpp"
#include <string>
#include <iostream>
#include <algorithm>

class FilterExampleReverb;
class FilterExampleReverb : public FilterPlugIn
{
protected:
  int mWindowSize;
  int mCallbackId;
  float mDelay;
  float mPower;
  AudioBuffer mLastBuf;
  std::vector<AudioFormat> mSupportedFormats;

public:
  FilterExampleReverb(int windowSize = DEFAULT_WINDOW_SIZE_USEC) : mWindowSize(windowSize), mDelay(0.0f), mPower(0.5f){
    ParameterManager* pParams = ParameterManager::getManager();

    ParameterManager::CALLBACK callback = [&](std::string key, std::string value){
      if( key == "filter.exampleReverb.delay" ){
        std::cout << "[FilterExampleReverb] delay parameter is set to " << value << std::endl;
        mDelay = std::stof( value );
      } else if( key == "filter.exampleReverb.power" ){
        std::cout << "[FilterExampleReverb] power parameter is set to " << value << std::endl;
        mPower = std::stof( value );
      }
    };
    mCallbackId = pParams->registerCallback("filter.exampleReverb.*", callback);

    for(int anEncoding = AudioFormat::ENCODING::PCM_8BIT; anEncoding < AudioFormat::ENCODING::COMPRESSED_UNKNOWN; anEncoding++){
      for( int aChannel = AudioFormat::CHANNEL::CHANNEL_MONO; aChannel < AudioFormat::CHANNEL::CHANNEL_UNKNOWN; aChannel++){
        mSupportedFormats.push_back( AudioFormat((AudioFormat::ENCODING)anEncoding, 48000, (AudioFormat::CHANNEL)aChannel) );
        mSupportedFormats.push_back( AudioFormat((AudioFormat::ENCODING)anEncoding, 96000, (AudioFormat::CHANNEL)aChannel) );
      }
    }
  };
  virtual ~FilterExampleReverb(){
    ParameterManager* pParams = ParameterManager::getManager();
    pParams->unregisterCallback(mCallbackId);
    mCallbackId = 0;
  };
  virtual std::vector<AudioFormat> getSupportedAudioFormats(void){ return mSupportedFormats; }

  virtual void process16(AudioBuffer& inBuf, AudioBuffer& outBuf){
    int16_t* pRawInBuf = reinterpret_cast<int16_t*>( inBuf.getRawBufferPointer() );
    int16_t* pRawInPrevBuf = reinterpret_cast<int16_t*>( mLastBuf.getRawBufferPointer() );
    int16_t* pRawOutBuf = reinterpret_cast<int16_t*>( outBuf.getRawBufferPointer() );
    int nSamples = inBuf.getNumberOfSamples();
    int nChannels = inBuf.getAudioFormat().getNumberOfChannels();
    int nDelaySamples = (float)mDelay * 1000000.0f / (float)inBuf.getAudioFormat().getSamplingRate();
    for(int i=0; i<nChannels; i++ ){
      for(int j=0; j<nSamples; j++ ){
        int32_t tmp = *(pRawInBuf + nChannels * j + i);
        for(int k=0; k<nDelaySamples; k++){
          float ratio = (float)(nDelaySamples - k)/(float)(nDelaySamples) * mPower;
          if( j<nDelaySamples ){
            tmp += (int32_t)( (float)(*(pRawInPrevBuf + nChannels * ( nSamples + j - k ) + i ) * ratio ) );
          } else {
            tmp += (int32_t)( (float)(*(pRawInBuf + nChannels * ( j - k ) + i ) * ratio ) );
          }
        }
        *(pRawOutBuf + nChannels * j + i) = (int16_t)(std::max<int32_t>(INT16_MIN, std::min<int32_t>(tmp, INT16_MAX)));
      }
    }
  }

  virtual void process(AudioBuffer& inBuf, AudioBuffer& outBuf){
    if( inBuf.getAudioFormat().getEncoding() == AudioFormat::ENCODING::PCM_16BIT ){
      // invoke optimimul impl. directly
      mLastBuf = inBuf;
      process16(inBuf, outBuf);
    } else {
      AudioBuffer tmpInBuf(AudioFormat(AudioFormat::ENCODING::PCM_16BIT), inBuf.getNumberOfSamples() );
      AudioBuffer tmpOutBuf(AudioFormat(AudioFormat::ENCODING::PCM_16BIT), inBuf.getNumberOfSamples() );
      AudioFormatAdaptor::convert(inBuf, tmpInBuf);
      mLastBuf = tmpInBuf;
      process16(tmpInBuf, tmpOutBuf);
      AudioFormatAdaptor::convert(tmpOutBuf, outBuf);
    }
  };
  virtual int getRequiredWindowSizeUsec(void){ return mWindowSize; };
  virtual std::string toString(void){ return "FilterExampleReverb"; };


  /* @desc initialize at loading the filter plug-in shared object such as .so */
  virtual void onLoad(void){
    std::cout << "onLoad" << std::endl;
  }
  /* @desc uninitialize at unloading the filter plug-in shared object such as .so */
  virtual void onUnload(void){
    std::cout << "onUnload" << std::endl;
  }
  /* @desc report your filter plug-in's unique id
     @return unique plug-in id. may use uuid. */
  virtual std::string getId(void){
    return std::string("FilterExampleReverb");
  }
  /* @desc this is expected to use by strategy
     @return new YourFilter()'s result */
  virtual std::shared_ptr<IPlugIn> newInstance(void){
    return std::make_shared<FilterExampleReverb>();
  }
};


extern "C"
{
void* getPlugInInstance(void)
{
  FilterExampleReverb* pFilter = new FilterExampleReverb();
  return reinterpret_cast<void*>(dynamic_cast<IPlugIn*>(pFilter));
}
};
