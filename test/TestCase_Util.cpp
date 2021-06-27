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
