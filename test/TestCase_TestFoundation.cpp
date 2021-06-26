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

#include <gtest/gtest.h>

#include "TestCase_TestFoundation.hpp"

#include "Pipe.hpp"
#include "Filter.hpp"
#include "Source.hpp"
#include "Sink.hpp"
#include "AudioFormat.hpp"
#include "FilterExample.hpp"
#include "Testability.hpp"
#include "Util.hpp"

#include <iostream>
#include <filesystem>
#include <chrono>
#include <memory>

TestCase_TestFoundation::TestCase_TestFoundation()
{
}

TestCase_TestFoundation::~TestCase_TestFoundation()
{
}

void TestCase_TestFoundation::SetUp()
{
}

void TestCase_TestFoundation::TearDown()
{
}

TEST_F(TestCase_TestFoundation, testSinkCapture)
{
  // Signal flow
  //   Source -> Pipe(->FilterIncrement->) -> SinkCapture ------> Sink
  //                                              +--(FifoRef)--> captureRead
  std::shared_ptr<ISource> pSource = std::make_shared<Source>();
  std::shared_ptr<ISink> pSink = std::dynamic_pointer_cast<ISink>( std::make_shared<SinkCapture>( std::make_shared<Sink>() ) );
  std::shared_ptr<ICapture> pCapture = std::dynamic_pointer_cast<ICapture>(pSink);
  std::unique_ptr<IPipe> pPipe = std::make_unique<Pipe>();

  pPipe->attachSource( pSource );
  pPipe->addFilterToTail( std::make_shared<FilterIncrement>() );
  pPipe->attachSink( pSink );

  pPipe->run();
  std::this_thread::sleep_for(std::chrono::microseconds(1000));
  AudioBuffer captureBuf( AudioFormat(), 240 );
  std::cout << "captureRead" << std::endl;
  pCapture->captureRead( captureBuf );
  pPipe->stop();
  Util::dumpBuffer( "SinkCapture result", captureBuf );

  EXPECT_EQ( pSink, pPipe->detachSink());
  EXPECT_EQ( pSource, pPipe->detachSource());
  std::cout << "Sink dump:" << std::endl;
  pSink->dump();
  pPipe->clearFilters();
  pCapture->unlock();
}

TEST_F(TestCase_TestFoundation, testSinkInjector)
{
  // Signal flow
  //   Source -> Pipe(->FilterIncrement->) -> SinkInjector ------> Sink
  //                                inject -------^
  std::shared_ptr<ISource> pSource = std::make_shared<Source>();
  std::shared_ptr<ISink> pSink = std::dynamic_pointer_cast<ISink>( std::make_shared<SinkInjector>( std::make_shared<Sink>() ) );
  std::shared_ptr<IInjector> pInjector = std::dynamic_pointer_cast<IInjector>(pSink);
  std::unique_ptr<IPipe> pPipe = std::make_unique<Pipe>();

  pPipe->attachSource( pSource );
  pPipe->addFilterToTail( std::make_shared<Filter>() );
  pPipe->attachSink( pSink );

  // 1st step: non injected
  EXPECT_FALSE( pInjector->getInjectorEnabled() );
  pPipe->run();
  std::this_thread::sleep_for(std::chrono::microseconds(100));
  pPipe->stop();
  std::cout << "Sink dump(non injected):" << std::endl;
  pSink->dump();

  // 2nd step: injected
  AudioBuffer injectBuf( AudioFormat(), 240 );
  ByteBuffer rawInjectBuf = injectBuf.getRawBuffer();
  for(int i=0; i<rawInjectBuf.size(); i=i+2){
    rawInjectBuf[i] = i % 16;
    rawInjectBuf[i+1] = 0;
  }
  injectBuf.setRawBuffer( rawInjectBuf );
  Util::dumpBuffer("injectBuf", injectBuf);

  pInjector->setInjectorEnabled( true );
  EXPECT_TRUE( pInjector->getInjectorEnabled() );

  pPipe->run();
  {
    std::chrono::milliseconds start = duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch());
    bool bRunning = true;
    while( bRunning ){
      pInjector->inject( injectBuf );
      std::chrono::milliseconds now = duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch());
      bRunning = ( ( now.count() - start.count() ) <= 2 ) ? true : false; // run 2msec
    };
  }
  pPipe->stop();

  EXPECT_EQ( pSink, pPipe->detachSink());
  EXPECT_EQ( pSource, pPipe->detachSource());
  std::cout << "Sink dump(injected result):" << std::endl;
  pSink->dump();
  pPipe->clearFilters();
  pInjector->unlock();
}


TEST_F(TestCase_TestFoundation, testSourceCapture)
{
  // Signal flow
  //   SourceCapture -> Pipe(->FilterIncrement->) -> Sink
  //     +-(FifoRef)--> captureRead
  std::shared_ptr<ISource> pSource = std::dynamic_pointer_cast<ISource>( std::make_shared<SourceCapture>( std::make_shared<Source>() ) );
  std::shared_ptr<ISink> pSink = std::make_shared<Sink>();
  std::shared_ptr<ICapture> pCapture = std::dynamic_pointer_cast<ICapture>(pSource);
  std::unique_ptr<IPipe> pPipe = std::make_unique<Pipe>();

  pPipe->attachSource( pSource );
  pPipe->addFilterToTail( std::make_shared<FilterIncrement>() );
  pPipe->attachSink( pSink );

  pPipe->run();
  std::this_thread::sleep_for(std::chrono::microseconds(1000));
  AudioBuffer captureBuf( AudioFormat(), 240 );
  std::cout << "captureRead" << std::endl;
  pCapture->captureRead( captureBuf );
  pPipe->stop();
  Util::dumpBuffer( "SourceCapture result", captureBuf );

  EXPECT_EQ( pSink, pPipe->detachSink());
  EXPECT_EQ( pSource, pPipe->detachSource());
  std::cout << "Sink dump:" << std::endl;
  pSink->dump();
  pPipe->clearFilters();
  pCapture->unlock();
}

TEST_F(TestCase_TestFoundation, testSourceInjector)
{
  // Signal flow
  //        Source -> Pipe(->FilterIncrement->) -> Sink
  // inject --^
  std::shared_ptr<ISource> pSource = std::dynamic_pointer_cast<ISource>( std::make_shared<SourceInjector>( std::make_shared<Source>() ) );
  std::shared_ptr<ISink> pSink = std::make_shared<Sink>();
  std::shared_ptr<IInjector> pInjector = std::dynamic_pointer_cast<IInjector>(pSource);
  std::unique_ptr<IPipe> pPipe = std::make_unique<Pipe>();

  pPipe->attachSource( pSource );
  pPipe->addFilterToTail( std::make_shared<Filter>() );
  pPipe->attachSink( pSink );

  // 1st step: non injected
  EXPECT_FALSE( pInjector->getInjectorEnabled() );
  pPipe->run();
  std::this_thread::sleep_for(std::chrono::microseconds(100));
  pPipe->stop();
  std::cout << "Sink dump(non injected):" << std::endl;
  pSink->dump();
  EXPECT_EQ( pSink, pPipe->detachSink());
  pSink = std::make_shared<Sink>();
  pPipe->attachSink( pSink );

  // 2nd step: injected
  AudioBuffer injectBuf( AudioFormat(), 240 );
  ByteBuffer rawInjectBuf = injectBuf.getRawBuffer();
  for(int i=0; i<rawInjectBuf.size(); i=i+2){
    rawInjectBuf[i] = i % 16;
    rawInjectBuf[i+1] = 0;
  }
  injectBuf.setRawBuffer( rawInjectBuf );
  Util::dumpBuffer("injectBuf", injectBuf);

  pInjector->setInjectorEnabled( true );
  EXPECT_TRUE( pInjector->getInjectorEnabled() );

  pPipe->run();
  {
    std::chrono::milliseconds start = duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch());
    bool bRunning = true;
    while( bRunning ){
      pInjector->inject( injectBuf );
      std::chrono::milliseconds now = duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch());
      bRunning = ( ( now.count() - start.count() ) <= 2 ) ? true : false; // run 2msec
    };
  }
  pPipe->stop();

  EXPECT_EQ( pSink, pPipe->detachSink());
  EXPECT_EQ( pSource, pPipe->detachSource());
  std::cout << "Sink dump(injected result):" << std::endl;
  pSink->dump();
  pPipe->clearFilters();
  pInjector->unlock();
}


TEST_F(TestCase_TestFoundation, testFilterCapture)
{
  // Signal flow
  //   Source -> Pipe(->FilterIncrement->FilterCapture->) -> Sink
  //                                          +-(FifoRef)--> captureRead
  std::shared_ptr<ISource> pSource = std::make_shared<Source>();
  std::shared_ptr<ISink> pSink = std::make_shared<Sink>();
  std::unique_ptr<IPipe> pPipe = std::make_unique<Pipe>();

  pPipe->attachSource( pSource );
  pPipe->attachSink( pSink );

  pPipe->addFilterToTail( std::make_shared<FilterIncrement>() );
  std::shared_ptr<IFilter> pFilter = std::make_shared<FilterCapture>();
  pPipe->addFilterToTail( pFilter );
  std::shared_ptr<ICapture> pCapture = std::dynamic_pointer_cast<ICapture>(pFilter);

  pPipe->run();
  std::this_thread::sleep_for(std::chrono::microseconds(1000));
  AudioBuffer captureBuf( AudioFormat(), 240 );
  std::cout << "captureRead" << std::endl;
  pCapture->captureRead( captureBuf );
  pPipe->stop();
  Util::dumpBuffer( "FilterCapture result", captureBuf );

  EXPECT_EQ( pSink, pPipe->detachSink());
  EXPECT_EQ( pSource, pPipe->detachSource());
  std::cout << "Sink dump:" << std::endl;
  pSink->dump();
  pPipe->clearFilters();
  pCapture->unlock();
}


TEST_F(TestCase_TestFoundation, testFilterInjector)
{
  // Signal flow
  //   Source -> Pipe(->FilterIncrement->FilterInjector->) -> Sink
  //                                    inject --^
  std::shared_ptr<ISource> pSource = std::make_shared<Source>();
  std::shared_ptr<ISink> pSink = std::make_shared<Sink>();
  std::unique_ptr<IPipe> pPipe = std::make_unique<Pipe>();

  pPipe->attachSource( pSource );
  pPipe->attachSink( pSink );

  pPipe->addFilterToTail( std::make_shared<FilterIncrement>() );
  std::shared_ptr<IFilter> pFilter = std::make_shared<FilterInjector>();
  pPipe->addFilterToTail( pFilter );
  std::shared_ptr<IInjector> pInjector = std::dynamic_pointer_cast<IInjector>(pFilter);

  // 1st step: non injected
  EXPECT_FALSE( pInjector->getInjectorEnabled() );
  pPipe->run();
  std::this_thread::sleep_for(std::chrono::microseconds(100));
  pPipe->stop();
  std::cout << "Sink dump(non injected):" << std::endl;
  pSink->dump();
  EXPECT_EQ( pSink, pPipe->detachSink());
  pSink = std::make_shared<Sink>();
  pPipe->attachSink( pSink );

  // 2nd step: injected
  AudioBuffer injectBuf( AudioFormat(), 240 );
  ByteBuffer rawInjectBuf = injectBuf.getRawBuffer();
  for(int i=0; i<rawInjectBuf.size(); i=i+2){
    rawInjectBuf[i] = i % 16;
    rawInjectBuf[i+1] = 0;
  }
  injectBuf.setRawBuffer( rawInjectBuf );
  Util::dumpBuffer("injectBuf", injectBuf);

  pInjector->setInjectorEnabled( true );
  EXPECT_TRUE( pInjector->getInjectorEnabled() );

  pPipe->run();
  {
    std::chrono::milliseconds start = duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch());
    bool bRunning = true;
    while( bRunning ){
      pInjector->inject( injectBuf );
      std::chrono::milliseconds now = duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch());
      bRunning = ( ( now.count() - start.count() ) <= 2 ) ? true : false; // run 2msec
    };
  }
  pPipe->stop();

  EXPECT_EQ( pSink, pPipe->detachSink());
  EXPECT_EQ( pSource, pPipe->detachSource());
  std::cout << "Sink dump:" << std::endl;
  pSink->dump();
  pPipe->clearFilters();
  pInjector->unlock();
}
