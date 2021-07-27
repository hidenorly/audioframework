# compiler env.
UNAME := $(shell uname -s)
ifeq ($(UNAME),Linux)
	CXX=ccache clang++
	LDLIBS=-ldl
endif
ifeq ($(UNAME),Darwin)
	CXX=ccache clang++
	LDLIBS=-stdlib=libc++
endif

CXXFLAGS=-std=c++2a -MMD -MP -Wall
LDFLAGS=-pthread

# project config
SRC_DIR ?= ./src
INC_DIR=./include
TEST_DIR=./test
FDK_DIR=./fdk
EXF_DIR=./example_filter
EXI_DIR=./example_source
EXO_DIR=./example_sink
LIB_DIR=./lib
LIB_FILTER_DIR=$(LIB_DIR)/filter-plugin
LIB_SOURCE_DIR=$(LIB_DIR)/source-plugin
LIB_SINK_DIR=$(LIB_DIR)/sink-plugin
BIN_DIR=./bin
OBJ_DIR=./out

# --- source code config --------------
INCS = $(wildcard $(INC_DIR)/*.hpp)

AFW_SRCS = $(wildcard $(SRC_DIR)/*.cpp)
TEST_SRCS = $(wildcard $(TEST_DIR)/*.cpp)
INTEG_SRCS = $(AFW_SRCS) $(TEST_SRCS)
FDK_SRCS = $(wildcard $(FDK_DIR)/*.cpp)
FEX_SRCS = $(wildcard $(EXF_DIR)/*.cpp)
EXI_SRCS = $(wildcard $(EXI_DIR)/*.cpp)
EXO_SRCS = $(wildcard $(EXO_DIR)/*.cpp)

# --- the object files config --------------
AFW_OBJS = $(addprefix $(OBJ_DIR)/, $(notdir $(AFW_SRCS:.cpp=.o)))
TEST_OBJS = $(addprefix $(OBJ_DIR)/, $(notdir $(TEST_SRCS:.cpp=.o)))
INTEG_OBJS = $(addprefix $(OBJ_DIR)/, $(notdir $(INTEG_SRCS:.cpp=.o)))
FDK_OBJS = $(addprefix $(OBJ_DIR)/, $(notdir $(FDK_SRCS:.cpp=.o)))
FEX_OBJS = $(addprefix $(OBJ_DIR)/, $(notdir $(FEX_SRCS:.cpp=.o)))
EXI_OBJS = $(addprefix $(OBJ_DIR)/, $(notdir $(EXI_SRCS:.cpp=.o)))
EXO_OBJS = $(addprefix $(OBJ_DIR)/, $(notdir $(EXO_SRCS:.cpp=.o)))

# --- build gtest (integrated) --------
INTEG_TARGET = $(BIN_DIR)/afw_test
INTEG_DEPS = $(INTEG_OBJS:.o=.d)

default: $(INTEG_TARGET)
.PHONY: default

#$(INTEG_TARGET): $(INTEG_SRCS)
#	@[ -d $(BIN_DIR) ] || mkdir -p $(BIN_DIR)
#	$(CXX) $(CXXFLAGS) -I$(INC_DIR) $(LDLIBS) -o $@ $^ -lgtest_main -lgtest

$(INTEG_TARGET): $(INTEG_OBJS)
	@[ -d $(BIN_DIR) ] || mkdir -p $(BIN_DIR)
	$(CXX) $(LDFLAGS) $(INTEG_OBJS) -o $@ $(LDLIBS) -lgtest_main -lgtest

$(AFW_OBJS): $(AFW_SRCS)
	@[ -d $(OBJ_DIR) ] || mkdir -p $(OBJ_DIR)
	$(CXX) $(CXXFLAGS) -I $(INC_DIR) -c src/$(notdir $(@:.o=.cpp)) -o $@

$(TEST_OBJS): $(TEST_SRCS)
	@[ -d $(OBJ_DIR) ] || mkdir -p $(OBJ_DIR)
	$(CXX) $(CXXFLAGS) -I $(INC_DIR) -c test/$(notdir $(@:.o=.cpp)) -o $@


-include $(INTEG_DEPS)


# --- Build for AFW -------------------
AFW_TARGET = $(LIB_DIR)/libafw.a
AFW_DEPS = $(AFW_OBJS:.o=.d)

afw: $(AFW_TARGET)
.PHONY: afw
CXXFLAGS+= -fPIC #-flto=full

$(AFW_TARGET): $(AFW_OBJS)
	@[ -d $(LIB_DIR) ] || mkdir -p $(LIB_DIR)
	ar rs $(AFW_TARGET) $(AFW_OBJS)
	ranlib -c $(AFW_TARGET)
#	$(AR) rvs $(AFW_TARGET) $(AFW_OBJS)
#	ranlib -c $@

-include $(AFW_DEPS)


# --- Build for AFW(shared) ------------
UNAME := $(shell uname -s)
ifeq ($(UNAME),Linux)
	AFW_SO_TARGET = $(LIB_DIR)/libafw.so
	SHARED_CXXFLAGS= -fPIC -shared
endif
ifeq ($(UNAME),Darwin)
	AFW_SO_TARGET = $(LIB_DIR)/libafw.dylib
	SHARED_CXXFLAGS= -flat_namespace -dynamiclib
endif

afwshared: $(AFW_SO_TARGET)
.PHONY: afwshared

$(AFW_SO_TARGET): $(AFW_OBJS)
	@[ -d $(LIB_DIR) ] || mkdir -p $(LIB_DIR)
	$(CXX) $(LDFLAGS) $(AFW_OBJS) $(SHARED_CXXFLAGS) -o $@ $(LDLIBS)

# --- Build for test cases w/libafw.a ---
TEST_TARGET = $(BIN_DIR)/test_with_afwlib
TEST_LDLIBS = $(LDLIBS) -L$(LIB_DIR)
TEST_LIBS = $(AFW_TARGET)

test: $(TEST_TARGET)
.PHONY: test

$(TEST_TARGET): $(TEST_OBJS)
	@[ -d $(BIN_DIR) ] || mkdir -p $(BIN_DIR)
	$(CXX) $(LDFLAGS) $(TEST_LDLIBS) $(TEST_OBJS) $(TEST_LIBS) -o $@ -lgtest_main -lgtest

# --- Build for test cases w/libafw.so ---
TEST_SHARED_TARGET = $(BIN_DIR)/test_with_afwlib_so
TEST_LDLIBS = $(LDLIBS) -L$(LIB_DIR)
TEST_LIBS = $(AFW_SO_TARGET)

testshared: $(TEST_SHARED_TARGET)
.PHONY: testshared

$(TEST_SHARED_TARGET): $(TEST_OBJS)
	@[ -d $(BIN_DIR) ] || mkdir -p $(BIN_DIR)
	$(CXX) $(LDFLAGS) $(TEST_LDLIBS) $(TEST_OBJS) $(TEST_LIBS) -o $@ -lgtest_main -lgtest


# --- Build for FDK w/libafw.a ------------
FDK_TARGET = $(BIN_DIR)/fdk_exec
FDK_LDLIBS = $(LDLIBS) -L$(LIB_DIR)
FDK_LIBS = $(AFW_SO_TARGET)
#FDK_LIBS = $(AFW_TARGET)
FDK_DEPS = $(FDK_OBJS:.o=.d)

fdk: $(FDK_TARGET)
.PHONY: fdk

$(FDK_TARGET): $(FDK_OBJS)
	@[ -d $(BIN_DIR) ] || mkdir -p $(BIN_DIR)
	$(CXX) $(LDFLAGS) $(FDK_LDLIBS) $(FDK_OBJS) $(FDK_LIBS) -o $@

$(FDK_OBJS): $(FDK_SRCS)
	@[ -d $(OBJ_DIR) ] || mkdir -p $(OBJ_DIR)
	$(CXX) $(CXXFLAGS) -I $(INC_DIR) -c fdk/$(notdir $(@:.o=.cpp)) -o $@

-include $(FDK_DEPS)


# --- Build for filter example(shared) ------------
UNAME := $(shell uname -s)
ifeq ($(UNAME),Linux)
	FEX_SO_TARGET = $(LIB_FILTER_DIR)/libfilter_example.so
	SHARED_CXXFLAGS= -fPIC -shared
endif
ifeq ($(UNAME),Darwin)
	FEX_SO_TARGET = $(LIB_FILTER_DIR)/libfilter_example.dylib
	SHARED_CXXFLAGS= -flat_namespace -dynamiclib
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

-include $(FEX_DEPS)


# --- Build for source example(shared) ------------
UNAME := $(shell uname -s)
ifeq ($(UNAME),Linux)
	EXI_SO_TARGET = $(LIB_SOURCE_DIR)/libsource_example.so
	SHARED_CXXFLAGS= -fPIC -shared
endif
ifeq ($(UNAME),Darwin)
	EXI_SO_TARGET = $(LIB_SOURCE_DIR)/libsource_example.dylib
	SHARED_CXXFLAGS= -flat_namespace -dynamiclib
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

-include $(EXI_DEPS)

# --- Build for sink example(shared) ------------
UNAME := $(shell uname -s)
ifeq ($(UNAME),Linux)
	EXO_SO_TARGET = $(LIB_SINK_DIR)/libsink_example.so
	SHARED_CXXFLAGS= -fPIC -shared
endif
ifeq ($(UNAME),Darwin)
	EXO_SO_TARGET = $(LIB_SINK_DIR)/libsink_example.dylib
	SHARED_CXXFLAGS= -flat_namespace -dynamiclib
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

-include $(EXO_DEPS)


# --- clean up ------------------------
clean:
	rm -f $(TARGET) $(TEST_TARGET) $(INTEG_TARGET) $(OBJS) $(AFW_DEPS) $(TEST_OBJS) $(INTEG_OBJS) $(INTEG_DEPS) $(FDK_OBJS) $(FDK_DEPS) $(FEX_OBJS) $(FEX_DEPS)
