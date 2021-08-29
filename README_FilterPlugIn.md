# Expected audience of this document

Expected audience of this document is developer who develops digital audio processing filter.

# What's filter plug-in?

The filter is implementation to do digital audio signal processing in this audio framework.
To do any signal processing, you need to create your own filter based on ```IFilter``` class.

Actual example is included in this audio framework git.

You can refer to ```example_filter``` folder.

Simplest filter example is as follows:

```FilterExample.hpp
#include "Filter.hpp"

class FilterIncrement : public Filter
{
protected:
  int mWindowSize;

public:
  FilterIncrement(int windowSize = DEFAULT_WINDOW_SIZE_USEC) : mWindowSize(windowSize){};
  virtual ~FilterIncrement(){};
  virtual void process(AudioBuffer& inBuf, AudioBuffer& outBuf);
  virtual int getRequiredWindowSizeUsec(void){ return mWindowSize; };
  virtual std::string toString(void){ return "FilterIncrement"; };
};
```

```FilterExample.cpp
#include "FilterExample.hpp"

void FilterIncrement::process(AudioBuffer& inBuf, AudioBuffer& outBuf)
{
  uint8_t* rawOutBuf = outBuf.getRawBufferPointer();
  for(auto& aData : inBuf.getRawBuffer()){
    *rawOutBuf = (uint8_t)((aData + 1) & 0xFF);
    rawOutBuf++;
  }
}
```

What you need to implement is to override ```process(AudioBuffer& inBuf, AudioBuffer& outBuf)``` method.

You can access input buffer and output buffer.
Then 

1. read the input buffer
2. process anything with the 1.
3. write the processing result to the output buffer

You can also access the buffer's detail such as audio format encoding, sampling rate and channel information.

```
AudioFormat format = inBuf.getAudioFormat();

std::cout << "sampling rate:" << (int)format.getSamplingRate() <<
	" format:" << format.getEncodingString() <<
	" channels:" << (int)format.getNumberOfChannels() <<
	" samples:" << (int)pAudioBuf->getNumberOfSamples() <<
	std::endl;
```

# How to use the filter?

```
$ make afwshared
$ make fdk
$ make filterexample
$ bin/fdk_exec -e PCM_16BIT -r 48000 -c 2 -f lib/filter-plugin/libfilter_example.dylib
```

```example output
$ bin/fdk_exec -e PCM_16BIT -r 48000 -c 2 -f lib/filter-plugin/libfilter_example.dylib
found:lib/filter-plugin/libfilter_example.dylib
onLoad
Specified audio format : encoding:PCM_16BIT channel:2 samplingRate:48000
Source:PcmSource
Sink:PcmSink
Filters:
FilterExampleReverb

Dump Sink data
sampling rate:48000 format:PCM_16BIT channels:2 samples:1920
0,0,0,0,59,0,59,0,b2,ff,b2,ff,a,0,a,0,61,0,61,0,b4,ff,b4,ff,..snip..
```

Without specifying ```-s``` or ```-i```, the default source data is ```PcmSource``` which generates sin wave.
Then you need to specify like ```-i inputdata.dat``` which means reading the binary file and just use the data as the source data which is input to your filter.

Also you can use ```-o``` which output the result to file. ex. ```-o result.dat```, etc. which format is following to what you specified by ```-e```, ```-r``` and ```-c```. You need to use ```ffplay``` etc. to playback on your development environment.


# Other example of filter plu-in, Makefile, etc.

* Filter example : refer to ```example_filter/```

* The makefile
```
UNAME := $(shell uname -s)
ifeq ($(UNAME),Linux)
	FEX_SO_TARGET = $(LIB_FILTER_DIR)/libfilter_example.so
endif
ifeq ($(UNAME),Darwin)
	FEX_SO_TARGET = $(LIB_FILTER_DIR)/libfilter_example.dylib
endif
FDK_DEPS = $(FEX_OBJS:.o=.d)

filterexample: $(FEX_SO_TARGET)
.PHONY: filterexample

$(FEX_SO_TARGET): $(FEX_OBJS)
	@[ -d $(LIB_DIR) ] || mkdir -p $(LIB_DIR)
	@[ -d $(LIB_FILTER_DIR) ] || mkdir -p $(LIB_FILTER_DIR)
	$(CXX) $(LDFLAGS) $(FEX_OBJS) $(SHARED_CXXFLAGS) -o $@ $(LDLIBS) $(AFW_SO_TARGET)

$(FEX_OBJS): $(FEX_SRCS)
	@[ -d $(OBJ_DIR) ] || mkdir -p $(OBJ_DIR)
	$(CXX) $(CXXFLAGS) -I $(INC_DIR) -c $(EXF_DIR)/$(notdir $(@:.o=.cpp)) -o $@
```

* Separated plug-in project examples:
  * https://github.com/hidenorly/DecoderPlugInFFmpeg : Decoder plug-in with FFmpeg
  * https://github.com/hidenorly/SinkPlugInAlsa : Sink plug-in with Alsa

