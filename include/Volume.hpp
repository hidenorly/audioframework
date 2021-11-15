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

#ifndef __VOLUME_HPP__
#define __VOLUME_HPP__

#include "Buffer.hpp"
#include "AudioFormat.hpp"
#include <map>
#include <vector>

class Volume
{
public:
  typedef std::map<AudioFormat::CH, float> CHANNEL_VOLUME;
  static bool process( AudioBuffer* pInBuf, AudioBuffer* pOutBuf, float volume );
  static bool process( AudioBuffer& inBuf, AudioBuffer& outBuf, float volume );
  static bool process( AudioBuffer* pInBuf, AudioBuffer* pOutBuf, std::vector<float> channelVolumes );
  static bool process( AudioBuffer* pInBuf, AudioBuffer* pOutBuf, CHANNEL_VOLUME volumes );
  static bool process( AudioBuffer& inBuf, AudioBuffer& outBuf, CHANNEL_VOLUME volumes );

  static bool isVolumeRequired(std::vector<float> channelVolumes);
  static bool isVolumeRequired(AudioFormat format, CHANNEL_VOLUME channelVolumes);
  static std::vector<float> getPerChannelVolumes(AudioFormat format, CHANNEL_VOLUME channelVolumes);
  static float getVolumeMax(std::vector<float> volumes);

  static CHANNEL_VOLUME getChannelVolume(AudioFormat::CHANNEL channel, std::vector<float> channelVolumes);
};

#endif /* __VOLUME_HPP__ */
