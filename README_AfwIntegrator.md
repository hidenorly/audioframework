# Expected audience of this document

Expected audience of this document is developer who integrates this audio framework to your application such as Android audio HAL, Player, General Digital Audio Signal Processing Applications.

# How to integrate the audio framework.

Since this C++ based framework, you need to prepare C++ compiler which supports C++20 such as calng++.
And this audio framework provides 2 ways to integrate to your application.

1. Using dynamic link library (shared object) (recommended)
2. Using static link library (static object)


The shared object is so called ```.so``` (```.dylib```).
The static object is ```.a```.

To create shared object, you need to do ```$ make afwshared```
To create static object, you need to do ```$ make afw```

The example makefiles to use shared object is ```make testshared``` and ```make fdk```.
The exmaple makefile to use static object is ```make test```.

# More detail of how to afw.

Since this is C++ based framework, you need to do ```#include "Pipe.hpp"```, etc. in your application.
Therefore the header's path is needed to specify in your applicarion's makefile.

In your Makefile, you need to specift the afw's include path and the library path.

```
AFW_DIR=../audioframework
AFW_INC_DIR=$(AFW_DIR)/include
AFW_LIB_DIR=$(AFW_DIR)/lib
AFW_LIB_DIR=$(AFW_LIB_DIR)/
AFW_SHARED_OBJECT = $(AFW_LIB_DIR)/libafw.so
```

Also you need to specify the include and the library path.


```
$ clang++ -I $(AFW_INC_DIR) $(AFW_SHARED_OBJECT)
```

You can refer to complete separated project's Makefile example is as follows:

* https://github.com/hidenorly/DecoderPlugInFFmpeg : Decoder plug-in with FFmpeg
* https://github.com/hidenorly/SinkPlugInAlsa : Sink plug-in with Alsa
