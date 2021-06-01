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

#ifndef __TESTCASE_PIPEANDFILTER_HPP__
#define __TESTCASE_PIPEANDFILTER_HPP__

#include <gtest/gtest.h>

class TestCase_PipeAndFilter : public ::testing::Test
{
protected:
  TestCase_PipeAndFilter();
  virtual ~TestCase_PipeAndFilter();
  virtual void SetUp();
  virtual void TearDown();

  void testAddFilters(void);
  void testAttachSourceSinkToPipe(void);

  void testFifoBuffer(void);
  void testInterPipeBridge(void);

  void testPipeMultiThread(void);
  void testMultipleSink(void);
  void testStreamSink(void);
  void testStreamSink_DifferentFormat(void);
  void testStreamSource(void);
  void testPipedSink(void);
  void testPipeMixer(void);

  void testDecoder(void);
  void testPlayer(void);
  void testEncoder(void);

  void testParameterManager(void);
  void testStringTokenizer(void);

  void testPlugInManager(void);
  void testFilterPlugInManager(void);
  void testSourcePlugInManager(void);
  void testSinkPlugInManager(void);

  void testDelayFilter(void);

  void testSinkCapture(void);
  void testSinkInjector(void);
  void testSourceCapture(void);
  void testSourceInjector(void);
  void testFilterCapture(void);
  void testFilterInjector(void);

  void testResourceManager(void);
  void testResourceManager_ResourceConsumer(void);
  void testResourceManager_Filter(void);
  void testResourceManager_Pipe(void);

  void testStrategy(void);

  void testDynamicSignalFlow_AddNewPipe(void);
  void testDynamicSignalFlow_AddNewFilter(void);
  void testDynamicSignalFlow_AddNewFilter_PipeMultiThread(void);
};

#endif /* __TESTCASE_PIPEANDFILTER_HPP__ */
