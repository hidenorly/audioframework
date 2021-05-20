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
    * Pipes
      * Different window size filters are supported.
        * LCM window size processing by Pipe
        * Minimum window size processing by PipeManager with multi threads & FIFO buffer connected among them.
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

 * Test case framework is gtest.
    * TestCase_PipeAndFilter
      * Basic Pipe, Source, Sink setup.
      * Basic PipeManager, Source, Sink.
      * MultiSink
      * StreamSource, StreamSink

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
  * [] Separate test and afw in Makefile : partially done. ```make``` equals ```make afw; make test;```.
  * [] Filter, Source, Sink development kit
* Filter example
  * [] Add reverb with ParameterManager
  * [] Add volume filter with ParameterManager
  * [] Add delay filter with ParameterManager
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
    * Add latency adjustment among registered Sinks with delay filter
  * [done] Add filter enabled sink (PipedSink)
  * [done] Add volume (setVolume() in ISink)
  * [done] Add get latency
* Source
  * [done] StreamSource : input from the stream
  * [done] Add get latency
  * [] Add filter enabled source
  * [] Add Sin curve surce
* Util
  * [done] PCM encoding converter
  * [done] Sampling rate converter
  * [done] Channel converter
  * [done] Volume
  * [done] FileStream
  * [done] InterPipeBridge (FIFOed Source and Sink)
  * ParameterManager
    * [done] basic set/get_ro, pub/sub with wild card
    * [] parameter hierachy support
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
  * CPU resource manager
* Strategy
  * [] Add dynamic pipe change
* All
  * [] Compressed data support
    * [done] AudioFormat
    * [done] IAudioBufer, CompressedAudioBuffer
    * [done] Sink, Source
    * [done] Decoder
    * [done] Encoder
    * [done] Player (ES controller)
  * [] A/V sync support
    * [] Tiny speed controller (with TinySRC)
* OSAL
  * Example Sink for ALSA and/or MacOSX
  * Example Source for ALSA and/or MacOSX
