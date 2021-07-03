# Audio framework

This is intending to establish audio framework.
The expection is to implement android audio hal, surround amplifier, mediaplayer, etc. with this framework.

* C++20 based audio framework
  * Pipe and filter
    * Source
      * Interface is ISoure
      * Require to create instance by ```new``` and dispose it by ```delete```
    * Sink
      * Interface is ISink
      * Require to create instance by ```new``` and dispose it by ```delete```
      * MultipleSink : For multiple output
    * Filter
      * The instance is attachable to Pipe
      * Pipe::clearFilters() will dispose the attached instances.
      * Accoustic Echo Cancel Filter
        * Note that the implementation is quite tiny.
          You need to replace high quality implementation. See the .cpp, you need to define the macro to disable the default implementations.
          * ```USE_TINY_AEC_IMPL 0```
    * Pipes
      * Different window size filters are supported.
        * LCM window size processing by Pipe
        * Minimum window size processing by PipeMultiThread which is multi threads & FIFO buffer connected among them.
          * Same window size is running in same thread
          * But the different window size will create different pipe and interconnected by FiFO Buffers automatically
      * PipeMixer
        * Mixing pipe output.
        * You can get the SinkAdaptor. And you can attach the SinkAdaptor instance to your Pipe.
        * You can attach Sink to output the mixed result.
        * Note that the implementation is quite tiny.
          You need to replace high quality implementation. See the .cpp, you need to define the macro to disable the default implementations.
          * ```USE_TINY_MIXER_IMPL 0```
          * ```USE_TINY_MIXER_PRIMITIVE_IMPL 0```
  * Utilities
    * AudioBuffer
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

 * Test case framework is gtest.
    * TestCase_PipeAndFilter
      * Basic Pipe, Source, Sink setup.
      * Basic PipeMultiThread, Source, Sink.
      * MultiSink
      * StreamSource, StreamSink
    * Split test cases as several files per component characteristic

# Status

under developing. It's in quite early stage.

# Build

```
$ make; ./bin/afw_test;

```

# External Dependencies

* gtest : https://github.com/google/googletest.git


# Plans to implement

* Build
  * [partially done] Separate test and afw in Makefile : partially done. ```make``` equals ```make afw; make test;```.
  * [done] Support -j option
  * [] Filter, Source, Sink development kit
* Filter example
  * [] Add reverb with ParameterManager
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
  * [] Add SourceExample : Sin curve source
* Util
  * [done] PCM encoding converter
  * [done] Sampling rate converter
  * [done] Channel converter
  * [done] Volume
  * [done] Per-channel Volume
  * [done] FileStream
  * [done] InterPipeBridge (FIFOed Source and Sink)
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

## For Ubuntu 20.04LTS

```
$ sudo apt-get install -y git build-essential clang-11 googletest libgtest-dev
```

