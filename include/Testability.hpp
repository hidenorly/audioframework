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

class SinkCapture : public ISink, public ICapture
{
protected:
  ISink* mpSink;

protected:
  virtual void writePrimitive(IAudioBuffer& buf);

public:
  SinkCapture(ISink* pSink);
  virtual ~SinkCapture();

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


class SourceCapture : public ISource, public ICapture
{
protected:
  ISource* mpSource;

protected:
  virtual void readPrimitive(IAudioBuffer& buf);

public:
  SourceCapture(ISource* pSource);
  virtual ~SourceCapture();
  virtual int getLatencyUSec(void);
  virtual int64_t getSourcePts(void);
  virtual AudioFormat getAudioFormat(void);
};

class FilterCapture : public IFilter, public ICapture
{
protected:
  int mWindowSize;
  int mLatency;
  int mProcessingTime;
public:
  FilterCapture(int windowSize = DEFAULT_WINDOW_SIZE_USEC, int latencyUsec = DEFAULT_WINDOW_SIZE_USEC, int processingTimeUsec = Filter::DEFAULT_PROCESSING_TIME_USEC): ICapture(), mWindowSize(windowSize), mLatency(latencyUsec), mProcessingTime(processingTimeUsec){};
  virtual ~FilterCapture(){};
  virtual void process(AudioBuffer& inBuf, AudioBuffer& outBuf);
  virtual int getRequiredWindowSizeUsec(void){ return mWindowSize; };
  virtual int getLatencyUSec(void){ return mLatency; };
  virtual int getExpectedProcessingUSec(void){ return mProcessingTime; };
};

#endif /* __TESTABILITY_HPP__ */