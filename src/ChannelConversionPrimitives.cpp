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

#include "ChannelConversionPrimitives.hpp"

// TODO: rewrite with template
// DO NOT USE THIS TINY CHANNEL CONVERSION in the production device
// SHOULD REPLACE WITH HIGH QUALITY SRC

#ifndef USE_TINY_CC_IMPL
  #define USE_TINY_CC_IMPL 1
#endif /* USE_TINY_CC_IMPL */

#if USE_TINY_CC_IMPL

std::vector<ChannelConverter::ChannelMapList> ChannelConverter::getChannelConversionMapList(AudioFormat::CHANNEL srcChannel, AudioFormat::CHANNEL dstChannel)
{
  struct CONVERT_CH_TABLE
  {
  public:
    AudioFormat::CHANNEL srcChannel;
    AudioFormat::CHANNEL dstChannel;
    std::vector<ChannelMapList> chMapList;

    CONVERT_CH_TABLE(AudioFormat::CHANNEL srcChannel, AudioFormat::CHANNEL dstChannel, std::vector<ChannelMapList> chMapList):srcChannel(srcChannel),dstChannel(dstChannel),chMapList(chMapList){};
  };

  static CONVERT_CH_TABLE convertChTable[]=
  {
    CONVERT_CH_TABLE(AudioFormat::CHANNEL::CHANNEL_MONO, AudioFormat::CHANNEL::CHANNEL_STEREO, std::vector<ChannelMapList>{
      {AudioFormat::CH::MONO, AudioFormat::CH::L},
      {AudioFormat::CH::MONO, AudioFormat::CH::R} }),
    CONVERT_CH_TABLE(AudioFormat::CHANNEL::CHANNEL_MONO, AudioFormat::CHANNEL::CHANNEL_2_1CH, std::vector<ChannelMapList>{
      {AudioFormat::CH::MONO, AudioFormat::CH::L, },
      {AudioFormat::CH::MONO, AudioFormat::CH::R},
      {AudioFormat::CH::MONO, AudioFormat::CH::SW} }),
    CONVERT_CH_TABLE(AudioFormat::CHANNEL::CHANNEL_MONO, AudioFormat::CHANNEL::CHANNEL_4CH, std::vector<ChannelMapList>{
      {AudioFormat::CH::MONO, AudioFormat::CH::L, },
      {AudioFormat::CH::MONO, AudioFormat::CH::R},
      {AudioFormat::CH::MONO, AudioFormat::CH::SL},
      {AudioFormat::CH::MONO, AudioFormat::CH::SR} }),
    CONVERT_CH_TABLE(AudioFormat::CHANNEL::CHANNEL_MONO, AudioFormat::CHANNEL::CHANNEL_5CH, std::vector<ChannelMapList>{
      {AudioFormat::CH::MONO, AudioFormat::CH::L},
      {AudioFormat::CH::MONO, AudioFormat::CH::R},
      {AudioFormat::CH::MONO, AudioFormat::CH::SL},
      {AudioFormat::CH::MONO, AudioFormat::CH::SR},
      {AudioFormat::CH::MONO, AudioFormat::CH::C} }),
    CONVERT_CH_TABLE(AudioFormat::CHANNEL::CHANNEL_MONO, AudioFormat::CHANNEL::CHANNEL_5_1CH, std::vector<ChannelMapList>{
      {AudioFormat::CH::MONO, AudioFormat::CH::L},
      {AudioFormat::CH::MONO, AudioFormat::CH::R},
      {AudioFormat::CH::MONO, AudioFormat::CH::SL},
      {AudioFormat::CH::MONO, AudioFormat::CH::SR},
      {AudioFormat::CH::MONO, AudioFormat::CH::C},
      {AudioFormat::CH::MONO, AudioFormat::CH::SW} }),
    CONVERT_CH_TABLE(AudioFormat::CHANNEL::CHANNEL_MONO, AudioFormat::CHANNEL::CHANNEL_5_0_2CH, std::vector<ChannelMapList>{
      {AudioFormat::CH::MONO, AudioFormat::CH::L},
      {AudioFormat::CH::MONO, AudioFormat::CH::R},
      {AudioFormat::CH::MONO, AudioFormat::CH::SL},
      {AudioFormat::CH::MONO, AudioFormat::CH::SR},
      {AudioFormat::CH::MONO, AudioFormat::CH::C},
      {AudioFormat::CH::MONO, AudioFormat::CH::SBL},
      {AudioFormat::CH::MONO, AudioFormat::CH::SBR} }),
    CONVERT_CH_TABLE(AudioFormat::CHANNEL::CHANNEL_MONO, AudioFormat::CHANNEL::CHANNEL_5_1_2CH, std::vector<ChannelMapList>{
      {AudioFormat::CH::MONO, AudioFormat::CH::L},
      {AudioFormat::CH::MONO, AudioFormat::CH::R},
      {AudioFormat::CH::MONO, AudioFormat::CH::SL},
      {AudioFormat::CH::MONO, AudioFormat::CH::SR},
      {AudioFormat::CH::MONO, AudioFormat::CH::C},
      {AudioFormat::CH::MONO, AudioFormat::CH::SW},
      {AudioFormat::CH::MONO, AudioFormat::CH::SBL},
      {AudioFormat::CH::MONO, AudioFormat::CH::SBR} }),
    CONVERT_CH_TABLE(AudioFormat::CHANNEL::CHANNEL_MONO, AudioFormat::CHANNEL::CHANNEL_7_1CH, std::vector<ChannelMapList>{
      {AudioFormat::CH::MONO, AudioFormat::CH::L},
      {AudioFormat::CH::MONO, AudioFormat::CH::R},
      {AudioFormat::CH::MONO, AudioFormat::CH::SL},
      {AudioFormat::CH::MONO, AudioFormat::CH::SR},
      {AudioFormat::CH::MONO, AudioFormat::CH::C},
      {AudioFormat::CH::MONO, AudioFormat::CH::SW},
      {AudioFormat::CH::MONO, AudioFormat::CH::SBL},
      {AudioFormat::CH::MONO, AudioFormat::CH::SBR} }),

    CONVERT_CH_TABLE(AudioFormat::CHANNEL::CHANNEL_STEREO, AudioFormat::CHANNEL::CHANNEL_MONO, std::vector<ChannelMapList>{
      {AudioFormat::CH::L,  AudioFormat::CH::MONO},
      {AudioFormat::CH::R,  AudioFormat::CH::MONO} }),
    CONVERT_CH_TABLE(AudioFormat::CHANNEL::CHANNEL_STEREO, AudioFormat::CHANNEL::CHANNEL_2_1CH, std::vector<ChannelMapList>{
      {AudioFormat::CH::L,  AudioFormat::CH::L},
      {AudioFormat::CH::R,  AudioFormat::CH::R},
      {AudioFormat::CH::L,  AudioFormat::CH::SW},
      {AudioFormat::CH::R,  AudioFormat::CH::SW} }),
    CONVERT_CH_TABLE(AudioFormat::CHANNEL::CHANNEL_STEREO, AudioFormat::CHANNEL::CHANNEL_4CH, std::vector<ChannelMapList>{
      {AudioFormat::CH::L,  AudioFormat::CH::L},
      {AudioFormat::CH::R,  AudioFormat::CH::R},
      {AudioFormat::CH::L,  AudioFormat::CH::SL},
      {AudioFormat::CH::R,  AudioFormat::CH::SR} }),
    CONVERT_CH_TABLE(AudioFormat::CHANNEL::CHANNEL_STEREO, AudioFormat::CHANNEL::CHANNEL_5CH, std::vector<ChannelMapList>{
      {AudioFormat::CH::L,  AudioFormat::CH::L},
      {AudioFormat::CH::R,  AudioFormat::CH::R},
      {AudioFormat::CH::L,  AudioFormat::CH::SL},
      {AudioFormat::CH::R,  AudioFormat::CH::SR},
      {AudioFormat::CH::L,  AudioFormat::CH::C},
      {AudioFormat::CH::R,  AudioFormat::CH::C} }),
    CONVERT_CH_TABLE(AudioFormat::CHANNEL::CHANNEL_STEREO, AudioFormat::CHANNEL::CHANNEL_5_1CH, std::vector<ChannelMapList>{
      {AudioFormat::CH::L,  AudioFormat::CH::L},
      {AudioFormat::CH::R,  AudioFormat::CH::R},
      {AudioFormat::CH::L,  AudioFormat::CH::SL},
      {AudioFormat::CH::R,  AudioFormat::CH::SR},
      {AudioFormat::CH::L,  AudioFormat::CH::C},
      {AudioFormat::CH::R,  AudioFormat::CH::C},
      {AudioFormat::CH::L,  AudioFormat::CH::SW},
      {AudioFormat::CH::R,  AudioFormat::CH::SW} }),
    CONVERT_CH_TABLE(AudioFormat::CHANNEL::CHANNEL_STEREO, AudioFormat::CHANNEL::CHANNEL_5_0_2CH, std::vector<ChannelMapList>{
      {AudioFormat::CH::L,  AudioFormat::CH::L},
      {AudioFormat::CH::R,  AudioFormat::CH::R},
      {AudioFormat::CH::L,  AudioFormat::CH::SL},
      {AudioFormat::CH::R,  AudioFormat::CH::SR},
      {AudioFormat::CH::L,  AudioFormat::CH::C},
      {AudioFormat::CH::R,  AudioFormat::CH::C},
      {AudioFormat::CH::L,  AudioFormat::CH::SBL},
      {AudioFormat::CH::R,  AudioFormat::CH::SBR} }),
    CONVERT_CH_TABLE(AudioFormat::CHANNEL::CHANNEL_STEREO, AudioFormat::CHANNEL::CHANNEL_5_1_2CH, std::vector<ChannelMapList>{
      {AudioFormat::CH::L,  AudioFormat::CH::L},
      {AudioFormat::CH::R,  AudioFormat::CH::R},
      {AudioFormat::CH::L,  AudioFormat::CH::SL},
      {AudioFormat::CH::R,  AudioFormat::CH::SR},
      {AudioFormat::CH::L,  AudioFormat::CH::C},
      {AudioFormat::CH::R,  AudioFormat::CH::C},
      {AudioFormat::CH::L,  AudioFormat::CH::SW},
      {AudioFormat::CH::R,  AudioFormat::CH::SW},
      {AudioFormat::CH::L,  AudioFormat::CH::SBL},
      {AudioFormat::CH::R,  AudioFormat::CH::SBR} }),
    CONVERT_CH_TABLE(AudioFormat::CHANNEL::CHANNEL_STEREO, AudioFormat::CHANNEL::CHANNEL_7_1CH, std::vector<ChannelMapList>{
      {AudioFormat::CH::L,  AudioFormat::CH::L},
      {AudioFormat::CH::R,  AudioFormat::CH::R},
      {AudioFormat::CH::L,  AudioFormat::CH::SL},
      {AudioFormat::CH::R,  AudioFormat::CH::SR},
      {AudioFormat::CH::L,  AudioFormat::CH::C},
      {AudioFormat::CH::R,  AudioFormat::CH::C},
      {AudioFormat::CH::L,  AudioFormat::CH::SW},
      {AudioFormat::CH::R,  AudioFormat::CH::SW},
      {AudioFormat::CH::L,  AudioFormat::CH::SBL},
      {AudioFormat::CH::R,  AudioFormat::CH::SBR} }),

    CONVERT_CH_TABLE(AudioFormat::CHANNEL::CHANNEL_4CH, AudioFormat::CHANNEL::CHANNEL_MONO, std::vector<ChannelMapList>{
      {AudioFormat::CH::SL, AudioFormat::CH::MONO},
      {AudioFormat::CH::SR, AudioFormat::CH::MONO},
      {AudioFormat::CH::L,  AudioFormat::CH::MONO},
      {AudioFormat::CH::R,  AudioFormat::CH::MONO} }),
    CONVERT_CH_TABLE(AudioFormat::CHANNEL::CHANNEL_4CH, AudioFormat::CHANNEL::CHANNEL_STEREO, std::vector<ChannelMapList>{
      {AudioFormat::CH::SL, AudioFormat::CH::L},
      {AudioFormat::CH::SR, AudioFormat::CH::R},
      {AudioFormat::CH::L,  AudioFormat::CH::L},
      {AudioFormat::CH::R,  AudioFormat::CH::R} }),
    CONVERT_CH_TABLE(AudioFormat::CHANNEL::CHANNEL_4CH, AudioFormat::CHANNEL::CHANNEL_2_1CH, std::vector<ChannelMapList>{
      {AudioFormat::CH::SL, AudioFormat::CH::L},
      {AudioFormat::CH::SR, AudioFormat::CH::R},
      {AudioFormat::CH::L,  AudioFormat::CH::L},
      {AudioFormat::CH::R,  AudioFormat::CH::R},
      {AudioFormat::CH::L,  AudioFormat::CH::SW},
      {AudioFormat::CH::R,  AudioFormat::CH::SW} }),
    CONVERT_CH_TABLE(AudioFormat::CHANNEL::CHANNEL_4CH, AudioFormat::CHANNEL::CHANNEL_5CH, std::vector<ChannelMapList>{
      {AudioFormat::CH::L,  AudioFormat::CH::L},
      {AudioFormat::CH::R,  AudioFormat::CH::R},
      {AudioFormat::CH::SL, AudioFormat::CH::SL},
      {AudioFormat::CH::SR, AudioFormat::CH::SR},
      {AudioFormat::CH::SL, AudioFormat::CH::C},
      {AudioFormat::CH::SR, AudioFormat::CH::C},
      {AudioFormat::CH::L,  AudioFormat::CH::C},
      {AudioFormat::CH::R,  AudioFormat::CH::C},}),
    CONVERT_CH_TABLE(AudioFormat::CHANNEL::CHANNEL_4CH, AudioFormat::CHANNEL::CHANNEL_5_1CH, std::vector<ChannelMapList>{
      {AudioFormat::CH::L,  AudioFormat::CH::L},
      {AudioFormat::CH::R,  AudioFormat::CH::R},
      {AudioFormat::CH::SL, AudioFormat::CH::SL},
      {AudioFormat::CH::SR, AudioFormat::CH::SR},
      {AudioFormat::CH::SL, AudioFormat::CH::C},
      {AudioFormat::CH::SR, AudioFormat::CH::C},
      {AudioFormat::CH::L,  AudioFormat::CH::C},
      {AudioFormat::CH::R,  AudioFormat::CH::C},
      {AudioFormat::CH::SL, AudioFormat::CH::SW},
      {AudioFormat::CH::SR, AudioFormat::CH::SW},
      {AudioFormat::CH::L,  AudioFormat::CH::SW},
      {AudioFormat::CH::R,  AudioFormat::CH::SW},}),
    CONVERT_CH_TABLE(AudioFormat::CHANNEL::CHANNEL_4CH, AudioFormat::CHANNEL::CHANNEL_5_0_2CH, std::vector<ChannelMapList>{
      {AudioFormat::CH::L,  AudioFormat::CH::L},
      {AudioFormat::CH::R,  AudioFormat::CH::R},
      {AudioFormat::CH::SL, AudioFormat::CH::SL},
      {AudioFormat::CH::SR, AudioFormat::CH::SR},
      {AudioFormat::CH::SL, AudioFormat::CH::C},
      {AudioFormat::CH::SR, AudioFormat::CH::C},
      {AudioFormat::CH::L,  AudioFormat::CH::C},
      {AudioFormat::CH::R,  AudioFormat::CH::C},
      {AudioFormat::CH::SL, AudioFormat::CH::SBL},
      {AudioFormat::CH::L,  AudioFormat::CH::SBL},
      {AudioFormat::CH::SR, AudioFormat::CH::SBR},
      {AudioFormat::CH::R,  AudioFormat::CH::SBR},}),
    CONVERT_CH_TABLE(AudioFormat::CHANNEL::CHANNEL_4CH, AudioFormat::CHANNEL::CHANNEL_5_1_2CH, std::vector<ChannelMapList>{
      {AudioFormat::CH::L,  AudioFormat::CH::L},
      {AudioFormat::CH::R,  AudioFormat::CH::R},
      {AudioFormat::CH::SL, AudioFormat::CH::SL},
      {AudioFormat::CH::SR, AudioFormat::CH::SR},
      {AudioFormat::CH::SL, AudioFormat::CH::C},
      {AudioFormat::CH::SR, AudioFormat::CH::C},
      {AudioFormat::CH::L,  AudioFormat::CH::C},
      {AudioFormat::CH::R,  AudioFormat::CH::C},
      {AudioFormat::CH::SL, AudioFormat::CH::SBL},
      {AudioFormat::CH::L,  AudioFormat::CH::SBL},
      {AudioFormat::CH::SR, AudioFormat::CH::SBR},
      {AudioFormat::CH::R,  AudioFormat::CH::SBR},
      {AudioFormat::CH::SL, AudioFormat::CH::SW},
      {AudioFormat::CH::SR, AudioFormat::CH::SW},
      {AudioFormat::CH::L,  AudioFormat::CH::SW},
      {AudioFormat::CH::R,  AudioFormat::CH::SW},}),
    CONVERT_CH_TABLE(AudioFormat::CHANNEL::CHANNEL_4CH, AudioFormat::CHANNEL::CHANNEL_7_1CH, std::vector<ChannelMapList>{
      {AudioFormat::CH::L,  AudioFormat::CH::L},
      {AudioFormat::CH::R,  AudioFormat::CH::R},
      {AudioFormat::CH::SL, AudioFormat::CH::SL},
      {AudioFormat::CH::SR, AudioFormat::CH::SR},
      {AudioFormat::CH::SL, AudioFormat::CH::C},
      {AudioFormat::CH::SR, AudioFormat::CH::C},
      {AudioFormat::CH::L,  AudioFormat::CH::C},
      {AudioFormat::CH::R,  AudioFormat::CH::C},
      {AudioFormat::CH::SL, AudioFormat::CH::SBL},
      {AudioFormat::CH::L,  AudioFormat::CH::SBL},
      {AudioFormat::CH::SR, AudioFormat::CH::SBR},
      {AudioFormat::CH::R,  AudioFormat::CH::SBR},
      {AudioFormat::CH::SL, AudioFormat::CH::SW},
      {AudioFormat::CH::SR, AudioFormat::CH::SW},
      {AudioFormat::CH::L,  AudioFormat::CH::SW},
      {AudioFormat::CH::R,  AudioFormat::CH::SW},}),

    CONVERT_CH_TABLE(AudioFormat::CHANNEL::CHANNEL_5CH, AudioFormat::CHANNEL::CHANNEL_MONO, std::vector<ChannelMapList>{
      {AudioFormat::CH::SL, AudioFormat::CH::MONO},
      {AudioFormat::CH::SR, AudioFormat::CH::MONO},
      {AudioFormat::CH::C,  AudioFormat::CH::MONO},
      {AudioFormat::CH::L,  AudioFormat::CH::MONO},
      {AudioFormat::CH::R,  AudioFormat::CH::MONO} }),
    CONVERT_CH_TABLE(AudioFormat::CHANNEL::CHANNEL_5CH, AudioFormat::CHANNEL::CHANNEL_STEREO, std::vector<ChannelMapList>{
      {AudioFormat::CH::SL, AudioFormat::CH::L},
      {AudioFormat::CH::SR, AudioFormat::CH::R},
      {AudioFormat::CH::L,  AudioFormat::CH::L},
      {AudioFormat::CH::R,  AudioFormat::CH::R} }), // Ignore C
    CONVERT_CH_TABLE(AudioFormat::CHANNEL::CHANNEL_5CH, AudioFormat::CHANNEL::CHANNEL_2_1CH, std::vector<ChannelMapList>{
      {AudioFormat::CH::L,  AudioFormat::CH::L},
      {AudioFormat::CH::R,  AudioFormat::CH::R},
      {AudioFormat::CH::C,  AudioFormat::CH::SW},}),
    CONVERT_CH_TABLE(AudioFormat::CHANNEL::CHANNEL_5CH, AudioFormat::CHANNEL::CHANNEL_4CH, std::vector<ChannelMapList>{
      {AudioFormat::CH::L,  AudioFormat::CH::L},
      {AudioFormat::CH::R,  AudioFormat::CH::R},
      {AudioFormat::CH::SL, AudioFormat::CH::SL},
      {AudioFormat::CH::SR, AudioFormat::CH::SR},}), // Ignore C
    CONVERT_CH_TABLE(AudioFormat::CHANNEL::CHANNEL_5CH, AudioFormat::CHANNEL::CHANNEL_5_1CH, std::vector<ChannelMapList>{
      {AudioFormat::CH::L,  AudioFormat::CH::L},
      {AudioFormat::CH::R,  AudioFormat::CH::R},
      {AudioFormat::CH::SL, AudioFormat::CH::SL},
      {AudioFormat::CH::SR, AudioFormat::CH::SR},
      {AudioFormat::CH::C,  AudioFormat::CH::C},
      {AudioFormat::CH::SL, AudioFormat::CH::SW},
      {AudioFormat::CH::SR, AudioFormat::CH::SW},
      {AudioFormat::CH::C,  AudioFormat::CH::SW},
      {AudioFormat::CH::L,  AudioFormat::CH::SW},
      {AudioFormat::CH::R,  AudioFormat::CH::SW},}),
    CONVERT_CH_TABLE(AudioFormat::CHANNEL::CHANNEL_5CH, AudioFormat::CHANNEL::CHANNEL_5_0_2CH, std::vector<ChannelMapList>{
      {AudioFormat::CH::L,  AudioFormat::CH::L},
      {AudioFormat::CH::R,  AudioFormat::CH::R},
      {AudioFormat::CH::SL, AudioFormat::CH::SL},
      {AudioFormat::CH::SR, AudioFormat::CH::SR},
      {AudioFormat::CH::C,  AudioFormat::CH::C},
      {AudioFormat::CH::SL, AudioFormat::CH::SBL},
      {AudioFormat::CH::L,  AudioFormat::CH::SBL},
      {AudioFormat::CH::SR, AudioFormat::CH::SBR},
      {AudioFormat::CH::R,  AudioFormat::CH::SBR},}),
    CONVERT_CH_TABLE(AudioFormat::CHANNEL::CHANNEL_5CH, AudioFormat::CHANNEL::CHANNEL_5_1_2CH, std::vector<ChannelMapList>{
      {AudioFormat::CH::L,  AudioFormat::CH::L},
      {AudioFormat::CH::R,  AudioFormat::CH::R},
      {AudioFormat::CH::SL, AudioFormat::CH::SL},
      {AudioFormat::CH::SR, AudioFormat::CH::SR},
      {AudioFormat::CH::C,  AudioFormat::CH::C},
      {AudioFormat::CH::SL, AudioFormat::CH::SW},
      {AudioFormat::CH::SR, AudioFormat::CH::SW},
      {AudioFormat::CH::C,  AudioFormat::CH::SW},
      {AudioFormat::CH::L,  AudioFormat::CH::SW},
      {AudioFormat::CH::R,  AudioFormat::CH::SW},
      {AudioFormat::CH::SL, AudioFormat::CH::SBL},
      {AudioFormat::CH::L,  AudioFormat::CH::SBL},
      {AudioFormat::CH::SR, AudioFormat::CH::SBR},
      {AudioFormat::CH::R,  AudioFormat::CH::SBR},}),
    CONVERT_CH_TABLE(AudioFormat::CHANNEL::CHANNEL_5CH, AudioFormat::CHANNEL::CHANNEL_7_1CH, std::vector<ChannelMapList>{
      {AudioFormat::CH::L,  AudioFormat::CH::L},
      {AudioFormat::CH::R,  AudioFormat::CH::R},
      {AudioFormat::CH::SL, AudioFormat::CH::SL},
      {AudioFormat::CH::SR, AudioFormat::CH::SR},
      {AudioFormat::CH::C,  AudioFormat::CH::C},
      {AudioFormat::CH::SL, AudioFormat::CH::SW},
      {AudioFormat::CH::SR, AudioFormat::CH::SW},
      {AudioFormat::CH::C,  AudioFormat::CH::SW},
      {AudioFormat::CH::L,  AudioFormat::CH::SW},
      {AudioFormat::CH::R,  AudioFormat::CH::SW},
      {AudioFormat::CH::SL, AudioFormat::CH::SBL},
      {AudioFormat::CH::L,  AudioFormat::CH::SBL},
      {AudioFormat::CH::SR, AudioFormat::CH::SBR},
      {AudioFormat::CH::R,  AudioFormat::CH::SBR},}),

    CONVERT_CH_TABLE(AudioFormat::CHANNEL::CHANNEL_5_0_2CH, AudioFormat::CHANNEL::CHANNEL_MONO, std::vector<ChannelMapList>{
      {AudioFormat::CH::SBL,  AudioFormat::CH::MONO},
      {AudioFormat::CH::SBR,  AudioFormat::CH::MONO},
      {AudioFormat::CH::SL,   AudioFormat::CH::MONO},
      {AudioFormat::CH::SR,   AudioFormat::CH::MONO},
      {AudioFormat::CH::C,    AudioFormat::CH::MONO},
      {AudioFormat::CH::L,    AudioFormat::CH::MONO},
      {AudioFormat::CH::R,    AudioFormat::CH::MONO} }),
    CONVERT_CH_TABLE(AudioFormat::CHANNEL::CHANNEL_5_0_2CH, AudioFormat::CHANNEL::CHANNEL_STEREO, std::vector<ChannelMapList>{
      {AudioFormat::CH::SBL,  AudioFormat::CH::L},
      {AudioFormat::CH::SBR,  AudioFormat::CH::R},
      {AudioFormat::CH::SL,   AudioFormat::CH::L},
      {AudioFormat::CH::SR,   AudioFormat::CH::R},
      {AudioFormat::CH::C,    AudioFormat::CH::L},
      {AudioFormat::CH::C,    AudioFormat::CH::R},
      {AudioFormat::CH::L,    AudioFormat::CH::L},
      {AudioFormat::CH::R,    AudioFormat::CH::R} }),
    CONVERT_CH_TABLE(AudioFormat::CHANNEL::CHANNEL_5_0_2CH, AudioFormat::CHANNEL::CHANNEL_2_1CH, std::vector<ChannelMapList>{
      {AudioFormat::CH::SBL,  AudioFormat::CH::L},
      {AudioFormat::CH::SBR,  AudioFormat::CH::R},
      {AudioFormat::CH::SL,   AudioFormat::CH::L},
      {AudioFormat::CH::SR,   AudioFormat::CH::R},
      {AudioFormat::CH::C,    AudioFormat::CH::L},
      {AudioFormat::CH::C,    AudioFormat::CH::R},
      {AudioFormat::CH::L,    AudioFormat::CH::L},
      {AudioFormat::CH::R,    AudioFormat::CH::R} }),
    CONVERT_CH_TABLE(AudioFormat::CHANNEL::CHANNEL_5_0_2CH, AudioFormat::CHANNEL::CHANNEL_4CH, std::vector<ChannelMapList>{
      {AudioFormat::CH::SBL,  AudioFormat::CH::SL},
      {AudioFormat::CH::SBR,  AudioFormat::CH::SR},
      {AudioFormat::CH::SL,   AudioFormat::CH::SL},
      {AudioFormat::CH::SR,   AudioFormat::CH::SR},
      {AudioFormat::CH::C,    AudioFormat::CH::L},
      {AudioFormat::CH::C,    AudioFormat::CH::R},
      {AudioFormat::CH::L,    AudioFormat::CH::L},
      {AudioFormat::CH::R,    AudioFormat::CH::R} }),
    CONVERT_CH_TABLE(AudioFormat::CHANNEL::CHANNEL_5_0_2CH, AudioFormat::CHANNEL::CHANNEL_5CH, std::vector<ChannelMapList>{
      {AudioFormat::CH::SBL,  AudioFormat::CH::SL},
      {AudioFormat::CH::SBR,  AudioFormat::CH::SR},
      {AudioFormat::CH::SL,   AudioFormat::CH::SL},
      {AudioFormat::CH::SR,   AudioFormat::CH::SR},
      {AudioFormat::CH::C,    AudioFormat::CH::C},
      {AudioFormat::CH::L,    AudioFormat::CH::L},
      {AudioFormat::CH::R,    AudioFormat::CH::R} }), // SW ignored
    CONVERT_CH_TABLE(AudioFormat::CHANNEL::CHANNEL_5_0_2CH, AudioFormat::CHANNEL::CHANNEL_5_1CH, std::vector<ChannelMapList>{
      {AudioFormat::CH::SBL,  AudioFormat::CH::SL},
      {AudioFormat::CH::SBR,  AudioFormat::CH::SR},
      {AudioFormat::CH::SL,   AudioFormat::CH::SL},
      {AudioFormat::CH::SR,   AudioFormat::CH::SR},
      {AudioFormat::CH::C,    AudioFormat::CH::C},
      {AudioFormat::CH::L,    AudioFormat::CH::L},
      {AudioFormat::CH::R,    AudioFormat::CH::R} }),
    CONVERT_CH_TABLE(AudioFormat::CHANNEL::CHANNEL_5_0_2CH, AudioFormat::CHANNEL::CHANNEL_5_1_2CH, std::vector<ChannelMapList>{
      {AudioFormat::CH::L,    AudioFormat::CH::L},
      {AudioFormat::CH::R,    AudioFormat::CH::R},
      {AudioFormat::CH::SL,   AudioFormat::CH::SL},
      {AudioFormat::CH::SR,   AudioFormat::CH::SR},
      {AudioFormat::CH::C,    AudioFormat::CH::C},
      {AudioFormat::CH::SBL,  AudioFormat::CH::SBL},
      {AudioFormat::CH::SBR,  AudioFormat::CH::SBR},}),
    CONVERT_CH_TABLE(AudioFormat::CHANNEL::CHANNEL_5_0_2CH, AudioFormat::CHANNEL::CHANNEL_7_1CH, std::vector<ChannelMapList>{
      {AudioFormat::CH::L,    AudioFormat::CH::L},
      {AudioFormat::CH::R,    AudioFormat::CH::R},
      {AudioFormat::CH::SL,   AudioFormat::CH::SL},
      {AudioFormat::CH::SR,   AudioFormat::CH::SR},
      {AudioFormat::CH::C,    AudioFormat::CH::C},
      {AudioFormat::CH::SBL,  AudioFormat::CH::SBL},
      {AudioFormat::CH::SBR,  AudioFormat::CH::SBR},}),

    CONVERT_CH_TABLE(AudioFormat::CHANNEL::CHANNEL_5_1_2CH, AudioFormat::CHANNEL::CHANNEL_MONO, std::vector<ChannelMapList>{
      {AudioFormat::CH::SBL,  AudioFormat::CH::MONO},
      {AudioFormat::CH::SBR,  AudioFormat::CH::MONO},
      {AudioFormat::CH::SL,   AudioFormat::CH::MONO},
      {AudioFormat::CH::SR,   AudioFormat::CH::MONO},
      {AudioFormat::CH::C,    AudioFormat::CH::MONO},
      {AudioFormat::CH::SW,   AudioFormat::CH::MONO},
      {AudioFormat::CH::L,    AudioFormat::CH::MONO},
      {AudioFormat::CH::R,    AudioFormat::CH::MONO} }),
    CONVERT_CH_TABLE(AudioFormat::CHANNEL::CHANNEL_5_1_2CH, AudioFormat::CHANNEL::CHANNEL_STEREO, std::vector<ChannelMapList>{
      {AudioFormat::CH::SBL,  AudioFormat::CH::L},
      {AudioFormat::CH::SBR,  AudioFormat::CH::R},
      {AudioFormat::CH::SL,   AudioFormat::CH::L},
      {AudioFormat::CH::SR,   AudioFormat::CH::R},
      {AudioFormat::CH::C,    AudioFormat::CH::L},
      {AudioFormat::CH::C,    AudioFormat::CH::R},
      {AudioFormat::CH::SW,   AudioFormat::CH::L},
      {AudioFormat::CH::SW,   AudioFormat::CH::R},
      {AudioFormat::CH::L,    AudioFormat::CH::L},
      {AudioFormat::CH::R,    AudioFormat::CH::R} }),
    CONVERT_CH_TABLE(AudioFormat::CHANNEL::CHANNEL_5_1_2CH, AudioFormat::CHANNEL::CHANNEL_2_1CH, std::vector<ChannelMapList>{
      {AudioFormat::CH::SBL,  AudioFormat::CH::L},
      {AudioFormat::CH::SBR,  AudioFormat::CH::R},
      {AudioFormat::CH::SL,   AudioFormat::CH::L},
      {AudioFormat::CH::SR,   AudioFormat::CH::R},
      {AudioFormat::CH::C,    AudioFormat::CH::L},
      {AudioFormat::CH::C,    AudioFormat::CH::R},
      {AudioFormat::CH::SW,   AudioFormat::CH::SW},
      {AudioFormat::CH::L,    AudioFormat::CH::L},
      {AudioFormat::CH::R,    AudioFormat::CH::R} }),
    CONVERT_CH_TABLE(AudioFormat::CHANNEL::CHANNEL_5_1_2CH, AudioFormat::CHANNEL::CHANNEL_4CH, std::vector<ChannelMapList>{
      {AudioFormat::CH::SBL,  AudioFormat::CH::SL},
      {AudioFormat::CH::SBR,  AudioFormat::CH::SR},
      {AudioFormat::CH::SL,   AudioFormat::CH::SL},
      {AudioFormat::CH::SR,   AudioFormat::CH::SR},
      {AudioFormat::CH::C,    AudioFormat::CH::L},
      {AudioFormat::CH::C,    AudioFormat::CH::R},
      {AudioFormat::CH::SW,   AudioFormat::CH::L},
      {AudioFormat::CH::SW,   AudioFormat::CH::R},
      {AudioFormat::CH::L,    AudioFormat::CH::L},
      {AudioFormat::CH::R,    AudioFormat::CH::R} }),
    CONVERT_CH_TABLE(AudioFormat::CHANNEL::CHANNEL_5_1_2CH, AudioFormat::CHANNEL::CHANNEL_5CH, std::vector<ChannelMapList>{
      {AudioFormat::CH::SBL,  AudioFormat::CH::SL},
      {AudioFormat::CH::SBR,  AudioFormat::CH::SR},
      {AudioFormat::CH::SL,   AudioFormat::CH::SL},
      {AudioFormat::CH::SR,   AudioFormat::CH::SR},
      {AudioFormat::CH::C,    AudioFormat::CH::C},
      {AudioFormat::CH::L,    AudioFormat::CH::L},
      {AudioFormat::CH::R,    AudioFormat::CH::R} }), // SW ignored
    CONVERT_CH_TABLE(AudioFormat::CHANNEL::CHANNEL_5_1_2CH, AudioFormat::CHANNEL::CHANNEL_5_1CH, std::vector<ChannelMapList>{
      {AudioFormat::CH::SBL,  AudioFormat::CH::SL},
      {AudioFormat::CH::SBR,  AudioFormat::CH::SR},
      {AudioFormat::CH::SL,   AudioFormat::CH::SL},
      {AudioFormat::CH::SR,   AudioFormat::CH::SR},
      {AudioFormat::CH::C,    AudioFormat::CH::C},
      {AudioFormat::CH::SW,   AudioFormat::CH::SW},
      {AudioFormat::CH::L,    AudioFormat::CH::L},
      {AudioFormat::CH::R,    AudioFormat::CH::R} }),
    CONVERT_CH_TABLE(AudioFormat::CHANNEL::CHANNEL_5_1_2CH, AudioFormat::CHANNEL::CHANNEL_5_0_2CH, std::vector<ChannelMapList>{
      {AudioFormat::CH::SBL,  AudioFormat::CH::SBL},
      {AudioFormat::CH::SBR,  AudioFormat::CH::SBR},
      {AudioFormat::CH::SL,   AudioFormat::CH::SL},
      {AudioFormat::CH::SR,   AudioFormat::CH::SR},
      {AudioFormat::CH::C,    AudioFormat::CH::C},
      {AudioFormat::CH::L,    AudioFormat::CH::L},
      {AudioFormat::CH::R,    AudioFormat::CH::R} }),
    CONVERT_CH_TABLE(AudioFormat::CHANNEL::CHANNEL_5_1_2CH, AudioFormat::CHANNEL::CHANNEL_7_1CH, std::vector<ChannelMapList>{
      {AudioFormat::CH::L,    AudioFormat::CH::L},
      {AudioFormat::CH::R,    AudioFormat::CH::R},
      {AudioFormat::CH::SL,   AudioFormat::CH::SL},
      {AudioFormat::CH::SR,   AudioFormat::CH::SR},
      {AudioFormat::CH::C,    AudioFormat::CH::C},
      {AudioFormat::CH::SW,   AudioFormat::CH::SW},
      {AudioFormat::CH::SBL,  AudioFormat::CH::SBL},
      {AudioFormat::CH::SBR,  AudioFormat::CH::SBR},}),

    CONVERT_CH_TABLE(AudioFormat::CHANNEL::CHANNEL_7_1CH, AudioFormat::CHANNEL::CHANNEL_MONO, std::vector<ChannelMapList>{
      {AudioFormat::CH::SBL,  AudioFormat::CH::MONO},
      {AudioFormat::CH::SBR,  AudioFormat::CH::MONO},
      {AudioFormat::CH::SL,   AudioFormat::CH::MONO},
      {AudioFormat::CH::SR,   AudioFormat::CH::MONO},
      {AudioFormat::CH::C,    AudioFormat::CH::MONO},
      {AudioFormat::CH::SW,   AudioFormat::CH::MONO},
      {AudioFormat::CH::L,    AudioFormat::CH::MONO},
      {AudioFormat::CH::R,    AudioFormat::CH::MONO} }),
    CONVERT_CH_TABLE(AudioFormat::CHANNEL::CHANNEL_7_1CH, AudioFormat::CHANNEL::CHANNEL_STEREO, std::vector<ChannelMapList>{
      {AudioFormat::CH::SBL,  AudioFormat::CH::L},
      {AudioFormat::CH::SBR,  AudioFormat::CH::R},
      {AudioFormat::CH::SL,   AudioFormat::CH::L},
      {AudioFormat::CH::SR,   AudioFormat::CH::R},
      {AudioFormat::CH::C,    AudioFormat::CH::L},
      {AudioFormat::CH::C,    AudioFormat::CH::R},
      {AudioFormat::CH::SW,   AudioFormat::CH::L},
      {AudioFormat::CH::SW,   AudioFormat::CH::R},
      {AudioFormat::CH::L,    AudioFormat::CH::L},
      {AudioFormat::CH::R,    AudioFormat::CH::R} }),
    CONVERT_CH_TABLE(AudioFormat::CHANNEL::CHANNEL_7_1CH, AudioFormat::CHANNEL::CHANNEL_2_1CH, std::vector<ChannelMapList>{
      {AudioFormat::CH::SBL,  AudioFormat::CH::L},
      {AudioFormat::CH::SBR,  AudioFormat::CH::R},
      {AudioFormat::CH::SL,   AudioFormat::CH::L},
      {AudioFormat::CH::SR,   AudioFormat::CH::R},
      {AudioFormat::CH::C,    AudioFormat::CH::L},
      {AudioFormat::CH::C,    AudioFormat::CH::R},
      {AudioFormat::CH::SW,   AudioFormat::CH::SW},
      {AudioFormat::CH::L,    AudioFormat::CH::L},
      {AudioFormat::CH::R,    AudioFormat::CH::R} }),
    CONVERT_CH_TABLE(AudioFormat::CHANNEL::CHANNEL_7_1CH, AudioFormat::CHANNEL::CHANNEL_4CH, std::vector<ChannelMapList>{
      {AudioFormat::CH::SBL,  AudioFormat::CH::SL},
      {AudioFormat::CH::SBR,  AudioFormat::CH::SR},
      {AudioFormat::CH::SL,   AudioFormat::CH::SL},
      {AudioFormat::CH::SR,   AudioFormat::CH::SR},
      {AudioFormat::CH::C,    AudioFormat::CH::L},
      {AudioFormat::CH::C,    AudioFormat::CH::R},
      {AudioFormat::CH::SW,   AudioFormat::CH::L},
      {AudioFormat::CH::SW,   AudioFormat::CH::R},
      {AudioFormat::CH::L,    AudioFormat::CH::L},
      {AudioFormat::CH::R,    AudioFormat::CH::R} }),
    CONVERT_CH_TABLE(AudioFormat::CHANNEL::CHANNEL_7_1CH, AudioFormat::CHANNEL::CHANNEL_5CH, std::vector<ChannelMapList>{
      {AudioFormat::CH::SBL,  AudioFormat::CH::SL},
      {AudioFormat::CH::SBR,  AudioFormat::CH::SR},
      {AudioFormat::CH::SL,   AudioFormat::CH::SL},
      {AudioFormat::CH::SR,   AudioFormat::CH::SR},
      {AudioFormat::CH::C,    AudioFormat::CH::C},
      {AudioFormat::CH::L,    AudioFormat::CH::L},
      {AudioFormat::CH::R,    AudioFormat::CH::R} }), // SW ignored
    CONVERT_CH_TABLE(AudioFormat::CHANNEL::CHANNEL_7_1CH, AudioFormat::CHANNEL::CHANNEL_5_1CH, std::vector<ChannelMapList>{
      {AudioFormat::CH::SBL,  AudioFormat::CH::SL},
      {AudioFormat::CH::SBR,  AudioFormat::CH::SR},
      {AudioFormat::CH::SL,   AudioFormat::CH::SL},
      {AudioFormat::CH::SR,   AudioFormat::CH::SR},
      {AudioFormat::CH::C,    AudioFormat::CH::C},
      {AudioFormat::CH::SW,   AudioFormat::CH::SW},
      {AudioFormat::CH::L,    AudioFormat::CH::L},
      {AudioFormat::CH::R,    AudioFormat::CH::R} }),
    CONVERT_CH_TABLE(AudioFormat::CHANNEL::CHANNEL_7_1CH, AudioFormat::CHANNEL::CHANNEL_5_0_2CH, std::vector<ChannelMapList>{
      {AudioFormat::CH::SBL,  AudioFormat::CH::SBL},
      {AudioFormat::CH::SBR,  AudioFormat::CH::SBR},
      {AudioFormat::CH::SL,   AudioFormat::CH::SL},
      {AudioFormat::CH::SR,   AudioFormat::CH::SR},
      {AudioFormat::CH::C,    AudioFormat::CH::C},
      {AudioFormat::CH::L,    AudioFormat::CH::L},
      {AudioFormat::CH::R,    AudioFormat::CH::R} }),
    CONVERT_CH_TABLE(AudioFormat::CHANNEL::CHANNEL_7_1CH, AudioFormat::CHANNEL::CHANNEL_5_1_2CH, std::vector<ChannelMapList>{
      {AudioFormat::CH::L,    AudioFormat::CH::L},
      {AudioFormat::CH::R,    AudioFormat::CH::R},
      {AudioFormat::CH::SL,   AudioFormat::CH::SL},
      {AudioFormat::CH::SR,   AudioFormat::CH::SR},
      {AudioFormat::CH::C,    AudioFormat::CH::C},
      {AudioFormat::CH::SW,   AudioFormat::CH::SW},
      {AudioFormat::CH::SBL,  AudioFormat::CH::SBL},
      {AudioFormat::CH::SBR,  AudioFormat::CH::SBR},}),

    CONVERT_CH_TABLE(AudioFormat::CHANNEL::CHANNEL_UNKNOWN, AudioFormat::CHANNEL::CHANNEL_UNKNOWN, std::vector<ChannelMapList>{ {AudioFormat::CH::L, AudioFormat::CH::L}, {AudioFormat::CH::R, AudioFormat::CH::R} })
  };

  CONVERT_CH_TABLE* pSelectedChMapper = nullptr;
  for(int i=0; ( (convertChTable[i].srcChannel != AudioFormat::CHANNEL::CHANNEL_UNKNOWN) && (convertChTable[i].dstChannel != AudioFormat::CHANNEL::CHANNEL_UNKNOWN) ) ; i++ ) {
    if( (convertChTable[i].srcChannel == srcChannel) && (convertChTable[i].dstChannel == dstChannel) ){
      pSelectedChMapper = &convertChTable[i];
      break;
    }
  }

  return pSelectedChMapper ? pSelectedChMapper->chMapList : std::vector<ChannelMapList>{};
}

bool ChannelConverter::channelConversion(AudioBuffer& srcBuf, AudioBuffer& dstBuf, AudioFormat::CHANNEL dstChannel)
{
  AudioFormat srcFormat = srcBuf.getAudioFormat();
  AudioFormat dstFormat(srcFormat.getEncoding(), srcFormat.getSamplingRate(), dstChannel );

  std::vector<ChannelConverter::ChannelMapList> chConvMapList = getChannelConversionMapList(srcFormat.getChannels(), dstFormat.getChannels());

  if( !chConvMapList.empty() ){
    // TODO: Check case of mix e.g. L->C, R->C then L+R -> C
    // The following getSelectedChannelData expects no mix case.
    AudioFormat::ChannelMapper mapper;
    for( auto& chMap : chConvMapList ){
      mapper.insert_or_assign( chMap.dstCh, chMap.srcCh );
    }
    AudioBuffer result = srcBuf.getSelectedChannelData(dstFormat, mapper);
    dstBuf = result;
  } else {
    dstBuf = srcBuf;
  }

  return dstBuf.getAudioFormat().getChannels() == dstChannel;
}

#endif /* USE_TINY_CC_IMPL */
