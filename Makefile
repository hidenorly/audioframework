PROGRAM := a.out
CXX := clang++
CXXFLAGS := -std=c++2a -v
LDLIBS := -stdlib=libc++ -pthread -lgtest_main -lgtest

SRCS := $(wildcard *.cpp)
OBJS := $(SRCS:.cpp=.o)

## Targets
.DEFAULT_GOAL := all
.PHONY: all clean

all: ${PROGRAM}
	@:

${PROGRAM}: ${OBJS}
	${LINK.cpp} ${OUTPUT_OPTION} $^ ${LDLIBS}

clean:
	${RM} ${OBJS} ${PROGRAM}