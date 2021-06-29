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
#include "FilterExample.hpp"
#include "FifoBuffer.hpp"
#include "InterPipeBridge.hpp"
#include "PipeMultiThread.hpp"
#include "MultipleSink.hpp"
#include "Stream.hpp"
#include "StreamSink.hpp"
#include "StreamSource.hpp"
#include "PipeMixer.hpp"
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

#include <iostream>
#include <filesystem>
#include <chrono>
#include <memory>

class TestSink : public Sink
{
  int mTestLatency;
public:
  TestSink(int latencyUsec): Sink(), mTestLatency(latencyUsec){};
  virtual ~TestSink(){};
  virtual int getLatencyUSec(void){ return mTestLatency; };
};

class TestSource : public Source
{
  int mTestLatency;
public:
  TestSource(int latencyUsec): Source(), mTestLatency(latencyUsec){};
  virtual ~TestSource(){};
  virtual int getLatencyUSec(void){ return mTestLatency; };
};


class CompressedSource : public Source
{
protected:
  AudioFormat mFormat;
public:
  CompressedSource():Source(){};
  virtual ~CompressedSource(){};
  virtual void setAudioFormat(AudioFormat format){ mFormat=format; };
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

public:
  CompressedSink(AudioFormat::ENCODING encodingStartPoint = AudioFormat::ENCODING::COMPRESSED):Sink(){
    for(int anEncoding = encodingStartPoint; anEncoding < AudioFormat::ENCODING::COMPRESSED_UNKNOWN; anEncoding++){
      mAudioFormats.push_back( AudioFormat((AudioFormat::ENCODING)anEncoding) );
    }
  };
  virtual ~CompressedSink(){};
  virtual bool setAudioFormat(AudioFormat format){
    mFormat=format;
    Sink::setAudioFormat(format);
    return true;
  };
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
    ParameterManager* pParams = ParameterManager::getManager();
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
    ParameterManager* pParams = ParameterManager::getManager();
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
    ParameterManager* pParams = ParameterManager::getManager();
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
};

class SpdifSink : public CompressedSink
{
public:
  SpdifSink():CompressedSink(){};
  virtual ~SpdifSink(){};
};

class  LPcmSink : public Sink
{
public:
  LPcmSink():Sink(){};
  virtual ~LPcmSink(){};
};

class  SpeakerSink : public LPcmSink
{
public:
  SpeakerSink():LPcmSink(){};
  virtual ~SpeakerSink(){};
};

class  HeadphoneSink : public LPcmSink
{
public:
  HeadphoneSink():LPcmSink(){};
  virtual ~HeadphoneSink(){};
};

class  BluetoothAudioSink : public LPcmSink
{
public:
  BluetoothAudioSink():LPcmSink(){};
  virtual ~BluetoothAudioSink(){};
};

#endif /* __TESTCASE_COMMON_HPP__ */
