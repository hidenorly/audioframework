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

#ifndef __AUDIOFORMATADAPTOR_HPP__
#define __AUDIOFORMATADAPTOR_HPP__

#include "AudioFormat.hpp"
#include "Buffer.hpp"

class AudioFormatAdaptor
{
public:
  static bool convert(AudioBuffer& srcBuf, AudioBuffer& dstBuf);

  static bool encodingConversion(AudioBuffer& srcBuf, AudioBuffer& dstBuf, AudioFormat::ENCODING dstEncoding);
  static bool samplingRateConversion(AudioBuffer& srcBuf, AudioBuffer& dstBuf, int dstSamplingRate);
  static bool channelConversion(AudioBuffer& srcBuf, AudioBuffer& dstBuf, AudioFormat::CHANNEL dstChannel);
};

#endif /* __AUDIOFORMATADAPTOR_HPP__ */