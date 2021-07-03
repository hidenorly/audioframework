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
LIB_DIR=./lib
BIN_DIR=./bin
OBJ_DIR=./out

# --- source code config --------------
INCS = $(wildcard $(INC_DIR)/*.hpp)

AFW_SRCS = $(wildcard $(SRC_DIR)/*.cpp)
TEST_SRCS = $(wildcard $(TEST_DIR)/*.cpp)
INTEG_SRCS = $(AFW_SRCS) $(TEST_SRCS)

# --- the object files config --------------
AFW_OBJS = $(addprefix $(OBJ_DIR)/, $(notdir $(AFW_SRCS:.cpp=.o)))
TEST_OBJS = $(addprefix $(OBJ_DIR)/, $(notdir $(TEST_SRCS:.cpp=.o)))
INTEG_OBJS = $(addprefix $(OBJ_DIR)/, $(notdir $(INTEG_SRCS:.cpp=.o)))

# --- build gtest (integrated) --------
INTEG_TARGET = $(BIN_DIR)/afw_test
INTEG_DEPS = $(INTEG_OBJS:.o=.d)

default: $(INTEG_TARGET)
.PHONY: default

#$(INTEG_TARGET): $(INTEG_SRCS)
#	@[ -d $(BIN_DIR) ] || mkdir -p $(BIN_DIR)
#	$(CXX) $(CXXFLAGS) -I$(INC_DIR) $(LDLIBS) -o $@ $^ -lgtest_main -lgtest

$(INTEG_TARGET): $(INTEG_OBJS)
	$(CXX) $(LDFLAGS) $(INTEG_OBJS) -o $@ $(LDLIBS) -lgtest_main -lgtest

$(AFW_OBJS): $(AFW_SRCS)
	@if [ ! -d $(OBJ_DIR) ]; \
		then echo "mkdir -p $(OBJ_DIR)"; mkdir -p $(OBJ_DIR); \
		fi
	$(CXX) $(CXXFLAGS) -I $(INC_DIR) -c src/$(notdir $(@:.o=.cpp)) -o $@

$(TEST_OBJS): $(TEST_SRCS)
	@if [ ! -d $(OBJ_DIR) ]; \
		then echo "mkdir -p $(OBJ_DIR)"; mkdir -p $(OBJ_DIR); \
		fi
	$(CXX) $(CXXFLAGS) -I $(INC_DIR) -c test/$(notdir $(@:.o=.cpp)) -o $@


-include $(INTEG_DEPS)


# --- Build for AFW -------------------
AFW_TARGET = $(LIB_DIR)/libafw.a

afw: $(AFW_TARGET)
.PHONY: afw
#CXXFLAGS+= -flto=full

$(AFW_TARGET): $(AFW_OBJS)
	@[ -d $(LIB_DIR) ] || mkdir -p $(LIB_DIR)
	ar rs $(AFW_TARGET) $(AFW_OBJS)
	ranlib -c $(AFW_TARGET)
#	$(AR) rvs $(AFW_TARGET) $(AFW_OBJS)
#	ranlib -c $@


# --- Build for test cases ------------
TEST_TARGET = $(BIN_DIR)/test_with_afwlib
TEST_LDLIBS = $(LDLIBS) -L$(LIB_DIR)
TEST_LDLIBS += -lafw

test: $(TEST_TARGET)
.PHONY: test

$(TEST_TARGET): $(TEST_OBJS)
	@[ -d $(BIN_DIR) ] || mkdir -p $(BIN_DIR)
	$(CXX) $(LDFLAGS) $(TEST_LDLIBS) $(TEST_OBJS) -o $@ -lgtest_main -lgtest


# --- clean up ------------------------
clean:
	rm -f $(TARGET) $(TEST_TARGET) $(INTEG_TARGET) $(OBJS) $(TEST_OBJS) $(INTEG_OBJS) $(INTEG_DEPS)
