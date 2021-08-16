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
#include <memory>

#ifndef __AFW_TEST__
   #define __AFW_TEST__ 1
#endif /* __AFW_TEST__ */

class ITestable
{
public:
  virtual std::weak_ptr<ITestable> getTestShim(void) = 0;
};

class ICapture : public IUnlockable
{
protected:
  std::shared_ptr<FifoBufferReadReference> mpRefBuf;
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
  std::shared_ptr<FifoBuffer> mpInjectorBuf;
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
  std::shared_ptr<ISink> mpSink;
  virtual void setAudioFormatPrimitive(AudioFormat audioFormat);

public:
  SinkTestBase(std::shared_ptr<ISink> pSink);
  virtual ~SinkTestBase();

  virtual std::vector<PRESENTATION> getAvailablePresentations(void);
  virtual bool isAvailablePresentation(PRESENTATION presentation);

  virtual bool setPresentation(PRESENTATION presentation);
  virtual PRESENTATION getPresentation(void);

  virtual AudioFormat getAudioFormat(void);

  virtual float getVolume(void);
  virtual bool setVolume(float volumePercentage);

  virtual int getLatencyUSec(void);
  virtual int64_t getSinkPts(void);

  virtual void dump(void);
  virtual int stateResourceConsumption(void);
};

class SinkCapture : public SinkTestBase, public ICapture
{
protected:
  virtual void writePrimitive(IAudioBuffer& buf);
  virtual void setAudioFormatPrimitive(AudioFormat audioFormat);

public:
  SinkCapture(std::shared_ptr<ISink> pSink);
  virtual ~SinkCapture();
};

class SinkInjector : public SinkTestBase, public IInjector
{
protected:
  virtual void writePrimitive(IAudioBuffer& buf);
  virtual void setAudioFormatPrimitive(AudioFormat audioFormat);

public:
  SinkInjector(std::shared_ptr<ISink> pSink);
  virtual ~SinkInjector();
};

class SourceTestBase : public ISource
{
protected:
  std::shared_ptr<ISource> mpSource;

public:
  SourceTestBase(std::shared_ptr<ISource> pSource);
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
  SourceCapture(std::shared_ptr<ISource> pSource);
  virtual ~SourceCapture();
};

class SourceInjector : public SourceTestBase, public IInjector
{
protected:
  virtual void readPrimitive(IAudioBuffer& buf);

public:
  SourceInjector(std::shared_ptr<ISource> pSource);
  virtual ~SourceInjector();
};

class FilterTestBase : public Filter
{
protected:
  int mWindowSize;
  int mLatency;
  int mProcessingResource;

public:
  FilterTestBase(int windowSize = DEFAULT_WINDOW_SIZE_USEC, int latencyUsec = DEFAULT_WINDOW_SIZE_USEC, int processingResource = Filter::DEFAULT_REQUIRED_PROCESSING_RESOURCE) : mWindowSize(windowSize), mLatency(latencyUsec), mProcessingResource(processingResource){};
  virtual ~FilterTestBase(){};

  virtual int getRequiredWindowSizeUsec(void){ return mWindowSize; };
  virtual int getLatencyUSec(void){ return mLatency; };
  virtual int stateResourceConsumption(void){ return mProcessingResource; };
};

class FilterCapture : public FilterTestBase, public ICapture
{
public:
  FilterCapture(int windowSize = DEFAULT_WINDOW_SIZE_USEC, int latencyUsec = DEFAULT_WINDOW_SIZE_USEC, int processingResource = Filter::DEFAULT_REQUIRED_PROCESSING_RESOURCE) : FilterTestBase(windowSize, latencyUsec, processingResource), ICapture(){};
  virtual ~FilterCapture(){};
  virtual void process(AudioBuffer& inBuf, AudioBuffer& outBuf);
};

class FilterInjector : public FilterTestBase, public IInjector
{
public:
  FilterInjector(int windowSize = DEFAULT_WINDOW_SIZE_USEC, int latencyUsec = DEFAULT_WINDOW_SIZE_USEC, int processingResource = Filter::DEFAULT_REQUIRED_PROCESSING_RESOURCE) : FilterTestBase(windowSize, latencyUsec, processingResource), IInjector(){};
  virtual ~FilterInjector(){};
  virtual void process(AudioBuffer& inBuf, AudioBuffer& outBuf);
};



#endif /* __TESTABILITY_HPP__ */