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

#ifndef __PATCH_PANEL_HPP__
#define __PATCH_PANEL_HPP__

#include "MixerSplitter.hpp"
#include "Source.hpp"
#include "Sink.hpp"
#include <memory>
#include <map>
#include <vector>
#include "Pipe.hpp"

class PatchPanel : public ThreadBase::RunnerListener
{
protected:
  std::shared_ptr<MixerSplitter> mMixerSplitter;
  std::map<std::shared_ptr<ISource>, std::shared_ptr<IPipe>> mPipes;

  void addSourcePipe(std::shared_ptr<ISource> pSource, std::shared_ptr<IPipe> pPipe);
  void getDeltaSink(std::vector<std::shared_ptr<ISink>> pCurrent, std::vector<std::shared_ptr<ISink>> pNext, std::vector<std::shared_ptr<ISink>>& pOutAdded, std::vector<std::shared_ptr<ISink>>& pOutRemoved);
  void getDeltaSource(std::vector<std::shared_ptr<ISink>> pCurrent, std::vector<std::shared_ptr<ISource>> pNext, std::vector<std::shared_ptr<ISource>>& pOutAdded, std::vector<std::shared_ptr<ISink>>& pOutRemoved);

  virtual void onRunnerStatusChanged(bool bRunning);
  PatchPanel();

public:
  virtual ~PatchPanel();
  std::shared_ptr<MixerSplitter> getMixerSplitter(void);

  static std::shared_ptr<PatchPanel> createPatch(std::vector<std::shared_ptr<ISource>> pSources, std::vector<std::shared_ptr<ISink>> pSinks);
  void updatePatch(std::vector<std::shared_ptr<ISource>> pSources, std::vector<std::shared_ptr<ISink>> pSinks);
};

#endif /* __PATCH_PANEL_HPP__ */