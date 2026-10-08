# Set C standard according to compiler version
CC = gcc
CXX = g++
USE_C23 := $(shell echo "int main(void) { return 0; }" | $(CC) -std=c23 -x c - -o /dev/null 2>/dev/null; echo $$?)

ifeq ($(USE_C23),0)
	CSTD := c23
else
	CSTD := c2x
endif


# This is a hack to detect whether the compiler accepts the -fopenmp flag for OpenMP directives
# Apple's Clang does not while GCC/LLVM do, so this avoids any Apple headaches and simply
# compiles without OpenMP directives. Note the last echo won't run if the compiler command fails.
OPEN_MP := $(shell echo "int main(void) { return 0; }" | $(CC) -fopenmp -x c - -o /dev/null 2>/dev/null && echo "-fopenmp" )

CFLAGS = -std=$(CSTD) -Wall -Wextra -Werror -pedantic-errors $(OPEN_MP) -g
CXXFLAGS = -std=c++17 -Wall -Wextra -Werror -pedantic-errors -g
CPPFLAGS = -Isrc -Itests
LDLIBS = -lm

APP_TARGET = lin
TEST_TARGET = lin_test
TEST_LINALG_TARGET = linalg_test
TEST_MEMORY = "tests/memory/memorycheck.sh"
COMPILE_DB = compile_commands.json

SRC_DIR = src
LINALG_SRC_DIR = $(SRC_DIR)/linalg
TEST_DIR = tests
TEST_LINALG_DIR = $(TEST_DIR)/linalg
OBJ_DIR = build

APP_SRCS := $(shell find $(SRC_DIR) -type f -name '*.c')
APP_OBJS := $(patsubst $(SRC_DIR)/%.c,$(OBJ_DIR)/%.o,$(APP_SRCS))
APP_OBJS_MAINLESS := $(filter-out $(OBJ_DIR)/main.o,$(APP_OBJS))

LINALG_SRCS := $(shell find $(LINALG_SRC_DIR) -type f -name '*.c')
LINALG_OBJS := $(patsubst $(SRC_DIR)/%.c,$(OBJ_DIR)/%.o,$(LINALG_SRCS))

TEST_SRCS := $(wildcard $(TEST_DIR)/*.c) # Note tests/ doesn't a nested structure
TEST_OBJS := $(TEST_SRCS:$(TEST_DIR)/%.c=$(OBJ_DIR)/%.o)

TEST_LINALG_SRCS := $(wildcard $(TEST_LINALG_DIR)/*.cpp)
TEST_LINALG_OBJS := $(patsubst $(TEST_LINALG_DIR)/%.cpp,$(OBJ_DIR)/linalg_tests/%.o,$(TEST_LINALG_SRCS))

COMPILE.c = $(CC) $(CPPFLAGS) $(CFLAGS)
COMPILE.cpp = $(CXX) $(CPPFLAGS) $(CXXFLAGS)
LINK = $(CC)
CPP_LINK = $(CXX)

all: $(APP_TARGET) $(COMPILE_DB)

test: $(TEST_TARGET)
	./$(TEST_TARGET)

test-linalg: $(TEST_LINALG_TARGET)
	./$(TEST_LINALG_TARGET)

memcheck: $(APP_TARGET)
	./$(TEST_MEMORY)

.PHONY: all clean test test-linalg memcheck

# APP BUILD
$(APP_TARGET): $(APP_OBJS)
	$(LINK) $(CFLAGS) -o $@ $^ $(LDLIBS)

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c
	mkdir -p $(dir $@)
	$(COMPILE.c) -MMD -MP -c $< -o $@

# TEST BUILD
$(TEST_TARGET): $(TEST_OBJS) $(APP_OBJS_MAINLESS)
	$(LINK) $(CFLAGS) -o $@ $^ $(LDLIBS)

$(OBJ_DIR)/%.o: $(TEST_DIR)/%.c
	mkdir -p $(OBJ_DIR)
	$(COMPILE.c) -MMD -MP -c $< -o $@

# TEST LINALG BUILD
$(TEST_LINALG_TARGET): $(TEST_LINALG_OBJS) $(LINALG_OBJS)
	$(CPP_LINK) $(CXXFLAGS) $(OPEN_MP) -o $@ $^ $(LDLIBS)

$(OBJ_DIR)/linalg_tests/%.o: $(TEST_LINALG_DIR)/%.cpp
	mkdir -p $(OBJ_DIR)/linalg_tests
	$(COMPILE.cpp) -MMD -MP -c $< -o $@


# Compile database
$(COMPILE_DB):
	# compile_db.py needs target .json file, curdir, compile command, list of source files
	python3 compile_db.py \
		--filename "$@" \
		--curdir "$(CURDIR)" \
		--compile-cmd "$(COMPILE.c)" \
		--srcs $(APP_SRCS)
	# Append compile entries for the lin tests 
	python3 compile_db.py \
		--filename "$@" \
		--curdir "$(CURDIR)" \
		--compile-cmd "$(COMPILE.c)" \
		--srcs $(TEST_SRCS) \
		--append
	# Append compile entries for linalg tests	
	python3 compile_db.py \
		--filename "$@" \
		--curdir "$(CURDIR)" \
		--compile-cmd "$(COMPILE.cpp)" \
		--srcs $(TEST_LINALG_SRCS) \
		--append

clean:
	rm -rf $(OBJ_DIR) $(APP_TARGET) $(TEST_TARGET) $(TEST_LINALG_TARGET)

# Include compile-time generated dependencies
-include $(APP_OBJS:.o=.d)
-include $(TEST_OBJS:.o=.d)
-include $(TEST_LINALG_OBJS:.o=.d)
-include $(LINALG_OBJS:.o=.d)
