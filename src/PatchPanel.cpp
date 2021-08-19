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

#include "PatchPanel.hpp"
#include <iostream>

PatchPanel::PatchPanel()
{

}

PatchPanel::~PatchPanel()
{
  if( mMixerSplitter ) {
    mMixerSplitter->stop();
  }
  mPipes.clear(); // these pipes should be stopped by onRunnerStatusChanged.
  mMixerSplitter.reset();
}

void PatchPanel::onRunnerStatusChanged(bool bRunning)
{
  for( auto& [pSource, pPipe] : mPipes ){
    if( bRunning ){
      pPipe->run();
    } else {
      pPipe->stop();
    }
  }
}


std::shared_ptr<MixerSplitter> PatchPanel::getMixerSplitter(void)
{
  if( !mMixerSplitter ){
    mMixerSplitter = std::make_shared<MixerSplitter>();
  }
  return mMixerSplitter;
}

void PatchPanel::addSourcePipe(std::shared_ptr<ISource> pSource, std::shared_ptr<IPipe> pPipe)
{
  if( pSource && pPipe ){
    mPipes.insert_or_assign( pSource, pPipe );
  }
}

std::shared_ptr<PatchPanel> PatchPanel::createPatch(std::vector<std::shared_ptr<ISource>> pSources, std::vector<std::shared_ptr<ISink>> pSinks)
{
  std::shared_ptr<PatchPanel> pPatchPanel = std::shared_ptr<PatchPanel>( new PatchPanel() );
  std::shared_ptr<MixerSplitter> pMixerSplitter = pPatchPanel->getMixerSplitter();

  for( auto& pSink : pSinks ){
    pMixerSplitter->addSink( pSink );
  }

  std::vector<std::shared_ptr<ISink>> pSinkAdaptors;
  for( auto& pSource : pSources ){
    std::shared_ptr<IPipe> pPipe = std::make_shared<Pipe>();
    pPipe->attachSource( pSource );
    pPipe->addFilterToTail( std::make_shared<PassThroughFilter>() );
    AudioFormat audioFormat = pSource ? pSource->getAudioFormat() : AudioFormat();
    std::shared_ptr<ISink> pSinkAdaptor = pMixerSplitter->allocateSinkAdaptor( audioFormat, pPipe );
    pSinkAdaptors.push_back( pSinkAdaptor );
    pPipe->attachSink( pSinkAdaptor );
    pPatchPanel->addSourcePipe( pSource, pPipe );
  }

  // TODO: Use MultiSink to improve the efficiency
  for( auto& pSink : pSinks ){
    for( auto& pSource : pSinkAdaptors ){
      pMixerSplitter->map( pSource, pSink );
    }
  }

  pMixerSplitter->registerRunnerStatusListener( pPatchPanel );

  return pPatchPanel;
}
