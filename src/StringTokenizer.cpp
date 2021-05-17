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

#include "StringTokenizer.hpp"
#include <string>
#include <iostream>

StringTokenizer::StringTokenizer(std::string sourceString, std::string token) : mBuf(sourceString), mToken(token), mPos(0)
{
  mBufLength = sourceString.length();
  mTokenLength = token.length();
}

StringTokenizer::~StringTokenizer()
{

}

std::string StringTokenizer::getNext()
{
  std::string result = "";

  int pos = mBuf.find( mToken, mPos );
  if( pos != std::string::npos ){
    result = mBuf.substr( mPos, pos - mPos );
    mPos = pos + mTokenLength;
  } else {
    result = mBuf.substr( mPos );
    mPos = mBuf.length();
  }

  return result;
}

bool StringTokenizer::hasNext(void)
{
  return ( mPos < mBufLength ) ? true : false;
}
