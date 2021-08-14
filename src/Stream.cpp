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

#include "Stream.hpp"
#include <fstream>
#include <filesystem>

void FileStream::ensureFile(std::string path)
{
  if( !std::filesystem::exists( path ) ){
    // TODO: ensure path recursively
    std::ofstream stream( path, std::ios::out | std::ios::binary | std::ios::trunc );
    stream.close();
  }
}

FileStream::FileStream(std::string path)
{
  ensureFile( path );
  mStream.open( path, std::ios::in | std::ios::out | std::ios::binary );
  mOpened = true;
  mPos = 0;
}

FileStream::~FileStream()
{
  close();
}

bool FileStream::isEndOfStream(void)
{
  return mOpened ? mStream.eof() : true;
}

int FileStream::read(ByteBuffer& buf)
{
  int nSize = 0;

  if( mOpened ){
    mStream.read( reinterpret_cast<char*>(buf.data()), buf.size() );
    nSize = mStream.gcount();
    if( buf.size() != nSize ){
      buf.resize( nSize );
    }
  }

  return nSize;
}

std::shared_ptr<ByteBuffer> FileStream::read(void)
{
  std::shared_ptr<ByteBuffer> buf;

  if( mOpened ){
    buf = std::make_shared<ByteBuffer>();
    read(*buf);
  }

  return buf;
}

void FileStream::write(ByteBuffer& buf)
{
  if( mOpened ){
    // mStream.write( reinterpret_cast<char*>(buf.data()), buf.size() );
    // THE ABOVE SHOULD WORK BUT DOESN'T WORK THEN THE FOLLOWING IS WORKAROUND.
    for(int i=0; i<buf.size(); i++){
      mStream.seekp( mPos++ );
      mStream.write( reinterpret_cast<char*>(&buf[i]), sizeof(char) );
    }
  }
}


bool FileStream::writeLine(std::string& line)
{
  bool result = !isEndOfStream();

  if( result ){
    mStream << line << std::endl;
  }

  return result;
}

bool FileStream::readLine(std::string& line)
{
  bool result = !isEndOfStream();

  if( result ){
    result = std::getline( mStream, line ) ? true : false;
  }

  return result;
}

void FileStream::close(void)
{
  if( mOpened ){
    mStream.close();
    mOpened = false;
  }
}