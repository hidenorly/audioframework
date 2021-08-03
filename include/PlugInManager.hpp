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
#include <memory>

class IPlugIn
{
public:
  IPlugIn();
  virtual ~IPlugIn();

  // for PlugIn Developer
  virtual void onLoad(void) = 0;
  virtual void onUnload(void) = 0;
  virtual std::string getId(void) = 0;
  virtual std::shared_ptr<IPlugIn> newInstance(void) = 0;

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
  virtual std::shared_ptr<IPlugIn> getPlugIn(std::string plugInId) = 0;
  virtual std::vector<std::string> getPlugInIds(void) = 0;
  virtual bool hasPlugIn(std::string plugInId) = 0;
};

class IPlugInManagerPlugIn
{
public:
  virtual std::string registerPlugIn(std::shared_ptr<IPlugIn> pPlugIn) = 0;
  virtual void unregisterPlugIn(std::string plugInId) = 0;
};

class IPlugInManager : public IPlugInManagerAdmin, public IPlugInManagerUser, public IPlugInManagerPlugIn
{
protected:
  std::map<std::string, std::shared_ptr<IPlugIn>> mPlugIns;
  std::string mPlugInPath;

  bool loadPlugIn(std::string path);
  virtual void _setPlugInPath(std::string path){ mPlugInPath = path; };

public:
  IPlugInManager(std::string path = ".");
  virtual ~IPlugInManager();

  // Administartive APIs
  virtual void initialize(void);
  virtual void terminate(void);

  // for Plug-In User APIs
  virtual std::shared_ptr<IPlugIn> getPlugIn(std::string plugInId);
  virtual std::vector<std::string> getPlugInIds(void);
  virtual bool hasPlugIn(std::string plugInId);

  // for PlugIn implementor APIs
  virtual std::string registerPlugIn(std::shared_ptr<IPlugIn> pPlugIn);
  virtual void unregisterPlugIn(std::string plugInId);

  // for debug
  virtual void dump(void);
};

template <class IFCLASS> class TPlugInManager : public IPlugInManager
{
  static inline TPlugInManager* mpManager;
  static inline std::string mPlugInPath;

protected:
  TPlugInManager(): IPlugInManager( mPlugInPath ){ mpManager = this; };
  virtual ~TPlugInManager(){};

public:
  virtual void terminate(void){
    IPlugInManager::terminate();
    mpManager = nullptr;
    delete this;
  }

  static void setPlugInPath(std::string path){
    mPlugInPath = path;
    if( mpManager ){
      mpManager->_setPlugInPath( path );
    }
  }

  static TPlugInManager* getInstance(void){
    if( !mpManager ){
      mpManager = new TPlugInManager<IFCLASS>();
    }
    return mpManager;
  }

  static std::shared_ptr<IFCLASS> newInstanceById(std::string plugInId){
    std::shared_ptr<IFCLASS> pPlugInInstance = nullptr;

    if( !mpManager ){
      getInstance();
    }

    if( mpManager ){
      std::shared_ptr<IPlugIn> pPlugIn = mpManager->getPlugIn( plugInId );
      if( pPlugIn ){
        pPlugInInstance = std::dynamic_pointer_cast<IFCLASS>( pPlugIn->newInstance() );
      }
    }

    return pPlugInInstance;
  }
};

#endif /* __PLUGIN_MANAGER_HPP__ */