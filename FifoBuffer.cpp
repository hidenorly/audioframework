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

#include "FifoBuffer.hpp"
#include <iterator>

FifoBuffer::FifoBuffer(AudioFormat& format):mFormat(format)
{

}

FifoBuffer::~FifoBuffer()
{

}

bool FifoBuffer::read(AudioBuffer& audioBuf)
{
  bool bResult = audioBuf.getAudioFormat().equal( mFormat );

  if( bResult ){
    ByteBuffer readBuffer = audioBuf.getRawBuffer();
    int size = readBuffer.size();

    std::atomic<bool> bReceived = false;
    while( !bReceived ){
      if( mBuf.size() >= size ){
        mMutex.lock();
        {
          readBuffer = ByteBuffer( mBuf.begin(), mBuf.begin()+size-1 );
          audioBuf.setRawBuffer( readBuffer );
          bReceived = true;
          if( mBuf.size() > size ){
            mBuf = ByteBuffer( mBuf.begin()+size, mBuf.end() );
          } else {
            mBuf.clear();
          }
        }
        mMutex.unlock();
      } else {
        std::unique_lock<std::mutex> lock(mEventMutex);
        mEvent.wait(lock);
      }
    }
  }

  return bResult;
}

bool FifoBuffer::write(AudioBuffer& audioBuf)
{
  bool bResult = audioBuf.getAudioFormat().equal( mFormat );

  if(  bResult ){
    mMutex.lock();
    {
      ByteBuffer extBuf = audioBuf.getRawBuffer();

      int newSize = extBuf.size() + mBuf.size();
      mBuf.reserve( newSize );

      std::copy( extBuf.begin(), extBuf.end(), std::back_inserter( mBuf ) );
    }
    mMutex.unlock();

    {
      std::lock_guard<std::mutex> lock(mEventMutex);
      mEvent.notify_all();
    }
  }

  return bResult;
}

int FifoBuffer::getBufferedSamples(void)
{
  return mBuf.size() / mFormat.getChannelsSampleByte();
}
