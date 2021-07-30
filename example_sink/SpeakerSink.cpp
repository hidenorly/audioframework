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

#include "Sink.hpp"
#include "PipedSink.hpp"
#include "Buffer.hpp"
#include "AudioFormatAdaptor.hpp"
#include <cmath>
#include <iostream>

class ExampleSpeakerSink : public Sink
{
protected:
  std::vector<AudioFormat> mSupportedFormats;

public:
  ExampleSpeakerSink(AudioFormat outputFormat = AudioFormat()):Sink(){
    for(int anEncoding = AudioFormat::ENCODING::PCM_8BIT; anEncoding < AudioFormat::ENCODING::COMPRESSED_UNKNOWN; anEncoding++){
      for( int aChannel = AudioFormat::CHANNEL::CHANNEL_MONO; aChannel < AudioFormat::CHANNEL::CHANNEL_UNKNOWN; aChannel++){
        mSupportedFormats.push_back( AudioFormat((AudioFormat::ENCODING)anEncoding, 48000, (AudioFormat::CHANNEL)aChannel) );
        mSupportedFormats.push_back( AudioFormat((AudioFormat::ENCODING)anEncoding, 96000, (AudioFormat::CHANNEL)aChannel) );
      }
    }
    mpBuf->setAudioFormat( outputFormat );
  };
  virtual ~ExampleSpeakerSink(){};
  virtual std::string toString(void){ return "ExampleSpeakerSink";};
  virtual std::vector<AudioFormat> getSupportedAudioFormats(void){ return mSupportedFormats; }
  virtual AudioFormat getAudioFormat(void){ return mpBuf->getAudioFormat(); };
  virtual bool isAvailableFormat(AudioFormat format){ return format.isEncodingPcm(); };

protected:
  virtual void setAudioFormatPrimitive(AudioFormat format){
    if( mpBuf ){
      mpBuf->setAudioFormat( format );
    }
  };
  virtual void writePrimitive(IAudioBuffer& buf){
    AudioBuffer* pSrcBuf = dynamic_cast<AudioBuffer*>(&buf);
    if( pSrcBuf ){
      AudioFormat bufFormat = mpBuf->getAudioFormat();
      if( !pSrcBuf->getAudioFormat().equal(bufFormat) ){
        mpBuf.reset();
        mpBuf = std::make_shared<AudioBuffer>( bufFormat, 0 );
        AudioBuffer outBuf( bufFormat, pSrcBuf->getNumberOfSamples() );
        AudioFormatAdaptor::convert( *pSrcBuf, outBuf );
        Sink::writePrimitive( outBuf );
      } else {
        Sink::writePrimitive( buf );
      }
    } else {
      // non Pcm Audio Buffer
      Sink::writePrimitive( buf );
    }
  }
};

class NullFilter : public Filter
{
protected:
  std::vector<AudioFormat> mSupportedFormats;
public:
  NullFilter(){
    for(int anEncoding = AudioFormat::ENCODING::PCM_8BIT; anEncoding < AudioFormat::ENCODING::COMPRESSED_UNKNOWN; anEncoding++){
      for( int aChannel = AudioFormat::CHANNEL::CHANNEL_MONO; aChannel < AudioFormat::CHANNEL::CHANNEL_UNKNOWN; aChannel++){
        mSupportedFormats.push_back( AudioFormat((AudioFormat::ENCODING)anEncoding, 48000, (AudioFormat::CHANNEL)aChannel) );
        mSupportedFormats.push_back( AudioFormat((AudioFormat::ENCODING)anEncoding, 96000, (AudioFormat::CHANNEL)aChannel) );
      }
    }
  };
  virtual ~NullFilter(){};
  virtual std::vector<AudioFormat> getSupportedAudioFormats(void){ return mSupportedFormats; };
  virtual void process(AudioBuffer& inBuf, AudioBuffer& outBuf){ outBuf = inBuf; };
  virtual std::string toString(void){ return "NullFilter"; };
};

class ExamplePipedSpeakerSink;
class ExamplePipedSpeakerSink : public SinkPlugIn
{
protected:
  std::shared_ptr<PipedSink> mPipedSink;

  void terminate(void){
    if( mPipedSink ){
      mPipedSink->stop();
      mPipedSink->clearFilters();
      mPipedSink->detachSink();
    }
    mPipedSink.reset();
  }

  virtual void writePrimitive(IAudioBuffer& buf){
    if( mPipedSink ){
      mPipedSink->run();
      mPipedSink->writePrimitive(buf);
    }
  }

public:
  ExamplePipedSpeakerSink(){
    mPipedSink = std::make_shared<PipedSink>( );
    attachSink( std::make_shared<ExampleSpeakerSink>() );
    addFilterToTail( std::make_shared<NullFilter>() );
  };
  virtual ~ExamplePipedSpeakerSink(){
    terminate();
  };

  virtual void setAudioFormatPrimitive(AudioFormat format){
    if( mPipedSink ){
      mPipedSink->setAudioFormat(format);
    } else {
      SinkPlugIn::setAudioFormatPrimitive(format);
    }
  };

  virtual AudioFormat getAudioFormat(void){
    AudioFormat result;
    if( mPipedSink ){
      result = mPipedSink->getAudioFormat();
    }
    return result;
  }
  virtual std::vector<AudioFormat> getSupportedAudioFormats(void){
    std::vector<AudioFormat> result;
    if( mPipedSink ){
      result = mPipedSink->getSupportedAudioFormats();
    }
    return result;
  }

  virtual std::shared_ptr<ISink> attachSink(std::shared_ptr<ISink> pSink){
    std::shared_ptr<ISink> result;
    if( mPipedSink ){
      result = mPipedSink->attachSink( pSink );
    }
    return result;
  }
  virtual std::shared_ptr<ISink> detachSink(void){
    std::shared_ptr<ISink> result;
    if( mPipedSink ){
      result = mPipedSink->detachSink();
    }
    return result;
  }

  virtual void addFilterToHead(std::shared_ptr<IFilter> pFilter){
    if( mPipedSink ){
      mPipedSink->addFilterToHead( pFilter );
    }
  }
  virtual void addFilterToTail(std::shared_ptr<IFilter> pFilter){
    if( mPipedSink ){
      mPipedSink->addFilterToTail( pFilter );
    }
  }
  virtual int stateResourceConsumption(void){
    int result = 0;
    if( mPipedSink ){
      result = mPipedSink->stateResourceConsumption();
    }
    return result;
  }

  virtual std::vector<ISink::PRESENTATION> getAvailablePresentations(void){
    std::vector<ISink::PRESENTATION> result({ISink::PRESENTATION::PRESENTATION_DEFAULT});
    if( mPipedSink ){
      result = mPipedSink->getAvailablePresentations();
    }
    return result;
  }
  virtual bool isAvailablePresentation(ISink::PRESENTATION presentation){
    bool result = false;
    if( mPipedSink ){
      result = mPipedSink->isAvailablePresentation(presentation);
    }
    return result;
  }
  virtual bool setPresentation(ISink::PRESENTATION presentation){
    bool result = false;
    if( mPipedSink ){
      result = mPipedSink->setPresentation(presentation);
    }
    return result;
  }
  virtual ISink::PRESENTATION getPresentation(void){
    ISink::PRESENTATION result = ISink::PRESENTATION::PRESENTATION_DEFAULT;
    if( mPipedSink ){
      result = mPipedSink->getPresentation();
    }
    return result;
  }

  virtual int getLatencyUSec(void){
    int result = 0;
    if( mPipedSink ){
      result = mPipedSink->getLatencyUSec();
    }
    return result;
  }
  virtual int64_t getSinkPts(void){
    int64_t result = 0;
    if( mPipedSink ){
      result = mPipedSink->getSinkPts();
    }
    return result;
  }

  virtual void dump(void){
    if( mPipedSink ){
      mPipedSink->dump();
    }
  }

  virtual std::string toString(void){ return "ExamplePipedSpeakerSink"; };

  /* @desc initialize at loading the filter plug-in shared object such as .so */
  virtual void onLoad(void){
    std::cout << "onLoad" << std::endl;
  }
  /* @desc uninitialize at unloading the filter plug-in shared object such as .so */
  virtual void onUnload(void){
    std::cout << "onUnload" << std::endl;
    terminate();
  }
  /* @desc report your filter plug-in's unique id
     @return unique plug-in id. may use uuid. */
  virtual std::string getId(void){
    return std::string("ExamplePipedSpeakerSink");
  }
  /* @desc this is expected to use by strategy
     @return new YourFilter()'s result */
  virtual std::shared_ptr<IPlugIn> newInstance(void){
    return std::make_shared<ExamplePipedSpeakerSink>();
  }
};


extern "C"
{
void* getPlugInInstance(void)
{
  ExamplePipedSpeakerSink* pInstance = new ExamplePipedSpeakerSink();
  return reinterpret_cast<void*>(dynamic_cast<IPlugIn*>(pInstance));
}
};
