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

#ifndef __CHANNELDEMULTIPLEXER_HPP__
#define __CHANNELDEMULTIPLEXER_HPP__

#include "Buffer.hpp"
#include <memory>

class ChannelDemuxer
{
public:
  static std::vector<std::shared_ptr<AudioBuffer>> perChannelDemux(std::shared_ptr<AudioBuffer> pSrcBuffer);
  static std::vector<std::shared_ptr<AudioBuffer>> perChannelDemux(std::shared_ptr<AudioBuffer> pSrcBuffer, std::vector<std::vector<AudioFormat::CH>> channels);
};

#endif /* __CHANNELDEMULTIPLEXER_HPP__ */