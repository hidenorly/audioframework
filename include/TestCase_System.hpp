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

#ifndef __TESTCASE_SYSTEM_HPP__
#define __TESTCASE_SYSTEM_HPP__

#include <gtest/gtest.h>

class TestCase_System : public ::testing::Test
{
protected:
  TestCase_System();
  virtual ~TestCase_System();
  virtual void SetUp();
  virtual void TearDown();

  void testParameterManager(void);
  void testParameterManagerRule(void);

  void testPlugInManager(void);
  void testFilterPlugInManager(void);
  void testSourcePlugInManager(void);
  void testSinkPlugInManager(void);
  void testCodecPlugInManager(void);

  void testResourceManager(void);
  void testResourceManager_ResourceConsumer(void);
  void testResourceManager_ResourceConsumer_SharedPtr(void);
  void testResourceManager_Filter(void);
  void testResourceManager_Filter_SharedPtr(void);
  void testResourceManager_Pipe(void);
  void testResourceManager_Pipe_SharedPtr(void);

  void testStrategy(void);
  void testStreamManager(void);

  void testPowerManager(void);
};

#endif /* __TESTCASE_SYSTEM_HPP__ */
