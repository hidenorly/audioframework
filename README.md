# Audio framework

This is intending to establish audio framework.
The expection is to implement android audio hal, surround amplifier, mediaplayer, etc. with this framework.

* C++20 based audio framework
  * Common
    * Use smart pointer ```std::make_shared<>``` to create instances
      e.g. ```std::shared_ptr<ISource> pSource = std::make_shared<Source>();```

  * Pipe and filter
    * Source
      * This is input source. Pipe read data from the source.
      * Interface is ```ISoure```
      * Concrete classes are derived from the ISource.
    * Sink
      * This is output destination. Pipe output to the Sink.
      * Interface is ```ISink```
      * Concrete classes are derived from the ISink.
      * Use MultipleSink to split the Sink for actual multiple sinks(=output)
    * Pipe
      * Pipe is place to do signal processing for read data from Source and output the result to Sink.
        * ```Source``` --> ```Pipe``` --> ```Sink```
      * Note that actual signal processing is done by attached filters to the pipe.
      * The interface is ```IPipe```
      * Concrete classes are derived from the IPipe.
      * There are 2 types of pipe.
      * ```Pipe```
        * Different window size filters are supported.
          * LCM window size processing by Pipe
          * Minimum window size processing by PipeMultiThread which is multi threads & FIFO buffer connected among them.
            * Same window size is running in same thread
            * But the different window size will create different pipe and interconnected by FiFO Buffers automatically
      * ```PipeMultiThread```
        * Internally ```PipeMultiThread``` includes ```Pipe``` instances to execute Pipes concurrently.
        * Since ```Pipe``` is using window size as LCM manner,
          this internally create ```Pipe``` instances if required filter size is different for attached filter.
          Therefore using this class enables you to reduce total latency by concurrent execution with minimized window size.
        * Note that the Pipe and the Pipe are connected by ```InterPipeBridge``` which is FifoBuffer which is working as ```ISink``` and ```ISource```.
    * Filter
      * In the Pipe, attached filteres will do signal processing.
      * The instance is attachable to ```IPipe```
      * ```Pipe::clearFilters()``` will detach the attached instances.
      * Special filter: Accoustic Echo Cancel Filter
        * This filter read Source(e.g. Mic) and Sink (e.g. Speaker) to cancel the accoustic echo.
        * Note that the implementation is quite tiny.
          You need to replace high quality implementation. See the .cpp, you need to define the macro to disable the default implementations.
          * ```USE_TINY_AEC_IMPL 0```
    * PipeMixer
      * Mixing pipe output.
      * This provides ```SinkAdapator``` for the Pipe's Sink.
      * And this output to Sink.
        * Note that the implementation is quite tiny.
          You need to replace high quality implementation. See the .cpp, you need to define the macro to disable the default implementations.
          * ```USE_TINY_MIXER_IMPL 0```
          * ```USE_TINY_MIXER_PRIMITIVE_IMPL 0```
    * MixerSplitter
      * This enables flexible signal flow.
        * case 1: Mapping specified Pipe to Sink
          * Pipe1 ---> Sink1
          * Pipe2 ---> Sink2
        * case 2: Mix pipe outputs and output to specified Sink
          * Pipe1+Pipe2 --> Sink2
        * case 3: depending on the AudioFormat, change the Sink
          * Pipe1+Pipe2 if PCM ----> Sink1
          * Pipe2 --ifCompressed --> Sink2
      * You can specify mapping (SinkAdaptor-Sink) and mapping condition with ```MapCondition``` dynamically.
        * Use ```map()```, ```conditionalMap()``` and ```unmap()```
  * Utilities
    * AudioBuffer
      * IAudioBuffer : interface class. The following classes are derived from this.
        * AudioBuffer : Buffer for PCM encoding data
        * CompressedBuffer : Buffer for ES(Compressed) data
    * AudioFormatAdaptor
      * PCM Format Conversion
        * Convertable bi-directltionally.
          * Encoding format : 8Bit, 16bit, 24bit, 32bit, float
          * Sampling rate conversion
          * Channel conversion.
        * Note that those implementations are quite tiny.
          You need to replace high quality implementation. See the .cpp, you need to define the macro to disable the default implementations.
          * ```USE_TINY_CC_IMPL 0```
          * ```USE_TINY_SRC_IMPL 0```
    * Volume
      * Note that the implementation is quite tiny.
      You need to replace high quality implementation. See the .cpp, you need to define the macro to disable the default implementations.
        * ```USE_TINY_VOLUME_PRIMITIVE_IMPL 0```

    * Stream
      * Abstraction of input/out from the others
        * e.g. FileStream can provide file input/output.
      * StreamSource : Work as Source but the source data is provided by attched Stream.
      * StreamSink : Work as Sink but the sink data is stored to attached Stream.

    * ParameterManager
      * This provides registry (parameter database).
      * You can subscribe the value change.
        * Key exact match e.g. "paramA"
        * Key wild card case e.g. "param*"
      * read only parameter. e.g. "ro.paramA"
    * Resource Manager
      * You need to report actual computing resource which is expected to measure at runtime.
      You can find dummy implementation as follows:
        ```__USE_DUMMY_CPU_RESOURCE_IMPL_ 0```
        ```
        #ifndef CPU_RESOURCE_VALUE
        #define CPU_RESOURCE_VALUE 1000000 // 1000DMIPS * 1000
        #endif /* CPU_RESOURCE_VALUE */

        int CpuResource::getComputingResource(void)
        {
          return CPU_RESOURCE_VALUE;
        }
        ```
    * Test foundation
      * ICapture
        * This enables to capture the result without any impact to the Source, Sink, Filter which implements this.
        * You can capture and check the result to verify what's going on.
        * SourceCapture : Capture the read source's data.
        * SinkCapture : Capture the data written to the Sink.
        * FilterCapture : Capture Filter's output data.
      * IInjector
        * This enables to inject data instead of the actual data without any impact to the Source, Sink, Filter which implements this.
        * You can inject data even to verify behavior with the injected data.
        * SourceInjector : Instead of actual Source Data, inject data is read by Pipe.
        * SinkInjector : Instead of written Sink data, injected data is output to actual Sink.
        * FilterInjector : Instead of signal processed data, inject data is used as the filter output.

 * Test case framework is gtest.
    * TestCase_Common
      * Define the common classes for testcases
        * Source
          * class TestSource : public Source
          * class SinSource : public TestSource
          * class CompressedSource : public Source
        * Sink
          * class TestSink : public Sink
          * class CompressedSink : public Sink
          * class HdmiAudioSink : public CompressedSink
          * class SpdifSink : public CompressedSink
          * class LPcmSink : public Sink
          * class SpeakerSink : public LPcmSink
          * class HeadphoneSink : public LPcmSink
          * class BluetoothAudioSink : public LPcmSink
          * class HQSpeakerSink : public SpeakerSink
          * class SinkFactory
          * class OutputManager : public MultipleSink
        * ExampleFilters for test cases
          * class VirtualizerA : public Filter
          * class VirtualizerB : public Filter
          * class VirtualizerC : public Filter
          * class FilterReverb : public Filter
          * class SpeakerProtectionFilter : public Filter
        * Strategy
          * class TunnelPlaybackContext : public StrategyContext
          * class TunnelPlaybackStrategy : public IStrategy
    * TestCase_PipeAndFilter
      * Basic Pipe, Source, Sink setup.
      * Basic PipeMultiThread, Source, Sink.
      * MultiSink
      * StreamSource, StreamSink
      * PipeMixer, MixerSplit
      * Decoder, Encoder
      * EncodedSink with transcoder/passthrough
      * Format Conversion
        * Encoding format conversion
        * Sampling rate conversion
        * Channel conversion
      * Filters
        * Example: FilterIncrement
        * DelayFilter, AccousticEchoCancelFilter
    * TestCase_TestFoundation
      * Capture, Injection
      * for Source, Sink, Filter
    * TestCase_Output
      * Output (Sink) related test cases
    * TestCase_DynamicSignalFlow
      * Add/Change/Remove filter
      * Attach/Detach Sink, Source
    * TestCase_System
      * PowerManagement
      * Property
      * PlugIn
      * ResourceManager
      * Starategy
    * TestCas_Util
      * StringTokenizer
      * FifoBuffer

# Status

under developing. It's in quite early stage.

# Build

```
$ make -j 4; ./bin/afw_test;

```

| make target | description |
| :--- | :--- |
| ```make``` | build test case executable (```bin/afwtest```) |
| ```make afw``` | build ```lib/libafw.a``` for static link library |
| ```make afwshared``` | build ```lib/libafw.so``` (or ```.dylib```) for dynamic link library |
| ```make test``` | build test case executable (```bin/test_with_afwlib```) (```libafw.a``` required) |
| ```make testshared``` | build (```bin/test_with_afwlib_so```) (```lib/libafw.so(.dylib)``` required) |
| ```make fdk``` | build (```bin/fdk_exec```) (```lib/libafw.so(.dylib)``` required) |
| ```make filterexample``` | build (```lib/filter-plugin/libfilter_example.so(.dylib)```) (```lib/libafw.so(.dylib)``` required) |
| ```make sourceexample``` | build (```lib/source-plugin/libfilter_example.so(.dylib)```) (```lib/libafw.so(.dylib)``` required) |
| ```make sinkexample``` | build (```lib/sink-plugin/libfilter_example.so(.dylib)```) (```lib/libafw.so(.dylib)``` required) |
| ```make codecexample``` | build (```lib/codec-plugin/libfilter_example.so(.dylib)```) (```lib/libafw.so(.dylib)``` required) |

* If you want to use dynamic library based development, the following is expected make and execution sequence.
```
$ make afwshared -j 4
$ make fdk filterexample testshared -j 4
$ ./bin/test_with_afwlib_so
$ ./bin/fdk_exec -f lib/filter-plugin/libfilter_example.so
```

# External Dependencies

* gtest : https://github.com/google/googletest.git


# Plans to implement

* Build
  * [done] Separate test and afw in Makefile
  * [done] Support -j option
  * [done] Filter, Source, Sink, Codec development kit
    * [done] filter development kit (fdk/)
    ```
    $ make afwshared -j4 # this required once
    $ make fdk -j 4  # this required once
    ```
    * [done] filter example (example_filter/)
    ```
    $ make filterexample -j 4
    $ bin/fdk_exec -f lib/filter-plugin -p "filter.exampleReverb.power=1;filter.exampleReverb.delay=5"
    ```
    * [done] source example (example_source/)
    ```
    $ make sourceexample -j 4
    $ bin/fdk_exec -u lib/source-plugin
    ```
    * [done] sink example (example_source/)
    ```
    $ make sinkexample -j 4
    $ bin/fdk_exec -s lib/sink-plugin
    ```
    * [done] codec example (example_codec/) (decoder)
    ```
    $ make codecexample -j 4
    $ bin/fdk_exec -d lib/codec-plugin -i esdata
    ```
    Note that specifying -i is required to test decoder plug-in since decoder require CompressedBuffer, not AudioBuffer(=PCM buffer) instance.

* Filter example
  * [done] Add reverb with ParameterManager (FilterReverb in TestCase_Common)
  * [] Add volume filter with ParameterManager
  * [] Add delay filter with ParameterManager
    * [done] Add simple delay filter
    * [done] Add per-channel delay filter
* Pipe
  * [done] window size LCM based Pipe
  * [done] min window size based Pipe
  * [done] PipeMixer
  * [done] Add get latency
* Sink
  * [done] StreamSink : output to the stream
  * [done] MultiSink
    * [done] getLatencyUSec as maximum of registered Sinks.
    * [done] Add latency adjustment among registered Sinks with delay filter
  * [done] Add filter enabled sink (PipedSink)
  * [done] Add volume (setVolume() in ISink)
  * [done] Per-channel Volume in ISink()
  * [done] Per-channel Volume in MultipleSink()
  * [done] Add get latency
  * [done] EncoderSink
    * [done] pass through
    * [done] transcode
      * [done] decoder only case
      * [done] encoder only case
      * [done] decoder+encoder only case
      * [done] PCM format conversion case
* Source
  * [done] StreamSource : input from the stream
  * [done] Add get latency
  * [done] Add filter enabled source (PipedSource)
  * [done] Add RefereceSoundSource
  * [done] Accoustic Echo Cancelled Source
    * [done] Delay Adjustment between Source and Reference Sound Sink
    * [done] Add tiny AEC filter & implement in the AEC-ed Source
  * [done] Add SourceExample : Sin curve source
* Util
  * [done] PCM encoding converter
  * [done] Sampling rate converter
  * [done] Channel converter
  * [done] Volume
  * [done] Per-channel Volume
  * [done] FileStream
  * [done] InterPipeBridge (FIFOed Source and Sink)
  * [done] Per-channel demuxer (ChannelDemuxer)
  * [done] Channel muxer (ChannelMuxer)
  * ParameterManager
    * [done] basic set/get & readonly, pub/sub with wild card
    * [done] parameter hierachy support
    * [done] persist support
    * [done] default value database
  * Plug-in
    * [done] Plug-in manager
    * [done] Filter Plug-in manager
      * Create example filter plug-in
    * [done]Sink Plug-in manager
    * [done]Source Plug-in manager
    * Strategy Plug-in manager
    * Util impl. plug-in : SRC, Encoding, Channel, Mixer, Volume
* ResourceManager
  * [done] CPU resource manager
    * [done] ResourceConsumer with ResourceManager
  * Utilize CPU resource manager in Pipe's filter setup
    * [done] Filter with ResourceConsumer
    * [done] Source with ResourceConsumer
    * [done] Sink with ResourceConsumer
  * Utilize CPU resource manager in Encode/Decoder setup
    * [done] Implement ResourceConsumer in Decoder, Encoder with InterPipeBridge for resource evaluaton by attached Pipe.
* Strategy
  * [] Add dynamic pipe change
    * [done] attach/detach new pipe to pipe mixer dynamically
    * [done] attach/detach new filter to pipe dynamically
    * [done] attach/detach new sink to pipe dynamically
    * [done] attach/detach new source to pipe dynamically
    * [done] Mute in Sink
    * [done] Mute in Source
    * [done] Mute in Pipe
    * [done] attach new Sink To ReferenceSoundSink for AecSinkSource
  * PowerManagement
    * Suspend/Resume/Shutdown/Cold Boot, etc.
      * [done] PowerManager
        * PowerManagerPrimitive
          * [done] Linux resume monitor from suspend
      * the handler in the example strategy
* StreamManager
  * [done] id, request for the stream, stream(=the pipe) which includes the source, the filters and the sink.
* Testability support
  * [done] dump
  * [done] file in/out with StreamFile and SteramSource/StreamSink
  * Injection/Capture support
    * [Done] SinkCapture
    * [Done] SinkInjector
    * [Done] SourceInjector
    * [Done] SourceCapture
    * [Done] FilterInjector
    * [Done] FilterCapture
  * [done] Split test cases as several files per component characteristic
* All
  * [done] Compressed data support
    * [done] AudioFormat
    * [done] IAudioBufer, CompressedAudioBuffer
    * [done] Sink, Source
    * [done] Decoder
    * [done] Encoder
    * [done] Player (ES controller)
  * [] A/V sync support
    * [] Tiny speed controller (with TinySRC)
* selectPrentation
* OSAL
  * Example Sink for ALSA and/or MacOSX
  * Example Source for ALSA and/or MacOSX


# Build environment setup

## For MacOSX

```
$ brew install ccache
```

* install gtest

```
$ brew install cmake
$ git clone https://github.com/google/googletest.git
$ mkdir build
$ cd build
$ cmake
```

And you should install gtest to the library path.


## For Ubuntu 20.04LTS

```
$ sudo apt-get install -y git build-essential ccache clang-11 googletest libgtest-dev
```

