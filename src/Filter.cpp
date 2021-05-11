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

#include "Filter.hpp"

IFilter::~IFilter()
{

}

void IFilter::process(AudioBuffer& inBuf, AudioBuffer& outBuf)
{
  outBuf = inBuf;
}

int IFilter::getRequiredWindowSizeUsec(void)
{
  return DEFAULT_WINDOW_SIZE_USEC;
}

int IFilter::getLatencyUSec(void)
{
  return DEFAULT_WINDOW_SIZE_USEC; // usually window size + processing time
}


Filter::Filter()
{

}

Filter::~Filter()
{

}

std::vector<AudioFormat> Filter::getSupportedAudioFormats(void)
{
    std::vector<AudioFormat> audioFormats;
    audioFormats.push_back( AudioFormat() );
    return audioFormats;
}

int Filter::getExpectedProcessingUSec(void)
{
  return DEFAULT_PROCESSING_TIME_USEC;
}