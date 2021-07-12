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

#include "TestCase_Common.hpp"
#include "TestCase_Util.hpp"
#include <thread>


TestCase_Util::TestCase_Util()
{
}

TestCase_Util::~TestCase_Util()
{
}

void TestCase_Util::SetUp()
{
}

void TestCase_Util::TearDown()
{
}

TEST_F(TestCase_Util, testStringTokenizer)
{
  std::string target1 = "data1:data2:data3:data4";
  StringTokenizer tok1(target1, ":");

  std::cout << "#1 getNext()" << std::endl;
  EXPECT_TRUE( tok1.hasNext() );
  EXPECT_EQ( tok1.getNext(), "data1" );

  std::cout << "#2 getNext()" << std::endl;
  EXPECT_TRUE( tok1.hasNext() );
  EXPECT_EQ( tok1.getNext(), "data2" );

  std::cout << "#3 getNext()" << std::endl;
  EXPECT_TRUE( tok1.hasNext() );
  EXPECT_EQ( tok1.getNext(), "data3" );

  std::cout << "#4 getNext()" << std::endl;
  EXPECT_TRUE( tok1.hasNext() );
  EXPECT_EQ( tok1.getNext(), "data4" );

  std::cout << "#5 getNext()" << std::endl;
  EXPECT_FALSE( tok1.hasNext() );

  std::string target2 = "data1:data2:data3:data4";
  StringTokenizer tok2(target1, ",");
  EXPECT_TRUE( tok2.hasNext() );
  EXPECT_EQ( tok2.getNext(), target2 );

  std::string target3 = "data1 : data2 : data3 : data4";
  StringTokenizer tok3(target3, " : ");

  std::cout << "#1 getNext()" << std::endl;
  EXPECT_TRUE( tok3.hasNext() );
  EXPECT_EQ( tok3.getNext(), "data1" );

  std::cout << "#2 getNext()" << std::endl;
  EXPECT_TRUE( tok3.hasNext() );
  EXPECT_EQ( tok3.getNext(), "data2" );

  std::cout << "#3 getNext()" << std::endl;
  EXPECT_TRUE( tok3.hasNext() );
  EXPECT_EQ( tok3.getNext(), "data3" );

  std::cout << "#4 getNext()" << std::endl;
  EXPECT_TRUE( tok3.hasNext() );
  EXPECT_EQ( tok3.getNext(), "data4" );

  std::cout << "#5 getNext()" << std::endl;
  EXPECT_FALSE( tok3.hasNext() );
}

TEST_F(TestCase_Util, testFifoBuffer)
{
  AudioFormat defaultFormat;
  FifoBuffer fifoBuf( defaultFormat );
  int nSize = 256;
  AudioBuffer readBuf( defaultFormat, nSize );
  AudioBuffer writeBuf( defaultFormat, nSize );

  EXPECT_TRUE( fifoBuf.write( writeBuf ) );
  EXPECT_EQ( fifoBuf.getBufferedSamples(), nSize );

  EXPECT_TRUE( fifoBuf.write( writeBuf ) );
  EXPECT_EQ( fifoBuf.getBufferedSamples(), nSize*2 );

  EXPECT_TRUE( fifoBuf.read( readBuf ) );
  EXPECT_EQ( fifoBuf.getBufferedSamples(), nSize );
  Util::dumpBuffer("readBuf:", readBuf);

  EXPECT_TRUE( fifoBuf.read( readBuf ) );
  EXPECT_EQ( fifoBuf.getBufferedSamples(), 0 );
  Util::dumpBuffer("readBuf:", readBuf);

  std::atomic<bool> bResult = false;
  std::thread thx([&]{ bResult = fifoBuf.read( readBuf );});
  std::this_thread::sleep_for(std::chrono::microseconds(1000));
  EXPECT_TRUE( fifoBuf.write( writeBuf ) );
  thx.join();
  EXPECT_TRUE( bResult );
}

TEST_F(TestCase_Util, testThreadBase)
{
  class MyThread : public ThreadBase
  {
  public:
    virtual void process(void)
    {
      std::this_thread::sleep_for(std::chrono::microseconds(1000));
    }
  };

  class MyRunningStatusListener : public ThreadBase::RunnerListener
  {
  public:
    bool bIsRunning;
    MyRunningStatusListener():bIsRunning(false){};
    ~MyRunningStatusListener(){};

    virtual void onRunnerStatusChanged(bool bRunning){
      bIsRunning = bRunning;
    };
  };

  std::shared_ptr<MyThread> pRunner = std::make_shared<MyThread>();
  std::shared_ptr<MyRunningStatusListener> pListenr = std::make_shared<MyRunningStatusListener>();
  pRunner->registerRunnerStatusListener( pListenr );
  EXPECT_FALSE( pListenr->bIsRunning );
  pRunner->run();
  std::this_thread::sleep_for(std::chrono::microseconds(1000));
  EXPECT_TRUE( pListenr->bIsRunning );
  pRunner->stop();
  EXPECT_FALSE( pListenr->bIsRunning );
}

TEST_F(TestCase_Util, testPcmEncodingConversion)
{
  int nSamples = 256;

  AudioBuffer srcBuf( AudioFormat(AudioFormat::ENCODING::PCM_16BIT), nSamples );
  uint16_t* pRawSrcBuf = reinterpret_cast<uint16_t*>( srcBuf.getRawBufferPointer() );
  for(int i=0, c=srcBuf.getNumberOfSamples()*srcBuf.getAudioFormat().getNumberOfChannels(); i<c; i++){
    *(pRawSrcBuf+i) = i;
  }
  // convert 16->32
  AudioBuffer dstBuf32( AudioFormat(AudioFormat::ENCODING::PCM_32BIT), nSamples );
  EXPECT_TRUE( AudioFormatAdaptor::convert( srcBuf, dstBuf32 ) );
  uint32_t* pDstBuf32 = reinterpret_cast<uint32_t*>( dstBuf32.getRawBufferPointer() );
  for(int i=0, c=srcBuf.getNumberOfSamples()*srcBuf.getAudioFormat().getNumberOfChannels(); i<c; i++){
    EXPECT_EQ( *(pRawSrcBuf+i), (*(pDstBuf32+i) >> 16) );
  }
  // convert 16->8
  AudioBuffer dstBuf8( AudioFormat(AudioFormat::ENCODING::PCM_8BIT), nSamples );
  EXPECT_TRUE( AudioFormatAdaptor::convert( srcBuf, dstBuf8 ) );
  uint8_t* pDstBuf8 = reinterpret_cast<uint8_t*>( dstBuf8.getRawBufferPointer() );
  for(int i=0, c=srcBuf.getNumberOfSamples()*srcBuf.getAudioFormat().getNumberOfChannels(); i<c; i++){
    EXPECT_EQ( (*(pRawSrcBuf+i) >> 8), *(pDstBuf8+i) );
  }
/*
  // convert 16->24
  AudioBuffer dstBuf24( AudioFormat(AudioFormat::ENCODING::PCM_24BIT_PACKED), nSamples );
  EXPECT_TRUE( AudioFormatAdaptor::convert( srcBuf, dstBuf24 ) );
  uint8_t* pDstBuf24 = reinterpret_cast<uint8_t*>( dstBuf24.getRawBufferPointer() );
  for(int i=0, c=srcBuf.getNumberOfSamples()*srcBuf.getAudioFormat().getNumberOfChannels(); i<c; i++){
    uint32_t aSrc = (uint32_t)(((uint16_t)*(pRawSrcBuf+i)) << 8);
    uint32_t aDst = (((uint32_t)((uint8_t)*(pDstBuf24+i*3+0))) << 0) +
      (((uint32_t)((uint8_t)*(pDstBuf24+i*3+1))) << 8) +
      (((uint32_t)((uint8_t)*(pDstBuf24+i*3+2))) << 16);
    std::cout << std::hex;
    std::cout << "i:" << std::to_string(i) << " aSrc:" << aSrc << " aDst:" << aDst;
    std::cout << " rawDst:" << (uint32_t)*(pDstBuf24+i*3) << " " <<  (uint32_t)*(pDstBuf24+i*3+1) << " " <<  (uint32_t)*(pDstBuf24+i*3+2) << std::endl;
    EXPECT_EQ( (*(pRawSrcBuf+i) >> 8), *(pDstBuf8+i) );
  }
*/
}

TEST_F(TestCase_Util, testPcmSamplingRateConversion)
{
  int nSamples = 256;

  AudioBuffer srcBuf( AudioFormat(AudioFormat::ENCODING::PCM_16BIT, AudioFormat::SAMPLING_RATE::SAMPLING_RATE_48_KHZ), nSamples );
  uint16_t* pRawSrcBuf = reinterpret_cast<uint16_t*>( srcBuf.getRawBufferPointer() );
  for(int i=0, c=srcBuf.getNumberOfSamples()*srcBuf.getAudioFormat().getNumberOfChannels(); i<c; i++){
    *(pRawSrcBuf+i) = i;
  }
  // convert 48->44.1 : down size case
  AudioBuffer dstBuf( AudioFormat(AudioFormat::ENCODING::PCM_16BIT, AudioFormat::SAMPLING_RATE::SAMPLING_RATE_44_1_KHZ), nSamples );
  EXPECT_TRUE( AudioFormatAdaptor::convert( srcBuf, dstBuf ) );
  EXPECT_EQ( (int)dstBuf.getNumberOfSamples(), (int)((float)srcBuf.getNumberOfSamples()*44.1f/48.0f+0.99f) );

  // convert 48->96 : up size case
  AudioBuffer dstBuf96( AudioFormat(AudioFormat::ENCODING::PCM_16BIT, AudioFormat::SAMPLING_RATE::SAMPLING_RATE_96_KHZ), nSamples );
  EXPECT_TRUE( AudioFormatAdaptor::convert( srcBuf, dstBuf96 ) );
  EXPECT_EQ( (int)dstBuf96.getNumberOfSamples(), (int)((float)srcBuf.getNumberOfSamples()*96.0f/48.0f+0.99f) );
}

TEST_F(TestCase_Util, testPcmChannelConversion)
{
  int nSamples = 256;
  AudioBuffer srcBuf( AudioFormat(AudioFormat::ENCODING::PCM_16BIT, AudioFormat::SAMPLING_RATE::SAMPLING_RATE_48_KHZ, AudioFormat::CHANNEL::CHANNEL_STEREO), nSamples );
  uint16_t* pRawSrcBuf = reinterpret_cast<uint16_t*>( srcBuf.getRawBufferPointer() );
  for(int i=0, c=srcBuf.getNumberOfSamples()*srcBuf.getAudioFormat().getNumberOfChannels(); i<c; i++){
    *(pRawSrcBuf+i) = i;
  }

  // stereo -> mono : down case
  AudioBuffer dstBuf( AudioFormat(AudioFormat::ENCODING::PCM_16BIT, AudioFormat::SAMPLING_RATE::SAMPLING_RATE_48_KHZ, AudioFormat::CHANNEL::CHANNEL_MONO), nSamples );
  EXPECT_TRUE( AudioFormatAdaptor::convert( srcBuf, dstBuf ) );
  uint16_t* pDstBuf = reinterpret_cast<uint16_t*>( dstBuf.getRawBufferPointer() );

  int nSrcChannels = srcBuf.getAudioFormat().getNumberOfChannels();
  int nDstChannels = dstBuf.getAudioFormat().getNumberOfChannels();
#if !defined(USE_TINY_CC_IMPL) || USE_TINY_CC_IMPL
  for(int i=0, c=srcBuf.getNumberOfSamples(); i<c; i++){
    EXPECT_EQ( *(pRawSrcBuf+(int)(i*nSrcChannels+1)), *(pDstBuf+(int)(i*nDstChannels)) ); // R is used in the mono as current tentative impl. but the replaced impl. should be Ok not to follow.
  }
#endif /* USE_TINY_CC_IMPL */

  // stereo -> 4ch : up case
  AudioBuffer dstBuf4( AudioFormat(AudioFormat::ENCODING::PCM_16BIT, AudioFormat::SAMPLING_RATE::SAMPLING_RATE_48_KHZ, AudioFormat::CHANNEL::CHANNEL_4CH), nSamples );
  EXPECT_TRUE( AudioFormatAdaptor::convert( srcBuf, dstBuf4 ) );
  Util::dumpBuffer(dstBuf4);
  pDstBuf = reinterpret_cast<uint16_t*>( dstBuf4.getRawBufferPointer() );
  nSrcChannels = srcBuf.getAudioFormat().getNumberOfChannels();
  nDstChannels = dstBuf4.getAudioFormat().getNumberOfChannels();
  for(int i=0, ii=srcBuf.getNumberOfSamples(); i<ii; i++){
    for(int j=0; j<nSrcChannels; j++){
      EXPECT_EQ( *(pRawSrcBuf+(int)(i*nSrcChannels+j)), *(pDstBuf+(int)(i*nDstChannels+j)) );
    }
  }
}