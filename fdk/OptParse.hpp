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

#include <iostream>
#include <string>
#include <algorithm>
#include "StringUtil.hpp"

class OptParse
{
public:
  std::vector<std::string> args;
  std::map<std::string, std::string> values;

  class OptParseItem
  {
  public:
    std::string option;     // "-h"
    std::string fullOption; // "--help"
    bool bArgRequired;      // true: the value required / false: the value not required
    std::string value;
    std::string description;

    OptParseItem(){};
    OptParseItem(std::string option, std::string fullOption, bool bArgRequired, std::string defaultValue, std::string description):option(option), fullOption(fullOption), bArgRequired(bArgRequired), value(defaultValue), description(description){};
    virtual ~OptParseItem(){};
  };

protected:
  virtual void parseOption(OptParseItem& anOption)
  {
    for( int i=0, c=args.size(); i<c; i++ ){
      if( args[i] == anOption.option ){
        if( anOption.bArgRequired && (i+1) < c ){
          values.insert_or_assign( anOption.option, args[i+1] );
        } else {
          values.insert_or_assign( anOption.option, anOption.option );
        }
        break;
      } else if( args[i].starts_with( anOption.fullOption ) ){
        int nPos = args[i].find("=");
        if( nPos != std::string::npos ){
          values.insert_or_assign( anOption.option, args[i].substr( nPos+1, args[i].size() ) );
        } else {
          values.insert_or_assign( anOption.option, anOption.option );
        }
        break;
      }
    }
  };

  virtual void parseOpts(std::vector<OptParseItem>& options){
    for( auto& anOption : options ){
      parseOption( anOption );
    }
    for( auto& anOption : options ){
      if( anOption.option != "-h" && !values.contains( anOption.option ) ){
        values.insert_or_assign( anOption.option, anOption.bArgRequired ? anOption.value : anOption.option );
      }
    }
  };

  bool isOptionIncluded(std::vector<OptParseItem>& options, std::string option, std::string fullOption){
    bool bFound = false;
    for( auto& anOption : options ){
      if( anOption.option == option || anOption.fullOption == fullOption ){
        bFound = true;
        break;
      }
    }
    return bFound;
  }


public:
  virtual void printHelp(std::vector<OptParseItem>& options){
    int nOptionMax = 0;
    int nFullOptionMax = 0;

    for( auto& anOption : options ){
      nOptionMax = std::max( nOptionMax, (int)anOption.option.size() );
      nFullOptionMax = std::max( nFullOptionMax, (int)anOption.fullOption.size() );
    }
    for( auto& anOption : options ){
      std::cout << "  " << StringUtil::ljust(anOption.option, nOptionMax) << "\t" << StringUtil::ljust(anOption.fullOption, nFullOptionMax) << " : " << anOption.description << std::endl;
    }
    exit(0);
  };

  OptParse(int argc, char **argv, std::vector<OptParseItem> options, std::string description = ""){
    for(int i=1; i<argc; i++){
      args.push_back( std::string(argv[i]) );
    }
    if( !isOptionIncluded(options, "-h", "--help") ){
      options.push_back( OptParse::OptParseItem("-h", "--help", false, "", "Show help") );
    }
    parseOpts( options );
    if( values.contains("-h") ){
      if( !description.empty() ){
        std::cout << description << std::endl;
      }
      printHelp(options);
    }
  }
  virtual ~OptParse(){};
};
