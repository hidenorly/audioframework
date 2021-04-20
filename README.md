# Audio framework

* C++17 based audio framework
  * Pipe and filter


# Status

under developing. It's in quite early stage.

# Tentative way to build

```
$ g++ -std=c++17 -stdlib=libc++ -v Pipe.cpp Buffer.cpp TestCase_PipeAndFilter.cpp  -lgtest_main -lgtest; ./a.out
```

* [] TODO: Makefile will be prepared.

# External Dependencies

* gtest : https://github.com/google/googletest.git