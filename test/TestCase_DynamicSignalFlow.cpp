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
#include "TestCase_DynamicSignalFlow.hpp"

TestCase_DynamicSignal::TestCase_DynamicSignal()
{
}

TestCase_DynamicSignal::~TestCase_DynamicSignal()
{
}

void TestCase_DynamicSignal::SetUp()
{
}

void TestCase_DynamicSignal::TearDown()
{
}

TEST_F(TestCase_DynamicSignal, testAddNewPipeToPipeMixer)
{
  // Signal flow: adding filterduring pipe is running
  //   step1: Source -> Pipe(->FilterIncrement->) -> SinkAdaptor -> [PipeMixer ] -> Sink
  //
  //   step2: Source -> Pipe(->FilterIncrement->) -> SinkAdaptor -> [PipeMixer ] -> Sink
  //          Source -> Pipe(->FilterIncrement->) -> SinkAdaptor -> [(mix here)]
  std::unique_ptr<PipeMixer> pPipeMixer = std::make_unique<PipeMixer>();
  std::shared_ptr<ISink> pSink = std::make_shared<Sink>();
  pPipeMixer->attachSink( pSink );

  std::unique_ptr<IPipe> pStream1 = std::make_unique<Pipe>();
  std::shared_ptr<ISource> pSource1 = std::make_shared<Source>();
  pStream1->attachSource( pSource1 );
  pStream1->attachSink( pPipeMixer->allocateSinkAdaptor() );
  pStream1->addFilterToTail( std::make_shared<FilterIncrement>() );

  std::cout << "start:mixer & stream1" << std::endl;
  pStream1->run();
  pPipeMixer->run();
  std::cout << "started" << std::endl;
  std::this_thread::sleep_for(std::chrono::microseconds(1000));

  std::cout << "add:stream2 during stream1 is running" << std::endl;
  std::unique_ptr<IPipe> pStream2 = std::make_unique<Pipe>();
  std::shared_ptr<ISource> pSource2 = std::make_shared<Source>();
  pStream2->attachSource( pSource2 );
  std::shared_ptr<ISink> pSinkAdaptor2 = pPipeMixer->allocateSinkAdaptor();
  pStream2->attachSink( pSinkAdaptor2 );
  std::cout << "added:stream2 during stream1 is running" << std::endl;
  pStream2->addFilterToTail( std::make_shared<FilterIncrement>() );
  std::cout << "start:stream2 during stream1 is running" << std::endl;
  pStream2->run();
  std::cout << "started:stream2 during stream1 is running" << std::endl;
  std::this_thread::sleep_for(std::chrono::microseconds(1000));

  // SHOULD DETACH SINK ADAPTOR FROM PIPEMIXER FIRST WITHOUT DELETE WHILE THE PIPE IS RUNNING.
  std::cout << "release detached sink adaptor of stream2 during stream1 is running" << std::endl;
  pPipeMixer->releaseSinkAdaptor( pSinkAdaptor2 );
  std::cout << "released detached sink adaptor of stream2 during stream1 is running" << std::endl;

  std::cout << "stop:stream2 during stream1 is running" << std::endl;
  pStream2->stop();
  std::cout << "stopped:stream2 during stream1 is running" << std::endl;
  // finalize stream2, the source and the sink

  std::cout << "detach:sink adaptor from stream2 during stream1 is running" << std::endl;
  std::shared_ptr<ISink> pSink2 = pStream2->detachSink();
  EXPECT_EQ(pSinkAdaptor2, pSink2);
  pSink2 = nullptr; pSinkAdaptor2 = nullptr;

  std::cout << "detach:source of stream2 during stream1 is running" << std::endl;
  pSource2 = pStream2->detachSource();
  EXPECT_NE(nullptr, pSource2);
  std::cout << "detached:source of stream2 during stream1 is running" << std::endl;
  pSource2 = nullptr;
  std::cout << "clear:filters of stream2 during stream1 is running" << std::endl;
  pStream2->clearFilters();
  std::cout << "cleared:filters of stream2 during stream1 is running" << std::endl;
  std::cout << "wait is running" << std::endl;
  std::this_thread::sleep_for(std::chrono::microseconds(1000));
  std::cout << "waited during stream1 is running" << std::endl;

  std::cout << "stop all" << std::endl;

  pPipeMixer->stop();
  pStream1->stop();
  EXPECT_FALSE(pPipeMixer->isRunning());
  std::cout << "stopped" << std::endl;

  // finalize stream1, the source and the sink
  std::shared_ptr<ISink> pSink1 = pStream1->detachSink();
  EXPECT_NE(nullptr, pSink1);
  pPipeMixer->releaseSinkAdaptor( pSink1 );
  pSource1 = pStream1->detachSource();
  EXPECT_NE(nullptr, pSource1);
  pSource1 = nullptr;
  pStream1->clearFilters();

  // finalize pipemixer
  pSink = pPipeMixer->detachSink();
  EXPECT_NE(nullptr, pSink);
  pSink->dump();
}

TEST_F(TestCase_DynamicSignal, testAddNewFilter)
{
  // Signal flow: adding filterduring pipe is running
  //   step1: Source -> Pipe(->FilterIncrement->) -> Sink
  //   step2: Source -> Pipe(->FilterIncrement->FilterIncrement->) -> Sink
  //   step3: Source -> Pipe(->FilterIncrement->) -> Sink
  std::unique_ptr<IPipe> pStream = std::make_unique<Pipe>();
  std::shared_ptr<ISource> pSource = std::make_shared<Source>();
  pStream->attachSource( pSource );
  std::shared_ptr<ISink> pSink = std::make_shared<Sink>();
  pStream->attachSink( pSink );
  pStream->addFilterToTail( std::make_shared<FilterIncrement>() );

  std::cout << "start" << std::endl;
  pStream->run();
  std::cout << "started" << std::endl;
  std::this_thread::sleep_for(std::chrono::microseconds(1000));

  std::cout << "add:filter during stream is running" << std::endl;
  std::shared_ptr<IFilter> pFilter = std::make_shared<FilterIncrement>();
  pStream->addFilterToTail( pFilter );
  std::cout << "added:filter during stream is running" << std::endl;
  std::this_thread::sleep_for(std::chrono::microseconds(1000));
  std::cout << "remove:filter during stream is running" << std::endl;
  pStream->removeFilter( pFilter );
  std::cout << "removed:filter during stream is running" << std::endl;
  std::this_thread::sleep_for(std::chrono::microseconds(1000));

  std::cout << "stop" << std::endl;
  pStream->stop();
  std::cout << "stopped" << std::endl;

  std::shared_ptr<ISink> pDetachedSink = pStream->detachSink();
  EXPECT_EQ(pSink, pDetachedSink);
  pSink->dump();
  pSink = pDetachedSink = nullptr;

  std::shared_ptr<ISource> pDetachedSource = pStream->detachSource();
  EXPECT_EQ(pSource, pDetachedSource);
  pSource = pDetachedSource = nullptr;

  pStream->clearFilters();
}

TEST_F(TestCase_DynamicSignal, testAddNewFilter_PipeMultiThread)
{
  // Signal flow: adding filterduring pipe is running
  //   step1: Source -> PipeMultiThread(->FilterIncrement->) -> Sink
  //   step2: Source -> PipeMultiThread(->FilterIncrement->FilterIncrement->) -> Sink
  //   step3: Source -> PipeMultiThread(->FilterIncrement->) -> Sink
  std::unique_ptr<IPipe> pStream = std::make_unique<PipeMultiThread>();
  std::shared_ptr<ISource> pSource = std::make_shared<Source>();
  pStream->attachSource( pSource );
  std::shared_ptr<ISink> pSink = std::make_shared<Sink>();
  pStream->attachSink( pSink );
  pStream->addFilterToTail( std::make_shared<FilterIncrement>() );

  std::cout << "start" << std::endl;
  pStream->run();
  std::cout << "started" << std::endl;
  std::this_thread::sleep_for(std::chrono::microseconds(1000));

  std::cout << "add:filter during stream is running" << std::endl;
  std::shared_ptr<IFilter> pFilter = std::make_shared<FilterIncrement>();
  pStream->addFilterToTail( pFilter );
  std::cout << "added:filter during stream is running" << std::endl;
  std::this_thread::sleep_for(std::chrono::microseconds(1000));
  std::cout << "remove:filter during stream is running" << std::endl;
  pStream->removeFilter( pFilter );
  std::cout << "removed:filter during stream is running" << std::endl;
  std::this_thread::sleep_for(std::chrono::microseconds(1000));

  std::cout << "stop" << std::endl;
  pStream->stop();
  std::cout << "stopped" << std::endl;

  std::shared_ptr<ISink> pDetachedSink = pStream->detachSink();
  EXPECT_EQ(pSink, pDetachedSink);
  pSink->dump();

  std::shared_ptr<ISource> pDetachedSource = pStream->detachSource();
  EXPECT_EQ(pSource, pDetachedSource);

  pStream->clearFilters();
}

TEST_F(TestCase_DynamicSignal, testAddNewSinkToPipe)
{
  // Signal flow: switch sink during pipe is running
  //   before: Source -> Pipe(->FilterIncrement->) -> Sink1
  //   after:  Source -> Pipe(->FilterIncrement->) -> Sink2
  std::unique_ptr<IPipe> pPipe = std::make_unique<Pipe>();

  std::shared_ptr<ISource> pSource = std::make_shared<Source>();
  pPipe->attachSource( pSource );

  std::shared_ptr<ISink> pSink1 = std::make_shared<Sink>();
  pPipe->attachSink( pSink1 );

  pPipe->addFilterToTail( std::make_shared<FilterIncrement>() );

  std::cout << "start" << std::endl;
  pPipe->run();
  std::this_thread::sleep_for(std::chrono::microseconds(1000));
  std::cout << "started" << std::endl;

  std::shared_ptr<ISink> pSink2 = std::make_shared<Sink>();
  std::cout << "attach new Sink to pipe during the pipe is running" << std::endl;
  std::shared_ptr<ISink> pDetachedSink1 = pPipe->attachSink( pSink2 );
  std::cout << "attached new Sink to pipe during the pipe is running" << std::endl;
  EXPECT_EQ( pDetachedSink1, pSink1 );
  pDetachedSink1->dump();
  std::this_thread::sleep_for(std::chrono::microseconds(1000));

  std::cout << "stop" << std::endl;
  pPipe->stop();
  std::cout << "stopped" << std::endl;

  std::shared_ptr<ISink> pDetachedSink2 = pPipe->detachSink();
  EXPECT_EQ( pDetachedSink2, pSink2 );
  pDetachedSink2->dump();

  std::shared_ptr<ISource> pDetachedSource = pPipe->detachSource();
  EXPECT_EQ( pDetachedSource, pSource );
}

TEST_F(TestCase_DynamicSignal, testAddNewSinkToPipe_PipeMultiThread)
{
  // Signal flow: switch sink during pipe is running
  //   before: Source -> PipeMultiThread(->FilterIncrement->) -> Sink1
  //   after:  Source -> PipeMultiThread(->FilterIncrement->) -> Sink2
  std::unique_ptr<IPipe> pPipe = std::make_unique<PipeMultiThread>();

  std::shared_ptr<ISource> pSource = std::make_shared<Source>();
  pPipe->attachSource( pSource );

  std::shared_ptr<ISink> pSink1 = std::make_shared<Sink>();
  pPipe->attachSink( pSink1 );

  pPipe->addFilterToTail( std::make_shared<FilterIncrement>() );

  std::cout << "start" << std::endl;
  pPipe->run();
  std::this_thread::sleep_for(std::chrono::microseconds(1000));
  std::cout << "started" << std::endl;

  std::shared_ptr<ISink> pSink2 = std::make_shared<Sink>();
  std::cout << "attach new Sink to pipe during the pipe is running" << std::endl;
  std::shared_ptr<ISink> pDetachedSink1 = pPipe->attachSink( pSink2 );
  std::cout << "attached new Sink to pipe during the pipe is running" << std::endl;
  EXPECT_EQ( pDetachedSink1, pSink1 );
  pDetachedSink1->dump();
  std::this_thread::sleep_for(std::chrono::microseconds(1000));

  std::cout << "stop" << std::endl;
  pPipe->stop();
  std::cout << "stopped" << std::endl;

  std::shared_ptr<ISink> pDetachedSink2 = pPipe->detachSink();
  EXPECT_EQ( pDetachedSink2, pSink2 );
  pDetachedSink2->dump();

  std::shared_ptr<ISource> pDetachedSource = pPipe->detachSource();
  EXPECT_EQ( pDetachedSource, pSource );
}


TEST_F(TestCase_DynamicSignal, testAddNewSourceToPipe)
{
  // Signal flow
  //   before: Source1 -> Pipe(->FilterIncrement->) -> Sink
  //   after:  Source2 -> Pipe(->FilterIncrement->) -> Sink
  std::unique_ptr<IPipe> pPipe = std::make_unique<Pipe>();

  std::shared_ptr<ISource> pSource1 = std::make_shared<Source>();
  pPipe->attachSource( pSource1 );

  std::shared_ptr<ISink> pSink = std::make_shared<Sink>();
  pPipe->attachSink( pSink );

  pPipe->addFilterToTail( std::make_shared<FilterIncrement>() );

  std::cout << "start" << std::endl;
  pPipe->run();
  std::this_thread::sleep_for(std::chrono::microseconds(1000));
  std::cout << "started" << std::endl;

  std::shared_ptr<ISource> pSource2 = std::make_shared<Source>();
  std::cout << "attach new Source to pipe during the pipe is running" << std::endl;
  std::shared_ptr<ISource> pDetachedSource1 = pPipe->attachSource( pSource2 );
  std::cout << "attached new Source to pipe during the pipe is running" << std::endl;
  EXPECT_EQ( pDetachedSource1, pSource1 );
  std::this_thread::sleep_for(std::chrono::microseconds(1000));

  std::cout << "stop" << std::endl;
  pPipe->stop();
  std::cout << "stopped" << std::endl;

  std::shared_ptr<ISink> pDetachedSink = pPipe->detachSink();
  EXPECT_EQ( pDetachedSink, pSink );
  pDetachedSink->dump();

  std::shared_ptr<ISource> pDetachedSource2 = pPipe->detachSource();
  EXPECT_EQ( pDetachedSource2, pSource2 );
}

TEST_F(TestCase_DynamicSignal, testAddNewSourceToPipe_PipeMultiThread)
{
  // Signal flow
  //   before: Source1 -> PipeMultiThread(->FilterIncrement->) -> Sink
  //   after:  Source2 -> PipeMultiThread(->FilterIncrement->) -> Sink
  std::unique_ptr<IPipe> pPipe = std::make_unique<PipeMultiThread>();

  std::shared_ptr<ISource> pSource1 = std::make_shared<Source>();
  pPipe->attachSource( pSource1 );

  std::shared_ptr<ISink> pSink = std::make_shared<Sink>();
  pPipe->attachSink( pSink );

  pPipe->addFilterToTail( std::make_shared<FilterIncrement>() );

  std::cout << "start" << std::endl;
  pPipe->run();
  std::this_thread::sleep_for(std::chrono::microseconds(1000));
  std::cout << "started" << std::endl;

  std::shared_ptr<ISource> pSource2 = std::make_shared<Source>();
  std::cout << "attach new Source to pipe during the pipe is running" << std::endl;
  std::shared_ptr<ISource> pDetachedSource1 = pPipe->attachSource( pSource2 );
  std::cout << "attached new Source to pipe during the pipe is running" << std::endl;
  EXPECT_EQ( pDetachedSource1, pSource1 );
  std::this_thread::sleep_for(std::chrono::microseconds(1000));

  std::cout << "stop" << std::endl;
  pPipe->stop();
  std::cout << "stopped" << std::endl;

  std::shared_ptr<ISink> pDetachedSink = pPipe->detachSink();
  EXPECT_EQ( pDetachedSink, pSink );
  pDetachedSink->dump();

  std::shared_ptr<ISource> pDetachedSource2 = pPipe->detachSource();
  EXPECT_EQ( pDetachedSource2, pSource2 );
}

TEST_F(TestCase_DynamicSignal, testAddNewSinkToReferenceSoundSink)
{
  class TestSink : public Sink
  {
    int mTestLatency;
  public:
    TestSink(int latencyUsec): Sink(), mTestLatency(latencyUsec){};
    virtual ~TestSink(){};
    virtual int getLatencyUSec(void){ return mTestLatency; };
  };

  class TestSource : public Source
  {
    int mTestLatency;
  public:
    TestSource(int latencyUsec): Source(), mTestLatency(latencyUsec){};
    virtual ~TestSource(){};
    virtual int getLatencyUSec(void){ return mTestLatency; };
  };

  std::unique_ptr<IPipe> pPipe = std::make_unique<Pipe>();

  std::shared_ptr<ISink> pGlobalSink1 = std::make_shared<Sink>();
  std::shared_ptr<ReferenceSoundSinkSource> pReferenceSource = std::make_shared<ReferenceSoundSinkSource>( pGlobalSink1 );
  std::shared_ptr<ISource> pRawMicSource = std::make_shared<TestSource>( 5*1000 );
  std::shared_ptr<AccousticEchoCancelledSource> pAecedMicSource = std::make_shared<AccousticEchoCancelledSource>( pRawMicSource, pReferenceSource );

  pPipe->attachSource( pAecedMicSource );

  pPipe->addFilterToTail( std::make_shared<Filter>() );

  std::shared_ptr<ISink> pMicSink = std::make_shared<Sink>();
  pPipe->attachSink( pMicSink );

  pPipe->run();
  std::this_thread::sleep_for(std::chrono::microseconds(1000));

  std::shared_ptr<ISink> pGlobalSink2 = std::make_shared<TestSink>(1*1000);
  std::shared_ptr<ISink> pDetachedSinkFromRefSound1 = pReferenceSource->attachSink(pGlobalSink2);
  EXPECT_EQ( pDetachedSinkFromRefSound1, pGlobalSink1 );
  pAecedMicSource->adjustDelay();
  std::this_thread::sleep_for(std::chrono::microseconds(5000));

  pPipe->stop();
  pMicSink->dump();

  std::shared_ptr<ISink> pDetachedMicSink = pPipe->detachSink();
  EXPECT_EQ( pDetachedMicSink, pMicSink );

  std::shared_ptr<ISource> pDetachedMicSource = pPipe->detachSource();
  EXPECT_EQ( pDetachedMicSource, pAecedMicSource );

  std::shared_ptr<ISink> pDetachedSinkFromRefSound2 = pReferenceSource->detachSink();
  EXPECT_EQ( pDetachedSinkFromRefSound2, pGlobalSink2 );
}
