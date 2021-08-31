# Expected audience of this document

Expected audience of this document is developer who develops digital audio processing's input implementation, called "Source".

Signal flow is that Source -> Pipe -> Sink.
In the Pipe, there can be filters which are actual digtal signal processings.


# What's Source plug-in?

The Source is implementation to input to Pipe which is place of the digital audio signal processing in this audio framework.

To input data to what you'd like to do signal processing, you need to create your own Source based on ```ISource``` class.

Actual example is included in this audio framework git.
You can refer to ```example_source``` folder.

Simplest Source example is as follows:

```
#include "Source.hpp"
#include "Util.hpp"

class SimpleSource : public ISource
{
protected:
  virtual void readPrimitive(IAudioBuffer& buf){
    int nSize = buf.getRawBufferSize();
    uint8_t* ptr = reinterpret_cast<uint8_t*>( buf.getRawBufferPointer() );
    for( int i = 0; i < nSize; i++ ){
      *ptr++ = i & 0xFF;
    }
  }
public:
  SimpleSource(){};
  virtual ~SimpleSource(){};
  virtual std::string toString(void){return "SimpleSource";};
};
```

What you need to implement is to override ```void readPrimitive(IAudioBuffer& buf)``` method.

You can access the buffer.
Then your implementation needs to write out your data to the buffer such as reading from file, reading audio input from mic, etc.

Note that you can also access the buffer's detail such as audio format encoding, sampling rate and channel information.

```
AudioFormat format = buf.getAudioFormat();

std::cout << "sampling rate:" << (int)format.getSamplingRate() <<
  " format:" << format.getEncodingString() <<
  " channels:" << (int)format.getNumberOfChannels() <<
  " samples:" << (int)pAudioBuf->getNumberOfSamples() <<
  std::endl;
```

# How to use the source?

```
$ make afwshared
$ make fdk
$ make sourceexample
$ bin/fdk_exec -e PCM_16BIT -r 48000 -c 2 -u lib/source-plugin/libsource_example.dylib
```

* Without specifying ```-f```, the default filter is passthrough filter. No any processing by it.
* Without specifying ```-o``` or ```-s```, the default sink data is ```PcmSink``` which is just to pool data to memory.


Please note that if you specify ```-i``` and ```-u```, the ```-i``` is prioritized than ```-u```.
Then if you'd like to test your source plug-in, please don't use ```-i```.

# Other example of source plug-in, Makefile, etc.

* Source example : refer to ```example_source/```

* The makefile
```
# --- Build for source example(shared) ------------
UNAME := $(shell uname -s)
ifeq ($(UNAME),Linux)
  EXI_SO_TARGET = $(LIB_SOURCE_DIR)/libsource_example.so
endif
ifeq ($(UNAME),Darwin)
  EXI_SO_TARGET = $(LIB_SOURCE_DIR)/libsource_example.dylib
endif
EXI_DEPS = $(EXI_OBJS:.o=.d)

sourceexample: $(EXI_SO_TARGET)
.PHONY: sourceexample

$(EXI_SO_TARGET): $(EXI_OBJS)
  @[ -d $(LIB_DIR) ] || mkdir -p $(LIB_DIR)
  @[ -d $(LIB_SOURCE_DIR) ] || mkdir -p $(LIB_SOURCE_DIR)
  $(CXX) $(LDFLAGS) $(EXI_OBJS) $(SHARED_CXXFLAGS) -o $@ $(LDLIBS) $(AFW_SO_TARGET)

$(EXI_OBJS): $(EXI_SRCS)
  @[ -d $(OBJ_DIR) ] || mkdir -p $(OBJ_DIR)
  $(CXX) $(CXXFLAGS) -I $(INC_DIR) -c $(EXI_DIR)/$(notdir $(@:.o=.cpp)) -o $@
```

* Separated plug-in project examples:
  * https://github.com/hidenorly/DecoderPlugInFFmpeg : Decoder plug-in with FFmpeg
  * https://github.com/hidenorly/SinkPlugInAlsa : Sink plug-in with Alsa
