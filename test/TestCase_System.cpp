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

#include "TestCase_Common.hpp"
#include "TestCase_System.hpp"

TestCase_System::TestCase_System()
{
}

TestCase_System::~TestCase_System()
{
}

void TestCase_System::SetUp()
{
}

void TestCase_System::TearDown()
{
}

TEST_F(TestCase_System, testParameterManager)
{
  std::shared_ptr<ParameterManager> pParams = ParameterManager::getManager().lock();

  ParameterManager::CALLBACK callbackW = [](std::string key, std::string value){
    std::cout << "callback(param*)): [" << key << "] = " << value << std::endl;
  };
  int callbackIdW = pParams->registerCallback("param*", callbackW);

  pParams->setParameter("paramA", "ABC");
  EXPECT_TRUE( pParams->getParameter("paramA", "HOGE") == "ABC" );
  pParams->setParameterBool("paramB", true);
  EXPECT_TRUE( pParams->getParameterBool("paramB", false) == true );
  pParams->setParameterInt("paramC", 1);
  EXPECT_TRUE( pParams->getParameterInt("paramC", 0) == 1 );
  EXPECT_TRUE( pParams->getParameterInt("paramD", -1) == -1 );

  std::vector<std::string> keys = {"paramA", "paramB", "paramC"};
  std::vector<ParameterManager::Param> params = pParams->getParameters(keys);
  EXPECT_EQ( params.size(), 3 );

  pParams->setParameterInt("ro.paramD", 1);
  EXPECT_TRUE( pParams->getParameterInt("ro.paramD", 0) == 1 );
  pParams->setParameterInt("ro.paramD", 2);
  EXPECT_TRUE( pParams->getParameterInt("ro.paramD", 0) == 1 );

  ParameterManager::CALLBACK callback2 = [](std::string key, std::string value){
    std::cout << "callback(exact match): [" << key << "] = " << value << std::endl;
  };
  int callbackId1 = pParams->registerCallback("paramC", callback2);
  int callbackId2 = pParams->registerCallback("ro.paramD", callback2);
  pParams->setParameterInt("paramC", 1);
  pParams->setParameterInt("paramC", 2);
  pParams->setParameterInt("paramC", 3);
  pParams->setParameterInt("ro.paramD", 3);

  pParams->unregisterCallback(callbackIdW);
  pParams->unregisterCallback(callbackId1);
  pParams->unregisterCallback(callbackId2);
  pParams->unregisterCallback(10000);
  std::cout << "unregistered all notifier" << std::endl;
  pParams->setParameterInt("paramC", 4);

  // dump all
  std::cout << "getParameters()" << std::endl;
  std::vector<ParameterManager::Param> paramsAll = pParams->getParameters();
  for(auto& aParam : paramsAll){
    std::cout << aParam.key << " = " << aParam.value << std::endl;
  }
  std::cout << std::endl;

  std::cout << "getParameters(\"param*\")" << std::endl;
  std::vector<ParameterManager::Param> paramsWilds = pParams->getParameters("param*");
  for(auto& aParam : paramsWilds){
    std::cout << aParam.key << " = " << aParam.value << std::endl;
  }
  std::cout << std::endl;

  const std::string paramFilePath = "TestProperties";

  if( std::filesystem::exists( paramFilePath) ){
    std::filesystem::remove( paramFilePath );
  }

  std::cout << "store to stream" << std::endl;
  std::shared_ptr<FileStream> pFileStream = std::make_shared<FileStream>( paramFilePath );
  pParams->storeToStream( pFileStream );
  pFileStream->close();

  std::cout << "reset all of params" << std::endl;
  pParams->resetAllOfParams();

  std::cout << "restore from stream" << std::endl;
  pFileStream = std::make_shared<FileStream>( paramFilePath );
  pParams->restoreFromStream( pFileStream );
  pFileStream->close();

  std::cout << "reset all of params" << std::endl;
  pParams->resetAllOfParams();

  // non-override restore. This helps to implement default params and user params load. Load current user config value(override=true) and Load the default(preset) value (override=false).
  pParams->setParameter("paramA", "XXX");
  std::cout << "restore from stream" << std::endl;
  pFileStream = std::make_shared<FileStream>( paramFilePath );
  pParams->restoreFromStream( pFileStream, false ); // no override
  pFileStream->close();
  EXPECT_EQ( pParams->getParameter("paramA"), "XXX");

  paramsAll = pParams->getParameters();
  for(auto& aParam : paramsAll){
    std::cout << aParam.key << " = " << aParam.value << std::endl;
  }
}

TEST_F(TestCase_System, testParameterManagerRule)
{
  std::shared_ptr<ParameterManager> pParams = ParameterManager::getManager().lock();
  pParams->resetAllOfParams();

  // --- int, range
  pParams->setParameterRule( "paramA",
    ParameterManager::ParamRule(
      ParameterManager::ParamType::TYPE_INT,
      -12, 12) );

  // out of range
  pParams->setParameterInt("paramA", -13);
  EXPECT_EQ( pParams->getParameterInt("paramA", 0), -12 );

  // in range
  pParams->setParameterInt("paramA", -10);
  EXPECT_EQ( pParams->getParameterInt("paramA", 0), -10 );

  // out of range
  pParams->setParameterInt("paramA", 13);
  EXPECT_EQ( pParams->getParameterInt("paramA", 0), 12 );

  // rule
  ParameterManager::ParamRule ruleA = pParams->getParameterRule("paramA");
  EXPECT_EQ( ruleA.type, ParameterManager::ParamType::TYPE_INT );
  EXPECT_EQ( ruleA.range, ParameterManager::ParamRange::RANGED );
  EXPECT_EQ( (int)ruleA.rangeMin, -12 );
  EXPECT_EQ( (int)ruleA.rangeMax, 12 );

  // --- float, range
  pParams->setParameterRule( "paramF",
    ParameterManager::ParamRule(
      ParameterManager::ParamType::TYPE_FLOAT,
      0.0f, 100.0f) );

  // out of range and get with different type
  pParams->setParameterInt("paramF", -1.0f);
  EXPECT_EQ( pParams->getParameterInt("paramF", 0), 0 );

  // in range and get with defined type
  pParams->setParameterFloat("paramF", 10.0f);
  EXPECT_EQ( pParams->getParameterFloat("paramF", 0.0f), 10.0f );

  // set in range with different type and get with defined type
  pParams->setParameterInt("paramF", 20);
  EXPECT_EQ( pParams->getParameterFloat("paramF", 0.0f), 20.0f );

  // out of range
  pParams->setParameterFloat("paramF", 120.0f);
  EXPECT_EQ( pParams->getParameterInt("paramF", 0), 100.0f );

  // illegal type
  pParams->setParameter("paramF", "120.0f");
  EXPECT_EQ( pParams->getParameterInt("paramF", 0), 100.0f );

  // rule
  ParameterManager::ParamRule ruleF = pParams->getParameterRule("paramF");
  EXPECT_EQ( ruleF.type, ParameterManager::ParamType::TYPE_FLOAT );
  EXPECT_EQ( ruleF.range, ParameterManager::ParamRange::RANGED );
  EXPECT_EQ( ruleF.rangeMin, 0.0f );
  EXPECT_EQ( ruleF.rangeMax, 100.0f );

  // --- enum int
  pParams->setParameterRule( "paramB",
    ParameterManager::ParamRule(
      ParameterManager::ParamType::TYPE_INT,
      {"0", "50", "100"}) );

  // ng case : enum
  pParams->setParameterInt("paramB", -13);
  EXPECT_EQ( pParams->getParameterInt("paramB", 0), 0 );

  // ok case : enum
  pParams->setParameterInt("paramB", 50);
  EXPECT_EQ( pParams->getParameterInt("paramB", 0), 50 );

  // rule
  ParameterManager::ParamRule ruleB = pParams->getParameterRule("paramB");
  EXPECT_EQ( ruleB.type, ParameterManager::ParamType::TYPE_INT );
  EXPECT_EQ( ruleB.range, ParameterManager::ParamRange::RANGE_ENUM );
  EXPECT_EQ( ruleB.enumVals, std::vector<std::string>({"0", "50", "100"}));

  // enum string
  pParams->setParameterRule( "paramC",
    ParameterManager::ParamRule(
      ParameterManager::ParamType::TYPE_STRING,
      {"LOW", "MID", "HIGH"}) );

  // ng case : enum
  pParams->setParameterInt("paramC", -13);
  EXPECT_EQ( pParams->getParameter("paramC", "LOW"), "LOW" );

  // ok case : enum
  pParams->setParameter("paramC", "HIGH");
  EXPECT_EQ( pParams->getParameter("paramC", "LOW"), "HIGH" );

  // rule
  ParameterManager::ParamRule ruleC = pParams->getParameterRule("paramC");
  EXPECT_EQ( ruleC.type, ParameterManager::ParamType::TYPE_STRING );
  EXPECT_EQ( ruleC.range, ParameterManager::ParamRange::RANGE_ENUM );
  EXPECT_EQ( ruleC.enumVals, std::vector<std::string>({"LOW", "MID", "HIGH"}));
}

TEST_F(TestCase_System, testPlugInManager)
{
  IPlugInManager* pPlugInManager = new IPlugInManager();
  pPlugInManager->initialize();

  std::vector<std::string> plugInIds = pPlugInManager->getPlugInIds();
  for(auto& aPlugInId : plugInIds){
    EXPECT_TRUE( pPlugInManager->hasPlugIn( aPlugInId ) );
    EXPECT_NE( nullptr, pPlugInManager->getPlugIn( aPlugInId ) );
  }
  pPlugInManager->dump();

  EXPECT_FALSE( pPlugInManager->hasPlugIn( "hogehogehoge" ) );

  pPlugInManager->terminate();
}

TEST_F(TestCase_System, testFilterPlugInManager)
{
  FilterManager::setPlugInPath("lib/filter-plugin");
  std::weak_ptr<FilterManager> pWeakManager = FilterManager::getInstance();
  std::shared_ptr<FilterManager> pManager = pWeakManager.lock();
  if( pManager ){
    pManager->initialize();

    std::vector<std::string> plugInIds = pManager->getPlugInIds();
    for(auto& aPlugInId : plugInIds){
      EXPECT_TRUE( pManager->hasPlugIn( aPlugInId ) );
      EXPECT_NE( nullptr, pManager->getPlugIn( aPlugInId ) );
      std::shared_ptr<IFilter> pFilter = FilterManager::newInstanceById( aPlugInId );
      EXPECT_NE( nullptr, pFilter );
    }
    pManager->dump();

    EXPECT_FALSE( FilterManager::newInstanceById( "hogehogehoge" ) );

    pManager->terminate();
  }
}

TEST_F(TestCase_System, testSourcePlugInManager)
{
  SourceManager::setPlugInPath("lib/source-plugin");
  std::weak_ptr<SourceManager> pWeakManager = SourceManager::getInstance();
  std::shared_ptr<SourceManager> pManager = pWeakManager.lock();
  if( pManager ){
    pManager->initialize();

    std::vector<std::string> plugInIds = pManager->getPlugInIds();
    for(auto& aPlugInId : plugInIds){
      EXPECT_TRUE( pManager->hasPlugIn( aPlugInId ) );
      EXPECT_NE( nullptr, pManager->getPlugIn( aPlugInId ) );
      std::shared_ptr<ISource> pSource = SourceManager::newInstanceById( aPlugInId );
      EXPECT_NE( nullptr, pSource );
    }
    pManager->dump();

    EXPECT_FALSE( SourceManager::newInstanceById( "hogehogehoge" ) );

    pManager->terminate();
  }
}

TEST_F(TestCase_System, testSinkPlugInManager)
{
  SinkManager::setPlugInPath("lib/sink-plugin");
  std::weak_ptr<SinkManager> pWeakManager = SinkManager::getInstance();
  std::shared_ptr<SinkManager> pManager = pWeakManager.lock();
  if( pManager ){
    pManager->initialize();

    std::vector<std::string> plugInIds = pManager->getPlugInIds();
    for(auto& aPlugInId : plugInIds){
      EXPECT_TRUE( pManager->hasPlugIn( aPlugInId ) );
      EXPECT_NE( nullptr, pManager->getPlugIn( aPlugInId ) );
      std::shared_ptr<ISink> pSink = SinkManager::newInstanceById( aPlugInId );
      EXPECT_NE( nullptr, pSink );
    }
    pManager->dump();

    EXPECT_FALSE( SinkManager::newInstanceById( "hogehogehoge" ) );

    pManager->terminate();
  }
}

TEST_F(TestCase_System, testCodecPlugInManager)
{
  std::string codecPath = "lib/codec-plugin";
  MediaCodecManager::setPlugInPath(codecPath);
  std::weak_ptr<MediaCodecManager> pWeakManager = MediaCodecManager::getInstance();
  std::shared_ptr<MediaCodecManager> pManager = pWeakManager.lock();
  if( pManager ){
    pManager->initialize();

    std::vector<std::string> plugInIds = pManager->getPlugInIds();
    for(auto& aPlugInId : plugInIds){
      EXPECT_TRUE( pManager->hasPlugIn( aPlugInId ) );
      EXPECT_NE( nullptr, pManager->getPlugIn( aPlugInId ) );
      std::shared_ptr<IMediaCodec> pCodec = MediaCodecManager::newInstanceById( aPlugInId );
      EXPECT_NE( nullptr, pCodec );
    }
    EXPECT_FALSE( MediaCodecManager::newInstanceById( "hogehogehoge" ) );

    pManager->dump();

#if __linux__
    std::string exampleCodecPath = codecPath + "/libcodec_example.so";
/* end of __linux */
#elif __APPLE__
    std::string exampleCodecPath = codecPath + "/libcodec_example.dylib";
#else
    // for the others
    std::string exampleCodecPath = codecPath;
#endif
    if( std::filesystem::exists(exampleCodecPath) ){
      std::shared_ptr<IMediaCodec> pCodec = IMediaCodec::createByFormat( AudioFormat(AudioFormat::ENCODING::COMPRESSED) );
      EXPECT_NE( pCodec, nullptr );
      if( pCodec ){
        EXPECT_EQ( pCodec->getId(), "CodecExampleNullDecoder" );
        EXPECT_EQ( pCodec->toString(), "CodecExampleNullDecoder" );
      }
    }

    pManager->terminate();
  }
}

TEST_F(TestCase_System, testResourceManager)
{
  CpuResourceManager::admin_setResource(1000);
  std::shared_ptr<IResourceManager> pResourceManager = CpuResourceManager::getInstance().lock();
  EXPECT_NE( pResourceManager, nullptr);

  int nResourceId1 = pResourceManager->acquire(500);
  EXPECT_NE( nResourceId1, -1 );
  EXPECT_EQ( nResourceId1, 0 );

  int nResourceId2 = pResourceManager->acquire(300);
  EXPECT_NE( nResourceId2, -1 );
  EXPECT_EQ( nResourceId2, 1 );

  int nResourceId3 = pResourceManager->acquire(300);
  EXPECT_EQ( nResourceId3, -1 );

  EXPECT_TRUE( pResourceManager->release(nResourceId2) );

  int nResourceId4 = pResourceManager->acquire(500);
  EXPECT_NE( nResourceId4, -1 );
  EXPECT_EQ( nResourceId4, 2 );

  EXPECT_TRUE( pResourceManager->release(nResourceId1) );
  EXPECT_FALSE( pResourceManager->release(nResourceId3) );
  EXPECT_TRUE( pResourceManager->release(nResourceId4) );

  CpuResourceManager::admin_terminate();
  pResourceManager = nullptr;
}

TEST_F(TestCase_System, testResourceManager_ResourceConsumer_SharedPtr)
{
  class DummyConsumer:public IResourceConsumer
  {
  public:
    DummyConsumer(){};
    virtual ~DummyConsumer(){};
    virtual int stateResourceConsumption(void){ return 300; };
  };

  CpuResourceManager::admin_setResource(1000);
  std::shared_ptr<IResourceManager> pResourceManager = CpuResourceManager::getInstance().lock();
  EXPECT_NE( pResourceManager, nullptr);

  std::shared_ptr<DummyConsumer> consumer1 = std::make_shared<DummyConsumer>();
  EXPECT_TRUE( pResourceManager->acquire(consumer1) );
  EXPECT_FALSE( pResourceManager->acquire(consumer1) );

  std::shared_ptr<DummyConsumer> consumer2 = std::make_shared<DummyConsumer>();
  EXPECT_TRUE( pResourceManager->acquire(consumer2) );

  std::shared_ptr<DummyConsumer> consumer3 = std::make_shared<DummyConsumer>();
  EXPECT_TRUE( pResourceManager->acquire(consumer3) );

  std::shared_ptr<DummyConsumer> consumer4 = std::make_shared<DummyConsumer>();
  EXPECT_FALSE( pResourceManager->acquire(consumer4) );

  EXPECT_TRUE( pResourceManager->release(consumer3) );
  EXPECT_FALSE( pResourceManager->release(consumer3) );
  EXPECT_TRUE( pResourceManager->acquire(consumer4) );

  consumer4.reset();

  std::shared_ptr<DummyConsumer> consumer5 = std::make_shared<DummyConsumer>();
  EXPECT_TRUE( pResourceManager->acquire(consumer5) );
  EXPECT_FALSE( pResourceManager->release(consumer4) );
  EXPECT_FALSE( pResourceManager->release(consumer3) );

  consumer1.reset();
  consumer2.reset();

  CpuResourceManager::admin_terminate();
  pResourceManager = nullptr;
}

TEST_F(TestCase_System, testResourceManager_Filter_SharedPtr)
{
  class DummyConsumer:public Filter
  {
  public:
    DummyConsumer():Filter(){};
    virtual ~DummyConsumer(){};
    virtual int stateResourceConsumption(void){
      return (int)((float)CpuResource::getComputingResource()/3.333f);
    };
  };

  CpuResourceManager::admin_setResource( CpuResource::getComputingResource() );
  std::shared_ptr<IResourceManager> pResourceManager = CpuResourceManager::getInstance().lock();
  EXPECT_NE( pResourceManager, nullptr);

  std::shared_ptr<DummyConsumer> consumer1 = std::make_shared<DummyConsumer>();
  EXPECT_TRUE( pResourceManager->acquire(consumer1) );
  EXPECT_FALSE( pResourceManager->acquire(consumer1) );

  std::shared_ptr<DummyConsumer> consumer2 = std::make_shared<DummyConsumer>();
  EXPECT_TRUE( pResourceManager->acquire(consumer2) );

  std::shared_ptr<DummyConsumer> consumer3 = std::make_shared<DummyConsumer>();
  EXPECT_TRUE( pResourceManager->acquire(consumer3) );

  std::shared_ptr<DummyConsumer> consumer4 = std::make_shared<DummyConsumer>();
  EXPECT_FALSE( pResourceManager->acquire(consumer4) );

  EXPECT_TRUE( pResourceManager->release(consumer3) );
  EXPECT_FALSE( pResourceManager->release(consumer3) );
  EXPECT_TRUE( pResourceManager->acquire(consumer4) );

  consumer4.reset();

  std::shared_ptr<DummyConsumer> consumer5 = std::make_shared<DummyConsumer>();
  EXPECT_TRUE( pResourceManager->acquire(consumer5) );
  EXPECT_FALSE( pResourceManager->release(consumer4) );
  EXPECT_FALSE( pResourceManager->release(consumer3) );

  consumer1.reset();
  consumer2.reset();

  CpuResourceManager::admin_terminate();
  pResourceManager = nullptr;
}


TEST_F(TestCase_System, testResourceManager_Pipe_SharedPtr)
{
  class DummyFilter:public Filter
  {
  public:
    DummyFilter():Filter(){};
    virtual ~DummyFilter(){};
    virtual int stateResourceConsumption(void){
      return (int)((float)CpuResource::getComputingResource()/3.333f);
    };
  };

  CpuResourceManager::admin_setResource( CpuResource::getComputingResource() );
  std::shared_ptr<IResourceManager> pResourceManager = CpuResourceManager::getInstance().lock();
  EXPECT_NE( pResourceManager, nullptr);

  std::shared_ptr<IPipe> pPipe = std::make_shared<Pipe>();
  pPipe->addFilterToTail( std::make_shared<DummyFilter>() );

  bool bSuccessAcquiredResource = pResourceManager->acquire(pPipe);
  EXPECT_TRUE( bSuccessAcquiredResource );
  if( bSuccessAcquiredResource ){
    pPipe->run();
    pPipe->stop();
    EXPECT_TRUE( pResourceManager->release(pPipe) );
  }

  pPipe->addFilterToTail( std::make_shared<DummyFilter>() );
  bSuccessAcquiredResource = pResourceManager->acquire(pPipe);
  EXPECT_TRUE( bSuccessAcquiredResource );
  if( bSuccessAcquiredResource ){
    pPipe->run();
    pPipe->stop();
    EXPECT_TRUE( pResourceManager->release(pPipe) );
  }

  pPipe->addFilterToTail( std::make_shared<DummyFilter>() );
  bSuccessAcquiredResource = pResourceManager->acquire(pPipe);
  EXPECT_TRUE( bSuccessAcquiredResource );
  if( bSuccessAcquiredResource ){
    pPipe->run();
    pPipe->stop();
    EXPECT_TRUE( pResourceManager->release(pPipe) );
  }

  pPipe->addFilterToTail( std::make_shared<DummyFilter>() );
  bSuccessAcquiredResource = pResourceManager->acquire(pPipe);
  EXPECT_FALSE( bSuccessAcquiredResource );
  if( bSuccessAcquiredResource ){
    pPipe->run();
    pPipe->stop();
    EXPECT_TRUE( pResourceManager->release(pPipe) );
  }

  pPipe->clearFilters();
  pPipe.reset();

  pPipe = std::make_shared<PipeMultiThread>();
  pPipe->addFilterToTail( std::make_shared<DummyFilter>() );

  bSuccessAcquiredResource = pResourceManager->acquire(pPipe);
  EXPECT_TRUE( bSuccessAcquiredResource );
  if( bSuccessAcquiredResource ){
    pPipe->run();
    pPipe->stop();
    EXPECT_TRUE( pResourceManager->release(pPipe) );
  }

  pPipe->addFilterToTail( std::make_shared<DummyFilter>() );
  bSuccessAcquiredResource = pResourceManager->acquire(pPipe);
  EXPECT_TRUE( bSuccessAcquiredResource );
  if( bSuccessAcquiredResource ){
    pPipe->run();
    pPipe->stop();
    EXPECT_TRUE( pResourceManager->release(pPipe) );
  }

  pPipe->addFilterToTail( std::make_shared<DummyFilter>() );
  bSuccessAcquiredResource = pResourceManager->acquire(pPipe);
  EXPECT_TRUE( bSuccessAcquiredResource );
  if( bSuccessAcquiredResource ){
    pPipe->run();
    pPipe->stop();
    EXPECT_TRUE( pResourceManager->release(pPipe) );
  }

  pPipe->addFilterToTail( std::make_shared<DummyFilter>() );
  bSuccessAcquiredResource = pResourceManager->acquire(pPipe);
  EXPECT_FALSE( bSuccessAcquiredResource );
  if( bSuccessAcquiredResource ){
    pPipe->run();
    pPipe->stop();
    EXPECT_TRUE( pResourceManager->release(pPipe) );
  }

  pPipe->clearFilters();

  CpuResourceManager::admin_terminate();
  pResourceManager = nullptr;
}


TEST_F(TestCase_System, testStrategy)
{
  class StrategyA : public IStrategy
  {
  public:
    StrategyA():IStrategy(){};
    virtual ~StrategyA(){};

    virtual bool canHandle(std::shared_ptr<StrategyContext> context){
      return true;
    }
    virtual bool execute(std::shared_ptr<StrategyContext> context){
      std::cout << "StrategyA is executed" << std::endl;
      return true;
    }
  };
  class StrategyB : public IStrategy
  {
  public:
    StrategyB():IStrategy(){};
    virtual ~StrategyB(){};

    virtual bool canHandle(std::shared_ptr<StrategyContext> context){
      return false;
    }
    virtual bool execute(std::shared_ptr<StrategyContext> context){
      std::cout << "StrategyB is executed" << std::endl;
      return true;
    }
  };
  Strategy strategy;
  std::shared_ptr<StrategyContext> context = std::make_shared<StrategyContext>();
  strategy.registerStrategy( std::make_shared<StrategyB>() );
  strategy.registerStrategy( std::make_shared<StrategyA>() );
  EXPECT_TRUE( strategy.execute(context) );
}


TEST_F(TestCase_System, testStreamManager)
{
  std::weak_ptr<StreamManager> pWeakManager = StreamManager::getInstance();
  std::shared_ptr<StreamManager> pManager = pWeakManager.lock();

  if( pManager ){
    std::shared_ptr<StrategyContext> pContext = std::make_shared<StrategyContext>();
    std::shared_ptr<IPipe> pPipe = std::make_shared<Pipe>();

    pManager->add( pContext, pPipe );
    std::shared_ptr<StreamInfo> pStreamInfo = pManager->get( pContext );

    EXPECT_EQ( pStreamInfo->pipe, pPipe );
    EXPECT_EQ( pStreamInfo->context, pContext );

    int id = pStreamInfo->id;

    pStreamInfo.reset();
    pStreamInfo = pManager->get( pPipe );
    EXPECT_EQ( pStreamInfo->pipe, pPipe );
    EXPECT_EQ( pStreamInfo->context, pContext );

    pStreamInfo.reset();
    pStreamInfo = pManager->get( id );
    EXPECT_EQ( pStreamInfo->pipe, pPipe );
    EXPECT_EQ( pStreamInfo->context, pContext );

    EXPECT_EQ( pPipe, pManager->getPipe( id ) );
    EXPECT_EQ( pPipe, pManager->getPipe( pContext ) );

    EXPECT_EQ( id, pManager->getId( pPipe ) );
    EXPECT_EQ( id, pManager->getId( pContext ) );

    EXPECT_EQ( pContext, pManager->getContext( id ) );
    EXPECT_EQ( pContext, pManager->getContext( pPipe ) );

    EXPECT_TRUE( pManager->remove( pStreamInfo ) );
    pStreamInfo.reset();
    pStreamInfo = pManager->get( pPipe );
    EXPECT_EQ( pStreamInfo, nullptr );
  }

  StreamManager::terminate();
}


TEST_F(TestCase_System, testPowerManager)
{
  std::shared_ptr<IPowerManager> pManager = PowerManager::getManager().lock();
  if( pManager ){
    IPowerManager::CALLBACK callback = [&](IPowerManager::POWERSTATE powerState){
      std::cout << "power state change(callback): " << pManager->getPowerStateString( powerState ) << std::endl;
    };
    int callbackId = pManager->registerCallback( callback );

    class MyListener : public IPowerManager::PowerChangeListener
    {
    public:
      MyListener(){};
      virtual ~MyListener(){};
      virtual void onPowerStateChanged(IPowerManager::POWERSTATE powerState){
        std::shared_ptr<IPowerManager> pManager = PowerManager::getManager().lock();
        std::cout << "power state change(listener): " << pManager->getPowerStateString( powerState ) << std::endl;
      }
    };
    std::shared_ptr<MyListener> pListener = std::make_shared<MyListener>();
    pManager->registerListener( pListener );

    std::shared_ptr<PowerManagerPrimitive> pTestShim = std::dynamic_pointer_cast<PowerManagerPrimitive>( pManager->getTestShim().lock() );
    if( pTestShim ){
      pTestShim->setPowerState( IPowerManager::POWERSTATE::ACTIVE );
      pTestShim->setPowerState( IPowerManager::POWERSTATE::IDLE );
      pTestShim->setPowerState( IPowerManager::POWERSTATE::SUSPEND );
      pTestShim->setPowerState( IPowerManager::POWERSTATE::OFF );
    }

    pManager->unregisterCallback( callbackId );
    EXPECT_TRUE( pManager->unregisterListener( pListener ) );
  }
}

#include "Singleton.hpp"

TEST_F(TestCase_System, testSingleton)
{
  class SingletonTest : public SingletonBase<SingletonTest>
  {
  public:
    bool isWorking(void){ return true; };
  };

  std::cout << SingletonTest::getReferenceCount() << std::endl;
  EXPECT_EQ( SingletonTest::getReferenceCount(), 0 );
  std::weak_ptr<SingletonTest> pWeakInstance = SingletonTest::getInstance();
  EXPECT_EQ( SingletonTest::getReferenceCount(), 1 );
  EXPECT_EQ( pWeakInstance.expired(), false );
  if( !pWeakInstance.expired() ){
    std::cout << SingletonTest::getReferenceCount() << std::endl;
    std::shared_ptr<SingletonTest> pSharedInstance = pWeakInstance.lock();
    EXPECT_EQ( SingletonTest::getReferenceCount(), 2 );
    std::cout << SingletonTest::getReferenceCount() << std::endl;
    EXPECT_NE( pSharedInstance, nullptr );
    if( pSharedInstance ){
      EXPECT_TRUE( pSharedInstance->isWorking() );
    }
    pSharedInstance.reset();
    std::cout << SingletonTest::getReferenceCount() << std::endl;
  EXPECT_EQ( SingletonTest::getReferenceCount(), 1 );
  }
  SingletonTest::close();
  std::cout << SingletonTest::getReferenceCount() << std::endl;
  EXPECT_EQ( SingletonTest::getReferenceCount(), 0 );
  EXPECT_EQ( pWeakInstance.expired(), true );
}
