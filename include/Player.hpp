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

#ifndef __PLAYER_HPP__
#define __PLAYER_HPP__

#include "Pipe.hpp"
#include "Source.hpp"
#include "Decoder.hpp"
#include <vector>
#include <memory>

class IPlayer
{
public:
  virtual ~IPlayer(){};
  virtual std::shared_ptr<ISource> prepare(std::shared_ptr<ISource> pSource, std::shared_ptr<IDecoder> pDecoder) = 0;
  virtual std::shared_ptr<ISource> terminate(std::shared_ptr<ISource> pSource) = 0;
  virtual bool isReady(void) = 0;

  virtual void play(int64_t ptsUSec = 0) = 0;
  virtual void stop(void) = 0;
  virtual void pause(void) = 0;
  virtual void resume(void) = 0;
  virtual void seek(int64_t ptsUSec) = 0;
  virtual int64_t getPosition(void) = 0;
};

class Player : public IPlayer
{
protected:
  std::shared_ptr<IDecoder> mpDecoder;
  bool mbIsSetupDone;
  bool mbIsPaused;
  int64_t mPosition;

public:
  Player();
  virtual ~Player();

  virtual std::shared_ptr<ISource> prepare(std::shared_ptr<ISource> pSource, std::shared_ptr<IDecoder> pDecoder);
  virtual std::shared_ptr<ISource> terminate(std::shared_ptr<ISource> pSourceAdaptor);
  virtual bool isReady(void);

  virtual void play(int64_t ptsUSec = 0);
  virtual void stop(void);
  virtual void pause(void);
  virtual void resume(void);
  virtual void seek(int64_t ptsUSec);
  virtual int64_t getPosition(void);
};

#endif /* __PLAYER_HPP__ */
