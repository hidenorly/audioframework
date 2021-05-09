# compiler env.
UNAME := $(shell uname -s)
ifeq ($(UNAME),Linux)
	CXX=g++
endif
ifeq ($(UNAME),Darwin)
	CXX=clang++
endif

CXXFLAGS := -std=c++2a -pthread -v
LDLIBS := -stdlib=libc++

# project config
SRC_DIR=./src
INC_DIR=./include
TEST_DIR=./test
LIB_DIR=./lib
BIN_DIR=./bin
OBJ_DIR=./out

# --- source code config --------------
INCS = $(wildcard $(INC_DIR)/*.hpp)
SRCS = $(wildcard $(SRC_DIR)/*.cpp)
TEST_SRCS = $(wildcard $(TEST_DIR)/*.cpp)
INTEG_SRCS = $(SRCS) $(TEST_SRCS)

# --- build gtest (integrated) --------
INTEG_TARGET = $(BIN_DIR)/afw_test
INTEG_OBJS = $(addprefix $(OBJ_DIR)/, $(notdir $(INTEG_SRCS:.cpp=.o)))

default: $(INTEG_TARGET)
.PHONY: default

$(INTEG_TARGET): $(INTEG_SRCS)
	$(CXX) $(CXXFLAGS) -I$(INC_DIR) $(LDLIBS) -o $@ $^ -lgtest_main -lgtest


# --- Build for AFW -------------------
AFW_TARGET = $(LIB_DIR)/libafw.a
AFW_OBJS = $(addprefix $(OBJ_DIR)/, $(notdir $(SRCS:.cpp=.o)))

afwlib: $(AFW_TARGET)
.PHONY: afwlib

$(AFW_TARGET): $(AFW_OBJS)
	@[ -d $(LIB_DIR) ] || mkdir -p $(LIB_DIR)
	ar rc $(AFW_TARGET) $(AFW_OBJS)
	ranlib $@
#	$(AR) rvs $(TARGET) $(OBJS)
#	ranlib -c $@

$(AFW_OBJS): $(SRCS)
	@[ -d $(OBJ_DIR) ] || mkdir -p $(OBJ_DIR)
	$(CXX) $(CXXFLAGS) -I$(INC_DIR) $(LDLIBS) -o $@ -c $<


# --- Build for test cases ------------
TEST_TARGET = $(BIN_DIR)/test_with_afwlib
TEST_OBJS  = $(addprefix $(OBJ_DIR)/, $(notdir $(TEST_SRCS:.cpp=.o)))
TEST_LDLIBS = $(LDLIBS) -L$(LIB_DIR)
TEST_LDLIBS += -lafw

test: $(TEST_TARGET)
.PHONY: test

$(TEST_TARGET): $(TEST_SRCS)
	@[ -d $(BIN_DIR) ] || mkdir -p $(BIN_DIR)
	$(CXX) $(LDFLAGS) $(CXXFLAGS) $(TEST_LDLIBS) -I$(INC_DIR) -o $@ -c $< -lgtest_main -lgtest



# --- clean up ------------------------
clean:
	rm -f $(TARGET) $(TEST_TARGET) $(INTEG_TARGET) $(OBJS) $(TEST_OBJS) $(INTEG_OBJS)

