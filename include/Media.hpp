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

#ifndef __MEDIACODEC_HPP__
#define __MEDIACODEC_HPP__

#include <vector>
#include <string>

class MediaParam
{
public:
  std::string key;
  std::string value;

  void set(std::string arg){ value = arg; };
  void setInt(int arg){ value = std::to_string( arg ); };
  void setBool(bool arg){ value = arg ? "true" : "false"; };

  std::string get(void){ return value; };
  int getInt(void){ return std::stoi( value ); };
  bool getBool(void){ return (value == "true") ? true : false; };

  MediaParam(std::string key, std::string value):key(key){ set(value); };
  MediaParam(std::string key, int value):key(key){ setInt(value); };
  MediaParam(std::string key, bool value):key(key){ setBool(value); };
};

#endif /* __MEDIA_HPP__ */