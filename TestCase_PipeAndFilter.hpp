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
  void testPipeManager(void);
  void testMultipleSink(void);
  void testStreamSink(void);
  void testStreamSink_DifferentFormat(void);

  void testStreamSource(void);

  void testParameterManager(void);
};

#endif /* __TESTCASE_PIPEANDFILTER_HPP__ */
