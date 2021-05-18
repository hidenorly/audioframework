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

#ifndef __PLUGIN_MANAGER_HPP__
#define __PLUGIN_MANAGER_HPP__

#include <string>
#include <map>
#include <vector>

class IPlugIn
{
public:
  IPlugIn();
  virtual ~IPlugIn();

  // for PlugIn Developer
  virtual void onLoad(void) = 0;
  virtual void onUnload(void) = 0;
  virtual std::string getId(void) = 0;
  virtual IPlugIn* newInstance(void) = 0;

  // for PlugIn Manager
  virtual void load(void* libraryNativeHandle);
  virtual void unload(void);

public:
  void* mLibraryNativeHandle;
};

class IPlugInManagerAdmin
{
public:
  virtual void initialize(void) = 0;
  virtual void terminate(void) = 0;
};

class IPlugInManagerUser
{
public:
  virtual IPlugIn* getPlugIn(std::string plugInId) = 0;
  virtual std::vector<std::string> getPlugInIds(void) = 0;
  virtual bool hasPlugIn(std::string plugInId) = 0;
};

class IPlugInManagerPlugIn
{
public:
  virtual std::string registerPlugIn(IPlugIn* pPlugIn) = 0;
  virtual void unregisterPlugIn(std::string plugInId) = 0;
};

class IPlugInManager : public IPlugInManagerAdmin, public IPlugInManagerUser, public IPlugInManagerPlugIn
{
protected:
  std::map<std::string, IPlugIn*> mPlugIns;
  std::string mPlugInPath;

public:
  IPlugInManager(std::string path = ".");
  virtual ~IPlugInManager();

  // Administartive APIs
  virtual void initialize(void);
  virtual void terminate(void);

  // for Plug-In User APIs
  virtual IPlugIn* getPlugIn(std::string plugInId);
  virtual std::vector<std::string> getPlugInIds(void);
  virtual bool hasPlugIn(std::string plugInId);

  // for PlugIn implementor APIs
  virtual std::string registerPlugIn(IPlugIn* pPlugIn);
  virtual void unregisterPlugIn(std::string plugInId);
};

#endif /* __PLUGIN_MANAGER_HPP__ */