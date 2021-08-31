# Expected audience of this document

Expected audience of this document is developer who develops digital audio processing's output implementation, called "Sink".

Signal flow is that Source -> Pipe -> Sink.
In the Pipe, there can be filters which are actual digtal signal processings.

# What's Sink plug-in?

The Sink is implementation to output the digital audio signal processing result in this audio framework.
To output any signal processing result, you need to create your own Sink based on ```ISink``` class.

Actual example is included in this audio framework git.
You can refer to ```example_sink``` folder.

Simplest Sink example is as follows:

```
#include "Sink.hpp"
#include "Util.hpp"

class SimpleSink : public ISink
{
protected:
  virtual void writePrimitive(IAudioBuffer& buf){
    AudioBuffer* pBuf = std::dynamic_pointer_cast<AudioBufer>(&buf);
    if( pBuf ){
      // this buffer is PCM buffer
    } else {
      // this buffer is non-PCM buffer which means encoded ES data such as AC-3 compressed data, etc.
    }
    Util::dumpBuffer("Dump Sink data", buf);
  }

public:
  SimpleSink(){};
  virtual ~SimpleSink(){};
  virtual std::string toString(void){ return "SimpleSink"; };
  virtual void dump(void){};
};
```

What you need to implement is to override ```void writePrimitive(IAudioBuffer& buf)``` method.

You can access the buffer.
Then

1. read the input buffer
2. write out the read input buffer to the other such as Alsa, etc.

You can also access the buffer's detail such as audio format encoding, sampling rate and channel information.

```
AudioFormat format = buf.getAudioFormat();

std::cout << "sampling rate:" << (int)format.getSamplingRate() <<
	" format:" << format.getEncodingString() <<
	" channels:" << (int)format.getNumberOfChannels() <<
	" samples:" << (int)pAudioBuf->getNumberOfSamples() <<
	std::endl;
```

# How to use the sink?

```
$ make afwshared
$ make fdk
$ make sinkexample
$ bin/fdk_exec -e PCM_16BIT -r 48000 -c 2 -s lib/sink-plugin/libsink_example.dylib
```

* Without specifying ```-i``` and ```-u```, the default source data is ```PcmSource``` which generates sin wave.
* Without specifying ```-f```, the default filter is passthrough filter. No any processing by it.

Then you need to specify like ```-i inputdata.dat``` which means reading the binary file and just use the data as the source data.

Please note that if you specify ```-o``` and ```-s```, the ```-o``` is prioritized than ```-s```.
Then if you'd like to test your sink plug-in, please don't use ```-o```.

# Other example of sink plug-in, Makefile, etc.

* Sink example : refer to ```example_sink/```

* The makefile
```
UNAME := $(shell uname -s)
ifeq ($(UNAME),Linux)
  EXO_SO_TARGET = $(LIB_SINK_DIR)/libsink_example.so
endif
ifeq ($(UNAME),Darwin)
  EXO_SO_TARGET = $(LIB_SINK_DIR)/libsink_example.dylib
endif
EXO_DEPS = $(EXO_OBJS:.o=.d)

sinkexample: $(EXO_SO_TARGET)
.PHONY: sinkexample

$(EXO_SO_TARGET): $(EXO_OBJS)
  @[ -d $(LIB_DIR) ] || mkdir -p $(LIB_DIR)
  @[ -d $(LIB_SINK_DIR) ] || mkdir -p $(LIB_SINK_DIR)
  $(CXX) $(LDFLAGS) $(EXO_OBJS) $(SHARED_CXXFLAGS) -o $@ $(LDLIBS) $(AFW_SO_TARGET)

$(EXO_OBJS): $(EXO_SRCS)
  @[ -d $(OBJ_DIR) ] || mkdir -p $(OBJ_DIR)
  $(CXX) $(CXXFLAGS) -I $(INC_DIR) -c $(EXO_DIR)/$(notdir $(@:.o=.cpp)) -o $@
```

* Separated plug-in project examples:
  * https://github.com/hidenorly/DecoderPlugInFFmpeg : Decoder plug-in with FFmpeg
  * https://github.com/hidenorly/SinkPlugInAlsa : Sink plug-in with Alsa

