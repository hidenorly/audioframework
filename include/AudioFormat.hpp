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

#ifndef __AUDIO_FORMAT_HPP__
#define __AUDIO_FORMAT_HPP__

#include <string>
#include <vector>
#include <map>

class AudioFormat
{
public:
  enum ENCODING {
    PCM_8BIT,
    PCM_16BIT,
    PCM_24BIT_PACKED,
    PCM_32BIT,
    PCM_FLOAT,
    PCM_UNKNOWN,
    COMPRESSED = PCM_UNKNOWN,
    ENCODING_DEFAULT = PCM_16BIT,

    COMPRESSED_MP3 = COMPRESSED,
    COMPRESSED_MP2,
    COMPRESSED_AAC,
    COMPRESSED_AAC_LATM,
    COMPRESSED_AAC_ADTS,
    COMPRESSED_AAC_ADIF,
    COMPRESSED_HE_AAC_V1,
    COMPRESSED_HE_AAC_V2,
    COMPRESSED_ALAC,
    COMPRESSED_FLAC,
    COMPRESSED_DSD,
    COMPRESSED_VORBIS,
    COMPRESSED_OPUS,
    COMPRESSED_CELT,

    COMPRESSED_AC3,
    COMPRESSED_E_AC3,
    COMPRESSED_AC4,
    COMPRESSED_DOLBY_TRUEHD,
    COMPRESSED_MAT,

    COMPRESSED_DTS,
    COMPRESSED_DTS_HD,

    COMPRESSED_WMA,
    COMPRESSED_WMA_PRO,

    COMPRESSED_SBC,
    COMPRESSED_APTX,
    COMPRESSED_APTX_HD,
    COMPRESSED_LDAC,
    COMPRESSED_LHDC,
    COMPRESSED_LHDC_LL,
    COMPRESSED_LC3,
  };

  enum CH {
    L,
    FL=L,
    MONO=L,
    R,
    FR=R,
    C,
    SL,
    SR,
    SW,
    SBL,
    SBR
  };

  enum CHANNEL {
    CHANNEL_MONO,
    CHANNEL_STEREO,
    CHANNEL_4CH,
    CHANNEL_5CH,
    CHANNEL_5_1CH,
    CHANNEL_5_1_2CH,
    CHANNEL_5_0_2CH,
    CHANNEL_7_1CH,
    CHANNEL_UNKNOWN,
    CHANNEL_DEFAULT = CHANNEL_STEREO
  };

  enum SAMPLING_RATE {
    SAMPLING_RATE_8_KHZ = 8000,
    SAMPLING_RATE_16_KHZ = 16000,
    SAMPLING_RATE_44_1_KHZ = 44100,
    SAMPLING_RATE_48_KHZ = 48000,
    SAMPLING_RATE_88_2_KHZ = 88200,
    SAMPLING_RATE_96_KHZ = 96000,
    SAMPLING_RATE_192_KHZ = 192000,
    SAMPLING_RATE_DEFAULT = SAMPLING_RATE_48_KHZ,
    SAMPLING_RATE_UNKNOWN,
  };
  typedef std::map<CH, CH> ChannelMapper;

protected:
  ENCODING mEncoding;
  int mSamplingRate;
  CHANNEL mChannel;

public:
  AudioFormat(ENCODING encoding = ENCODING_DEFAULT, int samplingRate = SAMPLING_RATE_DEFAULT, CHANNEL channel = CHANNEL_DEFAULT);
  virtual ~AudioFormat();

  ENCODING getEncoding(void);
  bool isEncodingPcm(void);
  static bool isEncodingPcm(ENCODING encoding);
  bool isEncodingCompressed(void);
  static bool isEncodingCompressed(ENCODING encoding);
  static std::string getEncodingString(ENCODING encoding);
  int getNumberOfChannels(void);
  static int getNumberOfChannels(CHANNEL channel);
  std::string getEncodingString(void);
  CHANNEL getChannels(void);
  int getSampleByte(void);
  static int getSampleByte(ENCODING encoding);
  int getChannelsSampleByte(void);
  static int getChannelsSampleByte(ENCODING encoding, CHANNEL channel);
  int getSamplingRate(void);
  int getOffSetByteInSample(AudioFormat::CH ch);
  static int getOffSetByteInSample(AudioFormat audioFormat, AudioFormat::CH ch);
  int getOffSetInSample(AudioFormat::CH ch);
  static int getOffSetInSample(AudioFormat::CHANNEL channel, AudioFormat::CH ch);
  bool equal(AudioFormat arg2);
  ChannelMapper getSameChannelMapper(void);
  static ChannelMapper getSameChannelMapper(CHANNEL channel);
  std::string toString(void);
};

class AudioBase
{
public:
  virtual std::vector<AudioFormat> getSupportedAudioFormats(void);
  virtual bool isAvailableFormat(AudioFormat format);
  virtual std::vector<AudioFormat> audioFormatOpAND(std::vector<AudioFormat>& formats1, std::vector<AudioFormat>& formats2);
  virtual std::vector<AudioFormat> audioFormatOpOR(std::vector<AudioFormat>& formats1, std::vector<AudioFormat>& formats2);
};

#endif /* __AUDIO_FORMAT_HPP__ */
