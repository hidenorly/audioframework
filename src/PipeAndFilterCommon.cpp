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

#include "PipeAndFilterCommon.hpp"

void IMuteable::setMuteEnabled(bool bEnableMute, bool bUseZero)
{
  bool bChanged = ( bEnableMute != mMuteEnabled );
  mMuteEnabled = bEnableMute;
  mMuteUseZeroEnabled = bUseZero;
  if( bChanged ){
    mutePrimitive(bEnableMute, bUseZero);
  }
}


float ISourceSinkCommon::getVolume(void)
{
  return getMuteEnabled() ? 0.0f : !mIsPerChannelVolume ? mVolume : Volume::getVolumeMax(mPerChannelVolumes);
}

bool ISourceSinkCommon::setVolume(float volumePercentage)
{
  mIsPerChannelVolume = false;
  mVolume = volumePercentage;
  return true;
}

bool ISourceSinkCommon::setVolume(Volume::CHANNEL_VOLUME perChannelVolumes)
{
  return setVolume( Volume::getPerChannelVolumes(getAudioFormat(), perChannelVolumes) );
}

bool ISourceSinkCommon::setVolume(std::vector<float> perChannelVolumes)
{
  mIsPerChannelVolume = true;
  mPerChannelVolumes = perChannelVolumes;
  return true;
}
