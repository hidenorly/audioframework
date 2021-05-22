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

class ICapture
{
};

class SinkCapture : public ISink, public ICapture
{
protected:
  ISink* mpSink;
public:
  SinkCapture(ISink* pSink);
  virtual ~SinkCapture();

  virtual void write(IAudioBuffer& buf);

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


#endif /* __TESTABILITY_HPP__ */
