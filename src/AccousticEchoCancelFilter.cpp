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

#include "AccousticEchoCancelFilter.hpp"
#include <iostream>

AccousticEchoCancelFilter::AccousticEchoCancelFilter(ISink* pReferenceSound) : mpReferenceSound(pReferenceSound)
{

}
AccousticEchoCancelFilter::~AccousticEchoCancelFilter()
{
	mpReferenceSound = nullptr;
}

void AccousticEchoCancelFilter::process(AudioBuffer& inBuf, AudioBuffer& outBuf)
{
	// TODO: do AEC
  std::cout << "AccousticEchoCancelFilter!" << std::endl;
  // outBuf = inBuf - pReferenceSund->read(). ref sound & source's delay are adjusted in AecSource.
	outBuf = inBuf;
}