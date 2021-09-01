# Expected audience of this document

Expected audience of this document is developer who develops plug-in of Filter, Source, Sink and Codec.
This is common documentation of the plug-in.


# What's plug-in?

Since audio framework is framework, the actual implementations of Filter, Source, Sink and Codec are separated as plug-in.

Those plug-in is based on common mechanism therefore the plug-in should satisfy with common requirements.

You can refer to actual codec implementations under example_codec/, example_filter/, example_sink/ and example_source.

# Common interfaces which you need to implement

You can see some of interfaces are defined as pure virtual ( = 0 ).
Therefore you must implement the pure virtual interfaces on your plug-in as common requirement of plug-in.

```
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
..snip..
};
```

* onLoad() is an event handler of loading the plug-in. If you need to do anything at loading timining, you do it in the implementation.

```
  virtual void onLoad(void){
    std::cout << "onLoad" << std::endl;
  }
```

* onUnload() is an event handler of loading the plug-in. This is an event handler of pair of onLoad().

```
  virtual void onUnload(void){
    std::cout << "onUnload" << std::endl;
  }
```

* getId() is to return your the plug-in ID. This must be unique.

```
  virtual std::string getId(void){
    return std::string("FilterExampleReverb");
  }
```

* newInstance() should do new instance for your implementation class and should be cast to IPlugIn.

```
  virtual std::shared_ptr<IPlugIn> newInstance(void){
    return std::make_shared<FilterExampleReverb>();
  }
```

And also you need to export the common method ```void* getPlugInInstance(void)``` as "C" from your plug-in shared object.

```
extern "C"
{
void* getPlugInInstance(void)
{
  FilterExampleReverb* pFilter = new FilterExampleReverb();
  return reinterpret_cast<void*>(dynamic_cast<IPlugIn*>(pFilter));
}
};```

Your implementation must instantiate your plug-in class and reinterpret_cast to void* after cast to IPlugIn* as the above example.


# Note that

* For Filter plug-in develop, you need to extend from FilterPlugIn.
* For Source plug-in develop, you need to extend from SourcePlugIn.
* For Sink plug-in develop, you need to extend from SinkPlugIn.
* For Codec plug-in develop, you need to extend from IDecoder.
