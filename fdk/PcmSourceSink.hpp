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
#include <cmath>

class PcmSource : public Source
{
protected:
  AudioFormat mFormat;

  virtual void setAudioFormatPrimitive(AudioFormat format){
    mFormat = format;
  };

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

public:
  PcmSource():Source(){};
  virtual ~PcmSource(){};

  virtual AudioFormat getAudioFormat(void){ return mFormat; };

  virtual bool setAudioFormat(AudioFormat format){
    setAudioFormatPrimitive(format);
    return true;
  };

  virtual bool isAvailableFormat(AudioFormat format){ return format.isEncodingPcm(); };

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

  virtual bool isAvailableFormat(AudioFormat format){ return format.isEncodingPcm(); };

  virtual AudioFormat getAudioFormat(void){ return mFormat; };

  std::vector<AudioFormat> getSupportedAudioFormats(void){
    std::vector<AudioFormat> formats;
    formats.push_back( mFormat );

    return formats;
  };

  virtual std::string toString(void){return "PcmSink";};
};

#endif /* __PCMSOURCESINK_HPP__ */
