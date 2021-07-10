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

#ifndef __TESTCASE_UTIL_HPP__
#define __TESTCASE_UTIL_HPP__

#include <gtest/gtest.h>

class TestCase_Util : public ::testing::Test
{
protected:
  TestCase_Util();
  virtual ~TestCase_Util();
  virtual void SetUp();
  virtual void TearDown();

  void testStringTokenizer(void);

  void testFifoBuffer(void);

  void testThreadBase(void);
};

#endif /* __TESTCASE_UTIL_HPP__ */
