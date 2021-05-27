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

#ifndef __TESTABILITY_HPP__
#define __TESTABILITY_HPP__

#include "Sink.hpp"
#include "Source.hpp"
#include "Filter.hpp"
#include "AudioFormat.hpp"
#include "PipeAndFilterCommon.hpp"
#include "FifoBufferReadReference.hpp"

class ICapture : public IUnlockable
{
protected:
  FifoBufferReadReference* mpRefBuf;
  void enqueToRefBuf(IAudioBuffer& buf);
  void setCaptureBufferSize(int nSamples);
public:
  ICapture( AudioFormat format = AudioFormat() );
  virtual ~ICapture();
  virtual void captureRead(IAudioBuffer& buf);

  virtual void setCaptureAudioFormat(AudioFormat audioFormat);
  virtual void unlock(void);
};

class IInjector : public IUnlockable
{
protected:
  FifoBuffer* mpInjectorBuf;
  bool mInjectorEnabled;

protected:
  void dequeFromInjectBuf(IAudioBuffer& buf);
  void setInjectBufferSize(int nSamples);

public:
  IInjector( AudioFormat format = AudioFormat() );
  virtual ~IInjector();

  virtual void setInjectorEnabled(bool bEnabled);
  virtual bool getInjectorEnabled(void);

  virtual void inject(IAudioBuffer& buf);
  virtual void setInjectAudioFormat(AudioFormat audioFormat);
  virtual void unlock(void);
};

class SinkTestBase : public ISink
{
protected:
  ISink* mpSink;

public:
  SinkTestBase(ISink* pSink);
  virtual ~SinkTestBase();

  virtual std::vector<PRESENTATION> getAvailablePresentations(void);
  virtual bool isAvailablePresentation(PRESENTATION presentation);

  virtual bool setPresentation(PRESENTATION presentation);
  virtual PRESENTATION getPresentation(void);

  virtual bool setAudioFormat(AudioFormat audioFormat);
  virtual AudioFormat getAudioFormat(void);

  virtual float getVolume(void);
  virtual bool setVolume(float volumePercentage);

  virtual int getLatencyUSec(void);
  virtual int64_t getSinkPts(void);

  virtual void dump(void);
};

class SinkCapture : public SinkTestBase, public ICapture
{
protected:
  virtual void writePrimitive(IAudioBuffer& buf);

public:
  SinkCapture(ISink* pSink);
  virtual ~SinkCapture();
  virtual bool setAudioFormat(AudioFormat audioFormat);
};

class SourceTestBase : public ISource
{
protected:
  ISource* mpSource;

public:
  SourceTestBase(ISource* pSource);
  virtual ~SourceTestBase();

  virtual int getLatencyUSec(void);
  virtual int64_t getSourcePts(void);
  virtual AudioFormat getAudioFormat(void);
};

class SourceCapture : public SourceTestBase, public ICapture
{
protected:
  virtual void readPrimitive(IAudioBuffer& buf);

public:
  SourceCapture(ISource* pSource);
  virtual ~SourceCapture();
};

class SourceInjector : public SourceTestBase, public IInjector
{
protected:
  virtual void readPrimitive(IAudioBuffer& buf);

public:
  SourceInjector(ISource* pSource);
  virtual ~SourceInjector();
};

class FilterTestBase : public IFilter
{
protected:
  int mWindowSize;
  int mLatency;
  int mProcessingTime;

public:
  FilterTestBase(int windowSize = DEFAULT_WINDOW_SIZE_USEC, int latencyUsec = DEFAULT_WINDOW_SIZE_USEC, int processingTimeUsec = Filter::DEFAULT_PROCESSING_TIME_USEC) : mWindowSize(windowSize), mLatency(latencyUsec), mProcessingTime(processingTimeUsec){};
  virtual ~FilterTestBase(){};

  virtual int getRequiredWindowSizeUsec(void){ return mWindowSize; };
  virtual int getLatencyUSec(void){ return mLatency; };
  virtual int getExpectedProcessingUSec(void){ return mProcessingTime; };
};

class FilterCapture : public FilterTestBase, public ICapture
{
public:
  FilterCapture(int windowSize = DEFAULT_WINDOW_SIZE_USEC, int latencyUsec = DEFAULT_WINDOW_SIZE_USEC, int processingTimeUsec = Filter::DEFAULT_PROCESSING_TIME_USEC) : ICapture(), FilterTestBase(windowSize, latencyUsec, processingTimeUsec){};
  virtual ~FilterCapture(){};
  virtual void process(AudioBuffer& inBuf, AudioBuffer& outBuf);
};

class FilterInjector : public FilterTestBase, public IInjector
{
public:
  FilterInjector(int windowSize = DEFAULT_WINDOW_SIZE_USEC, int latencyUsec = DEFAULT_WINDOW_SIZE_USEC, int processingTimeUsec = Filter::DEFAULT_PROCESSING_TIME_USEC) : IInjector(), FilterTestBase(windowSize, latencyUsec, processingTimeUsec){};
  virtual ~FilterInjector(){};
  virtual void process(AudioBuffer& inBuf, AudioBuffer& outBuf);
};



#endif /* __TESTABILITY_HPP__ */