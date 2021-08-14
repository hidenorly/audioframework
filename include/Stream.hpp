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
#include <memory>

/* stream I/O interface class */
class IStream
{
public:
  /* @desc check the stream is ended of stream or not
     @return true: stream is ended / false: stream is not end. */
  virtual bool isEndOfStream(void){ return true; };

  /* @desc read data from stream
     @arg buf will be output about the read data
     @return number of read bytes */
  virtual int read(ByteBuffer& buf){ return 0; };

  /* @desc read data from stream
     @return pointer of ByteBuffer. You should delete it */
  virtual std::shared_ptr<ByteBuffer> read(void){ return nullptr; };

  /* @desc write data to stream
     @arg buf: will be written to the stream
     @return number of read bytes */
  virtual void write(ByteBuffer& buf){};

  /* @desc write string to stream as one line
     @arg line: will be written to the stream
     @return true: success to write / false: fail to write */
  virtual bool writeLine(std::string& line){ return false; };

  /* @desc read one line from stream
     @arg buf will be output about the line read
     @return true: success to write / false: fail to write */
  virtual bool readLine(std::string& line){ return false; };

  /* @desc close the stream. */
  virtual void close(void){};
};

/* Stream for File */
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
  virtual std::shared_ptr<ByteBuffer> read(void);
  virtual void write(ByteBuffer& buf);
  virtual bool writeLine(std::string& line);
  virtual bool readLine(std::string& line);
  virtual void close(void);
};

#endif /* __STREAM_HPP__ */