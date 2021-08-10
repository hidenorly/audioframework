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

#include "PlugInManager.hpp"
#include <dlfcn.h>
#include <filesystem>
#include <iostream>
#include <cassert>

IPlugIn::IPlugIn() : mLibraryNativeHandle(nullptr)
{

}

IPlugIn::~IPlugIn()
{

}

void IPlugIn::load(void* libraryNativeHandle)
{
  mLibraryNativeHandle = libraryNativeHandle;
  onLoad();
}

void IPlugIn::unload(void)
{
  onUnload();
}

IPlugInManager::IPlugInManager(std::string path) : mPlugInPath(path)
{

}

IPlugInManager::~IPlugInManager()
{

}

bool IPlugInManager::loadPlugIn(std::string path)
{
  bool result = false;

#if __linux__
  const static std::string fileExt = ".so";
#elif __APPLE__
  const static std::string fileExt = ".dylib";
#else
  const static std::string fileExt = ".so";
#endif

  if( path.ends_with( fileExt ) ){
    std::cout << "found:" << path << std::endl;
    void* pNativeLibraryHandle = dlopen( path.c_str(), RTLD_LAZY|RTLD_LOCAL );
    if( pNativeLibraryHandle ){
      bool bLoadSuccess = false;
      typedef void* (*GetPlugInInstance(void));
      GetPlugInInstance* pGetPlugInInstance = reinterpret_cast<GetPlugInInstance*>( dlsym(pNativeLibraryHandle, "getPlugInInstance") );
      if( pGetPlugInInstance ){
        std::shared_ptr<IPlugIn> pPlugIn( reinterpret_cast<IPlugIn*>(pGetPlugInInstance()) );
        if( pPlugIn ){
          registerPlugIn(pPlugIn);
          pPlugIn->load(pNativeLibraryHandle);
          bLoadSuccess = true;
        } else {
          std::cout << path << ": unabled to get plugin instance" << std::endl;
        }
      } else {
        std::cout << path << ": unabled to get plugin func by dlsym" << std::endl;
      }
      if( !bLoadSuccess ){
        dlclose( pNativeLibraryHandle );
      }
    } else {
      std::cout << path << ": unabled to open" << std::endl;
    }
  }

  return result;
}

void IPlugInManager::initialize(void)
{
  if( std::filesystem::exists( mPlugInPath ) ){
    if( std::filesystem::is_directory(mPlugInPath) ){
      for( auto& aPlugInPath : std::filesystem::directory_iterator( mPlugInPath ) ) {
        loadPlugIn( aPlugInPath.path() );
      }
    } else {
      loadPlugIn( mPlugInPath );
    }
  }
}

void IPlugInManager::terminate(void)
{
  std::vector<std::string> plugInIds;
  for(auto& [plugInId, pPlugIn] : mPlugIns){
    plugInIds.push_back( plugInId );
  }
  for(auto& plugInId : plugInIds){
    unregisterPlugIn( plugInId );
  }
  mPlugIns.clear();
}

std::shared_ptr<IPlugIn> IPlugInManager::getPlugIn(std::string plugInId)
{
  return mPlugIns.contains( plugInId ) ? mPlugIns[ plugInId ] : nullptr;
}

std::vector<std::string> IPlugInManager::getPlugInIds(void)
{
  std::vector<std::string> plugInIds;

  for(auto& [plugInId, pPlugIn] : mPlugIns ){
    plugInIds.push_back( plugInId );
  }

  return plugInIds;
}

bool IPlugInManager::hasPlugIn(std::string plugInId)
{
  return mPlugIns.contains( plugInId );
}


std::string IPlugInManager::registerPlugIn(std::shared_ptr<IPlugIn> pPlugIn)
{
  std::string plugInId;

  if( pPlugIn ){
    plugInId = pPlugIn->getId();
    assert( !mPlugIns.contains(plugInId) );
    mPlugIns.insert_or_assign( plugInId, pPlugIn );
  }

  return plugInId;
}

void IPlugInManager::unregisterPlugIn(std::string plugInId)
{
  if( !plugInId.empty() && mPlugIns.contains(plugInId) ){
    std::shared_ptr<IPlugIn> pPlugIn = mPlugIns[ plugInId ];
    mPlugIns.erase( plugInId );
    if( pPlugIn ){
      pPlugIn->unload();
      void* pHandle = pPlugIn->mLibraryNativeHandle ? pPlugIn->mLibraryNativeHandle : nullptr;
      pPlugIn->mLibraryNativeHandle = nullptr;
      if( 1 == pPlugIn.use_count() ){
        pPlugIn.reset();
        dlclose( pHandle );
      }
    }
  }
}

// for debug
void IPlugInManager::dump(void)
{
  std::cout << "plug-in path: " << mPlugInPath << std::endl;
  for(auto& [plugInId, pPlugIn] : mPlugIns ){
    std::cout << "plug-in [" << plugInId << "]=" << (long)pPlugIn.get() << std::endl;
  }
}
