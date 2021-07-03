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

#ifndef __ENCODEDSINK_HPP__
#define __ENCODEDSINK_HPP__

#include "Sink.hpp"
#include "Buffer.hpp"
#include "Encoder.hpp"
#include "Decoder.hpp"
#include <memory>

class EncodedSink : public ISink
{
protected:
  std::shared_ptr<ISink> mpSink;
  bool mbTranscode;
  std::shared_ptr<IMediaCodec> mpDecoder;
  std::shared_ptr<IMediaCodec> mpEncoder;

protected:
  virtual void ensureTranscoder(AudioFormat srcFormat, AudioFormat dstFormat);

public:
  EncodedSink(std::shared_ptr<ISink> pSink = nullptr, bool bTranscode = false);
  virtual ~EncodedSink();

  virtual std::shared_ptr<ISink> attachSink(std::shared_ptr<ISink> pSink);
  virtual std::shared_ptr<ISink> detachSink(void);
  virtual void clearSink();
  virtual void setTranscodeEnabled(bool bTranscode);
  virtual bool getTranscodeEnabled(void);

  virtual void writePrimitive(IAudioBuffer& buf);
  virtual std::string toString(void){
    std::string result = "EncodedSink";
    if( mpSink ){
      result = result + "(" + mpSink->toString() + ")";
    }
    return result;
  };

  virtual bool setAudioFormat(AudioFormat audioFormat);
  virtual AudioFormat getAudioFormat(void);
  virtual std::vector<AudioFormat> getSupportedAudioFormats(void);

  virtual float getVolume(void);
  virtual bool setVolume(float volumePercentage);
  virtual bool setVolume(Volume::CHANNEL_VOLUME perChannelVolumes);
  virtual bool setVolume(std::vector<float> perChannelVolumes);

  virtual std::vector<PRESENTATION> getAvailablePresentations(void);
  virtual bool isAvailablePresentation(PRESENTATION presentation);
  virtual bool setPresentation(PRESENTATION presentation);
  virtual PRESENTATION getPresentation(void);

  virtual int getLatencyUSec(void);
  virtual int64_t getSinkPts(void);

  virtual void dump(void);
  virtual int stateResourceConsumption(void);
};

#endif /* __ENCODEDSINK_HPP__ */