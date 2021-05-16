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

#ifndef __STREAM_HPP__
#define __STREAM_HPP__

#include "Buffer.hpp"
#include <string>
#include <fstream>

class IStream
{
public:
  virtual bool isEndOfStream(void){ return true; };
  virtual int read(ByteBuffer& buf){ return 0; };
  virtual ByteBuffer* read(void){ return nullptr; };
  virtual void write(ByteBuffer& buf){};
  virtual bool writeLine(std::string& line){ return false; };
  virtual bool readLine(std::string& line){ return false; };
  virtual void close(void){};
};

class FileStream : public IStream
{
protected:
  std::fstream mStream;
  void ensureFile(std::string path);
  bool mOpened;
  uint64_t mPos;

public:
  FileStream(std::string path);
  virtual ~FileStream();

  virtual bool isEndOfStream(void);
  virtual int read(ByteBuffer& buf);
  virtual ByteBuffer* read(void);
  virtual void write(ByteBuffer& buf);
  virtual bool writeLine(std::string& line);
  virtual bool readLine(std::string& line);
  virtual void close(void);
};

#endif /* __STREAM_HPP__ */