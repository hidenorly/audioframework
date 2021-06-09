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
#include <thread>

FifoBufferBase::FifoBufferBase(AudioFormat format):mFormat(format), mFifoSizeLimit(0), mReadBlocked(false), mUnlockReadBlock(false)
{

}

FifoBufferBase::~FifoBufferBase()
{

}

int FifoBufferBase::getBufferedSamples(void)
{
  return mBuf.size() ? ( mBuf.size() / mFormat.getChannelsSampleByte() ) : 0;
}

void FifoBufferBase::setFifoSizeLimit(int nSampleLimit)
{
  int nChannelSampleByte = mFormat.getChannelsSampleByte();
  mFifoSizeLimit = nChannelSampleByte ? (nSampleLimit * nChannelSampleByte) : nSampleLimit;
}

void FifoBufferBase::setAudioFormat( AudioFormat audioFormat )
{
  if( !audioFormat.equal( mFormat) ){
    unlock();
    int nChannelSampleByte = mFormat.getChannelsSampleByte();
    int nSamples = nChannelSampleByte ? mFifoSizeLimit / nChannelSampleByte : mFifoSizeLimit;
    mFormat = audioFormat;
    setFifoSizeLimit( nSamples );
    mBuf.clear();
  }
}

void FifoBufferBase::clearBuffer(void)
{
  mBuf.clear();
}


FifoBuffer::FifoBuffer(AudioFormat format):FifoBufferBase(format), mWriteBlocked(false), mUnlockWriteBlock(false)
{

}

FifoBuffer::~FifoBuffer()
{

}

bool FifoBuffer::read(IAudioBuffer& audioBuf)
{
  bool bResult = audioBuf.getAudioFormat().equal( mFormat );

  if( bResult ){
    ByteBuffer readBuffer = audioBuf.getRawBuffer();
    int size = readBuffer.size();

    std::atomic<bool> bReceived = false;
    while( !bReceived && !mUnlockReadBlock){
      if( mBuf.size() >= size ){
        mBufMutex.lock();
        {
          readBuffer = ByteBuffer( mBuf.begin(), mBuf.begin()+size );
          audioBuf.setRawBuffer( readBuffer );
          bReceived = true;
          if( mBuf.size() > size ){
            mBuf.erase( mBuf.begin(), mBuf.begin() + size );
          } else {
            mBuf.clear();
          }
        }
        mBufMutex.unlock();

        {
          std::lock_guard<std::mutex> lock(mWriteBlockEventMutex);
          mWriteBlockEvent.notify_all();
        }
      } else {
        if( !mReadBlocked && !mWriteBlocked ){
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

bool FifoBuffer::write(IAudioBuffer& audioBuf)
{
  bool bResult = audioBuf.getAudioFormat().equal( mFormat );

  if(  bResult ){
    ByteBuffer extBuf = audioBuf.getRawBuffer();
    int nSizeExtBuf = extBuf.size();
    std::atomic<bool> bSent = false;
    while( !bSent && !mUnlockWriteBlock){
      if( !mReadBlocked && !mWriteBlocked && mFifoSizeLimit && ( mFifoSizeLimit > mBuf.size() ) && ( (mBuf.size()+nSizeExtBuf) > mFifoSizeLimit ) ){
          mWriteBlocked = true;
          std::unique_lock<std::mutex> lock(mWriteBlockEventMutex);
          mWriteBlockEvent.wait(lock);
          mWriteBlocked = false;
      } else {
        mBufMutex.lock();
        {
          int newSize = nSizeExtBuf + mBuf.size();
          mBuf.reserve( newSize );

          std::copy( extBuf.begin(), extBuf.end(), std::back_inserter( mBuf ) );
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

void FifoBuffer::unlock(void)
{
  mUnlockReadBlock = true;
  mUnlockWriteBlock = true;
  mReadBlockEvent.notify_all();
  mWriteBlockEvent.notify_all();
  std::this_thread::sleep_for(std::chrono::microseconds(100));
  mUnlockReadBlock = false;
  mUnlockWriteBlock = false;
}
