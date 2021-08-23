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

#ifndef __TESTCASE_COMMON_HPP__
#define __TESTCASE_COMMON_HPP__

#include <gtest/gtest.h>

#include "Pipe.hpp"
#include "Filter.hpp"
#include "Source.hpp"
#include "Sink.hpp"
#include "AudioFormat.hpp"
#include "AudioFormatAdaptor.hpp"
#include "FilterExample.hpp"
#include "FifoBuffer.hpp"
#include "InterPipeBridge.hpp"
#include "PipeMultiThread.hpp"
#include "MultipleSink.hpp"
#include "Stream.hpp"
#include "StreamSink.hpp"
#include "StreamSource.hpp"
#include "PipeMixer.hpp"
#include "MixerSplitter.hpp"
#include "PatchPanel.hpp"
#include "PipedSink.hpp"
#include "PipedSource.hpp"
#include "Media.hpp"
#include "Decoder.hpp"
#include "Encoder.hpp"
#include "EncodedSink.hpp"
#include "Player.hpp"
#include "ParameterManager.hpp"
#include "StringTokenizer.hpp"
#include "PlugInManager.hpp"
#include "DelayFilter.hpp"
#include "Testability.hpp"
#include "Util.hpp"
#include "ResourceManager.hpp"
#include "Strategy.hpp"
#include "StreamManager.hpp"
#include "PowerManager.hpp"
#include "PowerManagerPrimitive.hpp"
#include "AccousticEchoCancelledSource.hpp"
#include "ReferenceSoundSinkSource.hpp"
#include "ChannelDemultiplexer.hpp"
#include "ChannelMultiplexer.hpp"

#include <iostream>
#include <filesystem>
#include <chrono>
#include <memory>
#include <cmath>

class TestSink : public Sink
{
  int mTestLatency;
public:
  TestSink(int latencyUsec=0): Sink(), mTestLatency(latencyUsec){};
  virtual ~TestSink(){};
  virtual int getLatencyUSec(void){ return mTestLatency; };
  virtual std::vector<AudioFormat> getSupportedAudioFormats(void){
    std::vector<AudioFormat> formats;
    formats.push_back( AudioFormat(AudioFormat::ENCODING::PCM_8BIT) );
    formats.push_back( AudioFormat(AudioFormat::ENCODING::PCM_16BIT) );
    formats.push_back( AudioFormat(AudioFormat::ENCODING::PCM_24BIT_PACKED) );
    formats.push_back( AudioFormat(AudioFormat::ENCODING::PCM_32BIT) );
    formats.push_back( AudioFormat(AudioFormat::ENCODING::PCM_FLOAT) );
    return formats;
  }
};

class TestSource : public Source
{
  int mTestLatency;
public:
  TestSource(int latencyUsec=0): Source(), mTestLatency(latencyUsec){};
  virtual ~TestSource(){};
  virtual int getLatencyUSec(void){ return mTestLatency; };
  virtual std::vector<AudioFormat> getSupportedAudioFormats(void){
    std::vector<AudioFormat> formats;
    formats.push_back( AudioFormat(AudioFormat::ENCODING::PCM_8BIT) );
    formats.push_back( AudioFormat(AudioFormat::ENCODING::PCM_16BIT) );
    formats.push_back( AudioFormat(AudioFormat::ENCODING::PCM_24BIT_PACKED) );
    formats.push_back( AudioFormat(AudioFormat::ENCODING::PCM_32BIT) );
    formats.push_back( AudioFormat(AudioFormat::ENCODING::PCM_FLOAT) );
    return formats;
  }
};

class SinSource : public TestSource
{
public:
  SinSource(){};
  virtual ~SinSource(){};
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
};

class CompressedSource : public Source
{
protected:
  AudioFormat mFormat;
  virtual void setAudioFormatPrimitive(AudioFormat format){ mFormat=format; };
public:
  CompressedSource():Source(),mFormat(AudioFormat::ENCODING::COMPRESSED){};
  virtual ~CompressedSource(){};
  virtual AudioFormat getAudioFormat(void){ return mFormat; };
  virtual void readPrimitive(IAudioBuffer& buf){
    ByteBuffer esRawBuf( 256, 0 );
    buf.setRawBuffer( esRawBuf );
    buf.setAudioFormat( mFormat );
  }
  virtual std::string toString(void){return "CompressedSource";};
};

class CompressedSink : public Sink
{
protected:
  std::vector<AudioFormat> mAudioFormats;
  AudioFormat mFormat;

protected:
  virtual void setAudioFormatPrimitive(AudioFormat format){mFormat=format;};

public:
  CompressedSink(AudioFormat::ENCODING encodingStartPoint = AudioFormat::ENCODING::COMPRESSED):Sink(),mFormat(AudioFormat::ENCODING::ENCODING_DEFAULT){
    for(int anEncoding = encodingStartPoint; anEncoding < AudioFormat::ENCODING::COMPRESSED_UNKNOWN; anEncoding++){
      mAudioFormats.push_back( AudioFormat((AudioFormat::ENCODING)anEncoding) );
    }
  };
  virtual ~CompressedSink(){};
  virtual AudioFormat getAudioFormat(void){ return mFormat; };
  std::vector<AudioFormat> getSupportedAudioFormats(void){ return mAudioFormats; };
  virtual std::string toString(void){return "CompressedSink";};
};


class VirtualizerA : public Filter
{
public:
  static inline std::string applyConditionKey = "virtualizer.virtualizerA.applyCondition";
  static void ensureDefaultAssumption(void){
    std::vector<AudioFormat::ENCODING> encodings = {
      AudioFormat::ENCODING::COMPRESSED_AC3,
      AudioFormat::ENCODING::COMPRESSED_E_AC3,
      AudioFormat::ENCODING::COMPRESSED_AC4,
      AudioFormat::ENCODING::COMPRESSED_DOLBY_TRUEHD,
      AudioFormat::ENCODING::COMPRESSED_MAT,
    };
    std::string applyCondition;
    for(auto& anEncoding : encodings ){
      applyCondition = applyCondition + std::to_string((int)anEncoding) + ",";
    }
    std::shared_ptr<ParameterManager> pParams = ParameterManager::getManager().lock();
    pParams->setParameter(applyConditionKey, applyCondition);
  };
  VirtualizerA():Filter(){
    ensureDefaultAssumption();
  }
  virtual ~VirtualizerA(){};
  virtual void process(AudioBuffer& inBuf, AudioBuffer& outBuf)
  {
    std::cout << "virtualizer A" << std::endl;
    ByteBuffer buf( inBuf.getRawBuffer().size(), 'A');
    outBuf.setRawBuffer(buf);
  }
  virtual std::string toString(void){ return "VirtualizerA"; };
};

class VirtualizerB : public Filter
{
public:
  static inline std::string applyConditionKey = "virtualizer.virtualizerB.applyCondition";
  static void ensureDefaultAssumption(void){
    std::vector<AudioFormat::ENCODING> encodings = {
      AudioFormat::ENCODING::COMPRESSED_DTS,
      AudioFormat::ENCODING::COMPRESSED_DTS_HD,
    };
    std::string applyCondition;
    for(auto& anEncoding : encodings ){
      applyCondition = applyCondition + std::to_string((int)anEncoding) + ",";
    }
    std::shared_ptr<ParameterManager> pParams = ParameterManager::getManager().lock();
    pParams->setParameter(applyConditionKey, applyCondition);
  };
  VirtualizerB():Filter(){
    ensureDefaultAssumption();
  };
  virtual ~VirtualizerB(){};
  virtual void process(AudioBuffer& inBuf, AudioBuffer& outBuf)
  {
    std::cout << "virtualizer B" << std::endl;
    ByteBuffer buf( inBuf.getRawBuffer().size(), 'B');
    outBuf.setRawBuffer(buf);
  }
  virtual std::string toString(void){ return "VirtualizerB"; };
};

class VirtualizerC : public Filter
{
public:
  static inline std::string applyConditionKey = "virtualizer.virtualizerC.applyCondition";
  static void ensureDefaultAssumption(void){
    std::shared_ptr<ParameterManager> pParams = ParameterManager::getManager().lock();
    pParams->setParameter(applyConditionKey, "*");
  };
  VirtualizerC():Filter(){
    ensureDefaultAssumption();
  };
  virtual ~VirtualizerC(){};
  virtual void process(AudioBuffer& inBuf, AudioBuffer& outBuf)
  {
    std::cout << "virtualizer C" << std::endl;
    ByteBuffer buf( inBuf.getRawBuffer().size(), 'C');
    outBuf.setRawBuffer(buf);
  }
  virtual std::string toString(void){ return "VirtualizerC"; };
};

class SpeakerProtectionFilter : public Filter
{
public:
  SpeakerProtectionFilter(){};
  virtual ~SpeakerProtectionFilter(){};
  virtual void process(AudioBuffer& inBuf, AudioBuffer& outBuf){ outBuf = inBuf; };
};

// examples
class HdmiAudioSink : public CompressedSink
{
public:
  HdmiAudioSink():CompressedSink(){};
  virtual ~HdmiAudioSink(){};
  virtual std::string toString(void){ return "HdmiAudioSink";};
};

class SpdifSink : public CompressedSink
{
public:
  SpdifSink():CompressedSink(){};
  virtual ~SpdifSink(){};
  virtual std::string toString(void){ return "SpdifSink";};
};

class LPcmSink : public Sink
{
public:
  LPcmSink():Sink(){};
  virtual ~LPcmSink(){};
  virtual std::string toString(void){ return "LPcmSink";};
};

class SpeakerSink : public LPcmSink
{
public:
  SpeakerSink():LPcmSink(){};
  virtual ~SpeakerSink(){};
  virtual std::string toString(void){ return "SpeakerSink";};
};

class HeadphoneSink : public LPcmSink
{
public:
  HeadphoneSink():LPcmSink(){};
  virtual ~HeadphoneSink(){};
  virtual std::string toString(void){ return "HeadphoneSink";};
};

class BluetoothAudioSink : public LPcmSink
{
public:
  BluetoothAudioSink():LPcmSink(){};
  virtual ~BluetoothAudioSink(){};
  virtual std::string toString(void){ return "BluetoothAudioSink";};
};

class HQSpeakerSink : public SpeakerSink
{
protected:
    std::vector<AudioFormat> mSupportedFormats;
    AudioFormat mOutputFormat;
public:
  HQSpeakerSink(bool bFloat = false, AudioFormat outputFormat = AudioFormat(AudioFormat::ENCODING::PCM_32BIT)):SpeakerSink(), mOutputFormat(outputFormat){
    if( bFloat ){
      mSupportedFormats.push_back( AudioFormat(AudioFormat::ENCODING::PCM_FLOAT) );
    } else {
      mSupportedFormats.push_back( AudioFormat(AudioFormat::ENCODING::PCM_32BIT) );
    }
  };
  virtual ~HQSpeakerSink(){};
  virtual std::string toString(void){ return "HQSpeakerSink";};
  virtual std::vector<AudioFormat> getSupportedAudioFormats(void){
    return mSupportedFormats;
  }
protected:
  virtual void writePrimitive(IAudioBuffer& buf){
    AudioBuffer* pSrcBuf = dynamic_cast<AudioBuffer*>(&buf);
    if( pSrcBuf ){
      if( !pSrcBuf->getAudioFormat().equal(mOutputFormat) ){
        mpBuf.reset();
        mpBuf = std::make_shared<AudioBuffer>( mOutputFormat, 0 );
      }
      AudioBuffer outBuf( mOutputFormat, pSrcBuf->getNumberOfSamples() );
      AudioFormatAdaptor::convert( *pSrcBuf, outBuf );
      Sink::writePrimitive( outBuf );
    } else {
      SpeakerSink::writePrimitive( buf );
    }
  }
};

class SinkFactory
{
public:
  static std::map<std::string, std::shared_ptr<ISink>> getSinks(void)
  {
    std::map<std::string, std::shared_ptr<ISink>> sinkManager;
    sinkManager.insert_or_assign("speaker",   std::make_shared<PipedSink>( std::make_shared<SpeakerSink>() ));
    sinkManager.insert_or_assign("headphone", std::make_shared<HeadphoneSink>());
    sinkManager.insert_or_assign("hdmi",      std::make_shared<EncodedSink>( std::make_shared<HdmiAudioSink>(), /* trasncoder */ true ));
    sinkManager.insert_or_assign("spdif",     std::make_shared<EncodedSink>( std::make_shared<SpdifSink>(), /* trasncoder */ true ));
    sinkManager.insert_or_assign("bluetooth", std::make_shared<BluetoothAudioSink>());

    return sinkManager;
  }
};

class TunnelPlaybackContext : public StrategyContext
{
public:
  std::shared_ptr<IPipe> pPipe;
  std::shared_ptr<ISource> pSource;
  std::shared_ptr<ISink> pSink;
  std::shared_ptr<IPlayer> pPlayer;

public:
  TunnelPlaybackContext():StrategyContext(){};
  TunnelPlaybackContext(std::shared_ptr<IPipe> pPipe, std::shared_ptr<ISource> pSource, std::shared_ptr<ISink> pSink):StrategyContext(), pPipe(pPipe), pSource(pSource), pSink(pSink){};
  virtual ~TunnelPlaybackContext(){
    pPipe.reset();
    pSource.reset();
    pSink.reset();
    pPlayer.reset();
  };
};

class TunnelPlaybackStrategy : public IStrategy
{
protected:
  std::shared_ptr<IMediaCodec> getCodec(AudioFormat format){
    return IMediaCodec::createByFormat(format, true);
  }
  bool shouldHandleFormat(AudioFormat format, std::string encodings)
  {
    std::cout << "shouldHandleFormat(" << (int)format.getEncoding() << " , " << encodings << " )" << std::endl;
    // TODO: should expand not only encodings but also the other conditions such as channels
    StringTokenizer tok(encodings, ",");
    AudioFormat::ENCODING theEncoding = format.getEncoding();
    while( tok.hasNext() ){
      if( (int)theEncoding == std::stoi(tok.getNext()) ){
        return true;
      }
    }
    return false;
  }
  std::shared_ptr<IFilter> getVirtualizer(AudioFormat format, std::shared_ptr<StrategyContext> context){
    std::shared_ptr<ParameterManager> pParams = ParameterManager::getManager().lock();
    if( shouldHandleFormat(format, pParams->getParameter(VirtualizerA::applyConditionKey) ) ){
      std::cout << "Create instance of Virtualizer A" << std::endl;
      return std::make_shared<VirtualizerA>();
    } else if( shouldHandleFormat(format, pParams->getParameter(VirtualizerB::applyConditionKey) ) ){
      std::cout << "Create instance of Virtualizer B" << std::endl;
      return std::make_shared<VirtualizerB>();
    }
    return nullptr;
  }
  std::vector<std::shared_ptr<IFilter>> getFilters(AudioFormat format, std::shared_ptr<StrategyContext> context){
    std::vector<std::shared_ptr<IFilter>> filters;
    std::shared_ptr<IFilter> pFilter = getVirtualizer(format, context);
    if( pFilter ){
      filters.push_back( pFilter );
    }

    return filters;
  }
public:
  TunnelPlaybackStrategy():IStrategy(){};
  virtual ~TunnelPlaybackStrategy(){};
  virtual bool canHandle(std::shared_ptr<StrategyContext> context){
    return true;
  }
  virtual bool execute(std::shared_ptr<StrategyContext> pContext){
    std::shared_ptr<TunnelPlaybackContext> context = std::dynamic_pointer_cast<TunnelPlaybackContext>(pContext);
    if( context ){
      std::shared_ptr<ParameterManager> pParams = ParameterManager::getManager().lock();
      // ensure pipe
      if( !context->pPipe ){
        std::cout << "create Pipe instance" << std::endl;
        context->pPipe = std::make_shared<Pipe>();
      } else {
        context->pPipe->stop();
        context->pPipe->clearFilters();
        context->pPlayer.reset();
      }
      // setup encoder sink if necessary
      bool bTranscoder = pParams->getParameterBool("sink.transcoder");
      if( bTranscoder && context->pSink && !std::dynamic_pointer_cast<EncodedSink>(context->pSink) && !std::dynamic_pointer_cast<InterPipeBridge>(context->pSink) ){
        std::cout << "create EncodedSink instance" << std::endl;
        context->pSink = std::make_shared<EncodedSink>(context->pSink, bTranscoder);
      }
      // setup source : player if necessary
      // should be sink.passthrough = false if SpeakerSink, HeadphoneSink and BluetoothAudioSink
      if( context->pSource ){
        AudioFormat srcFormat = context->pSource->getAudioFormat();
        // Attach source (=Player or the Source as-is) to the Pipe's Source
        if( srcFormat.isEncodingCompressed() && !pParams->getParameterBool("sink.passthrough") ){
          std::cout << "create Player instance" << std::endl;
          context->pPlayer = std::make_shared<Player>();
          context->pPipe->attachSource(
            context->pPlayer->prepare(
              context->pSource,
              getCodec( context->pSource->getAudioFormat() )
            )
          );
        } else {
          context->pPipe->attachSource( context->pSource );
        }
        // setup filter if PCM is input to the pipe
        if( srcFormat.isEncodingPcm() || context->pPlayer ){
          std::cout << "create Filter instance" << std::endl;
          std::vector<std::shared_ptr<IFilter>> pFilters = getFilters(srcFormat, context);
          for( auto& aFilter : pFilters ){
            context->pPipe->addFilterToTail( aFilter );
          }
        }
      }

      // setup sink
      context->pPipe->attachSink(context->pSink);

      return context->pPipe != nullptr;
    } else {
      return false;
    }
  }
};


class OutputManager : public MultipleSink
{
  std::map<std::string, std::shared_ptr<ISink>> mSinks;
  std::string mPrimaryOutput;
  std::string mConcurrentOutput;
public:
  OutputManager(std::map<std::string, std::shared_ptr<ISink>> sinks, std::string primayOutput, std::string concurrentOutput = ""){
    mSinks = sinks;
    setPrimaryOutput(primayOutput, concurrentOutput);
  };
  virtual ~OutputManager(){};
  void setPrimaryOutput(std::string primaryOutput, std::string concurrentOutput = ""){
    if( concurrentOutput == "" ){
      concurrentOutput = mConcurrentOutput;
    }
    std::cout << "setPrimaryOutput(" << primaryOutput << ") , concurrent:(" << concurrentOutput << ")" << std::endl;
    bool isPrimaryChanged = false;
    if( ( mPrimaryOutput != primaryOutput ) && mSinks.contains(primaryOutput) && mSinks[primaryOutput] ){
      detachSink( mSinks[mPrimaryOutput] );
      attachSink( mSinks[primaryOutput], mSinks[primaryOutput]->getAudioFormat().getSameChannelMapper() );
      mPrimaryOutput = primaryOutput;
      isPrimaryChanged = true;
    }
    if( ( ( mConcurrentOutput != concurrentOutput ) || isPrimaryChanged ) && mSinks.contains(concurrentOutput) && mSinks[concurrentOutput]){
      detachSink( mSinks[mConcurrentOutput] );
      attachSink( mSinks[concurrentOutput], mSinks[concurrentOutput]->getAudioFormat().getSameChannelMapper() );
      mConcurrentOutput = concurrentOutput;
    }
  };
};


class FilterReverb : public Filter
{
protected:
  int mWindowSize;
  int mCallbackId;
  float mDelay;
  float mPower;
  AudioBuffer mLastBuf;

public:
  FilterReverb(int windowSize = DEFAULT_WINDOW_SIZE_USEC) : mWindowSize(windowSize), mDelay(0.0f), mPower(0.5f){
    std::shared_ptr<ParameterManager> pParams = ParameterManager::getManager().lock();

    ParameterManager::CALLBACK callback = [&](std::string key, std::string value){
      if( key == "filter.exampleReverb.delay" ){
        std::cout << "[FilterReverb] delay parameter is set to " << value << std::endl;
        mDelay = std::stof( value );
      } else if( key == "filter.exampleReverb.power" ){
        std::cout << "[FilterReverb] power parameter is set to " << value << std::endl;
        mPower = std::stof( value );
      }
    };
    mCallbackId = pParams->registerCallback("filter.exampleReverb.*", callback);
  };
  virtual ~FilterReverb(){
    std::shared_ptr<ParameterManager> pParams = ParameterManager::getManager().lock();
    pParams->unregisterCallback(mCallbackId);
    mCallbackId = 0;
  };
  virtual std::vector<AudioFormat> getSupportedAudioFormats(void){
    std::vector<AudioFormat> formats;
    formats.push_back( AudioFormat(AudioFormat::ENCODING::PCM_8BIT) );
    formats.push_back( AudioFormat(AudioFormat::ENCODING::PCM_16BIT) );
    formats.push_back( AudioFormat(AudioFormat::ENCODING::PCM_24BIT_PACKED) );
    formats.push_back( AudioFormat(AudioFormat::ENCODING::PCM_32BIT) );
    formats.push_back( AudioFormat(AudioFormat::ENCODING::PCM_FLOAT) );
    return formats;
  }

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
  virtual std::string toString(void){ return "FilterReverb"; };
};

class IPerformanceMeasurement
{
protected:
  long mPerfCount;
  std::chrono::high_resolution_clock::time_point mStartTime;
  std::chrono::high_resolution_clock::time_point mLastTime;
public:
  virtual void reset(void){
    mPerfCount = 0;
    mStartTime = std::chrono::high_resolution_clock::now();
    mLastTime = mStartTime;
  }
  IPerformanceMeasurement(){
    reset();
  };
  virtual ~IPerformanceMeasurement(){};
  virtual void update(long count){
    mPerfCount = count;
    mLastTime = std::chrono::high_resolution_clock::now();
  }
  virtual void updateAdd(long count){
    mPerfCount += count;
    mLastTime = std::chrono::high_resolution_clock::now();
  }
  virtual std::chrono::nanoseconds getElapsedTime(void){
     return std::chrono::duration_cast<std::chrono::nanoseconds>(mLastTime - mStartTime);
  }
  virtual std::chrono::high_resolution_clock::time_point getStartTime(void){
     return mStartTime;
  }
  virtual std::chrono::high_resolution_clock::time_point getLastTime(void){
     return mLastTime;
  }
  virtual long getCount(void){
    return mPerfCount;
  }
  virtual double getCountPerTime(void){
    return (double)mPerfCount * 1000000000 / (double)getElapsedTime().count();
  }
};

class WindowSizeVariableFilter : public Filter
{
protected:
  int mWindowSize;
public:
  WindowSizeVariableFilter():Filter(), mWindowSize(5000){};
  virtual ~WindowSizeVariableFilter(){};
  virtual int getRequiredWindowSizeUsec(void){ return mWindowSize; };
  virtual void setWindowSizeUsec(int uSec){ mWindowSize = uSec; };
};

#endif /* __TESTCASE_COMMON_HPP__ */
