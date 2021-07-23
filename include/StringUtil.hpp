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

#ifndef __STRING_UTIL_HPP__
#define __STRING_UTIL_HPP__

#include <string>

class StringUtil
{
public:
  static std::string trim(std::string value)
  {
    const static std::string trimString = " \r\n\"";

    int nPos = value.find_last_not_of( trimString );
    if( nPos != std::string::npos ){
      value = value.substr( 0, nPos+1 );
    }

    nPos = value.find_first_not_of( trimString );
    if( nPos != std::string::npos ){
      value = value.substr( nPos );
    }

    return value;
  }
};


#endif /* __STRING_UTIL_HPP__ */