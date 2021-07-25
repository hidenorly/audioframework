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

#ifndef __PCMSOURCESINK_HPP__
#define __PCMSOURCESINK_HPP__

#include "Source.hpp"
#include "Sink.hpp"

class PcmSource : public Source
{
protected:
  AudioFormat mFormat;

  virtual void setAudioFormatPrimitive(AudioFormat format){
    mFormat = format;
  };

  virtual void readPrimitive(IAudioBuffer& buf){
    AudioBuffer* pBuf = dynamic_cast<AudioBuffer*>(&buf);
    if( pBuf ){
      pBuf->setAudioFormat( mFormat );
      ByteBuffer rawBuf( pBuf->getRawBuffer().size(), 0 );
      buf.setRawBuffer( rawBuf );
      buf.setAudioFormat( mFormat );
    }
  }

public:
  PcmSource():Source(){};
  virtual ~PcmSource(){};

  virtual AudioFormat getAudioFormat(void){ return mFormat; };

  virtual bool setAudioFormat(AudioFormat format){
    setAudioFormatPrimitive(format);
    return true;
  };

  virtual std::vector<AudioFormat> getSupportedAudioFormats(void){
    std::vector<AudioFormat> formats;
    formats.push_back(mFormat);
    return formats;
  }

  virtual std::string toString(void){return "PcmSource";};
};


class PcmSink : public Sink
{
protected:
  AudioFormat mFormat;

protected:
  virtual void setAudioFormatPrimitive(AudioFormat format){
    mFormat = format;
    mpBuf->setAudioFormat(format);
  };

public:
  PcmSink(AudioFormat format = AudioFormat()):Sink(),mFormat(format){};
  virtual ~PcmSink(){};

  virtual bool setAudioFormat(AudioFormat format){
    setAudioFormatPrimitive(format);
    return true;
  };

  virtual AudioFormat getAudioFormat(void){ return mFormat; };

  std::vector<AudioFormat> getSupportedAudioFormats(void){
    std::vector<AudioFormat> formats;
    formats.push_back( mFormat );

    return formats;
  };

  virtual std::string toString(void){return "PcmSink";};
};

#endif /* __PCMSOURCESINK_HPP__ */
