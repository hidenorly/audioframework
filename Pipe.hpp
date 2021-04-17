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

#ifndef __PIPE_H__
#define __PIPE_H__

#include "Filter.hpp"
#include "Source.hpp"
#include "Sink.hpp"
#include <vector>

class Pipe
{
public:
  Pipe();
  virtual ~Pipe();

  void addFilterToHead(Filter* pFilter);
  void addFilterToTail(Filter* pFilter);

  Sink* attachSink(Sink* pSink);
  Sink* detachSink(void);
  Source* attachSource(Source* pSink);
  Source* detachSource(void);

  void run(void);
  void stop(void);
  bool isRunning(void);

  void dump(void);
  void clearFilers(void);

protected:
  virtual void process(void);

  std::vector<Filter*> mFilters;
  Sink* mpSink;
  Source* mpSource;
  bool mbIsRunning;
};

#endif /* __PIPE_H__ */