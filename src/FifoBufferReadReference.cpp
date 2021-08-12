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

#include "FifoBufferReadReference.hpp"
#include <iterator>
#include <thread>
#include <cassert>

FifoBufferReadReference::FifoBufferReadReference(AudioFormat format) : FifoBufferBase(format)
{

}

FifoBufferReadReference::~FifoBufferReadReference()
{

}

bool FifoBufferReadReference::readReference(IAudioBuffer& audioBuf)
{
  bool bResult = audioBuf.getAudioFormat().equal( mFormat );

  if( bResult ){
    int size = audioBuf.getRawBufferSize();
    size = size ? size : ( mBuf.size() ? (mBuf.size() / 3) : 256 );

    std::atomic<bool> bReceived = false;
    while( !bReceived && !mUnlockReadBlock ){
      if( mBuf.size() >= size ){
        mBufMutex.lock();
        {
          ByteBuffer readBuffer = ByteBuffer( mBuf.begin(), mBuf.begin()+size );
          audioBuf.setRawBuffer( readBuffer );
          bReceived = true;
          if( mBuf.size() > size ){
            mBuf.erase( mBuf.begin(), mBuf.begin() + size );
          } else {
            mBuf.clear();
          }
        }
        mBufMutex.unlock();
      } else {
        if( !mReadBlocked ){
          mReadBlocked = true;
          std::unique_lock<std::mutex> lock(mReadBlockEventMutex);
          mReadBlockEvent.wait(lock);
          mReadBlocked = false;
        }
      }
    }
  }

  return bResult;
}

bool FifoBufferReadReference::write(IAudioBuffer& audioBuf)
{
  bool bResult = audioBuf.getAudioFormat().equal( mFormat );

  if(  bResult ){
    ByteBuffer& extBuf = audioBuf.getRawBuffer();
    int nSizeExtBuf = extBuf.size();
    std::atomic<bool> bSent = false;
    while( !bSent ){
      if( ( mFifoSizeLimit > mBuf.size() ) && ( ( mBuf.size() + nSizeExtBuf ) > mFifoSizeLimit ) ){
        // exceed FIFO buffer size limit then drop the exceeded buffer since this is read reference fifo buffer
        mBufMutex.lock();
        {
          int nExceedingSize = mBuf.size() + nSizeExtBuf - mFifoSizeLimit;
          mBuf.erase( mBuf.begin(), mBuf.begin() + nExceedingSize );
        }
        mBufMutex.unlock();
      } else {
        mBufMutex.lock();
        {
#if __USE_RESERVE__
          int newSize = nSizeExtBuf + mBuf.size();
          mBuf.reserve( newSize );
#endif // __USE_RESERVE__
#if __USE_COPY_WITH_BACKINSERTER__
          std::copy( extBuf.begin(), extBuf.end(), std::back_inserter( mBuf ) );
#endif // __USE_COPY_WITH_BACKINSERTER__
#if __USE_INSERT__
          mBuf.insert( mBuf.end(), extBuf.begin(), extBuf.end() );
#endif // __USE_INSERT__
          bSent = true;
        }
        mBufMutex.unlock();

        {
          std::lock_guard<std::mutex> lock(mReadBlockEventMutex);
          mReadBlockEvent.notify_all();
        }
      }
    }
  }

  return bResult;
}

void FifoBufferReadReference::unlock(void)
{
  mUnlockReadBlock = true;
  mReadBlockEvent.notify_all();
  std::this_thread::sleep_for(std::chrono::microseconds(100));
  mUnlockReadBlock = false;
}