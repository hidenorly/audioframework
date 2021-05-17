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

#ifndef __STRING_TOKENIZER_HPP__
#define __STRING_TOKENIZER_HPP__

#include <string>

class StringTokenizer
{
protected:
  std::string mBuf;
  std::string mToken;
  int mPos;
  int mBufLength;
  int mTokenLength;

public:
  StringTokenizer(std::string sourceString, std::string token);
  virtual ~StringTokenizer();

  std::string getNext();
  bool hasNext(void);
};


#endif /* __STRING_TOKENIZER_HPP__ */