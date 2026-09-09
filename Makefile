# Set C standard according to compiler version
CC = gcc
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
CPPFLAGS = -Isrc
LDLIBS = -lm

APP_TARGET = lin
TEST_TARGET = lin_test
TEST_MEMORY = "tests/memory/memorycheck.sh"
COMPILE_DB = compile_commands.json


SRC_DIR = src
TEST_DIR = tests
OBJ_DIR = build

APP_SRCS := $(shell find $(SRC_DIR) -type f -name '*.c')
APP_OBJS := $(patsubst $(SRC_DIR)/%.c,$(OBJ_DIR)/%.o,$(APP_SRCS))
APP_OBJS_MAINLESS := $(filter-out $(OBJ_DIR)/main.o,$(APP_OBJS))

TEST_SRCS := $(wildcard $(TEST_DIR)/*.c) # Note tests/ doesn't a nested structure
TEST_OBJS := $(TEST_SRCS:$(TEST_DIR)/%.c=$(OBJ_DIR)/%.o)

COMPILE.c = $(CC) $(CPPFLAGS) $(CFLAGS)
LINK = $(CC)

all: $(APP_TARGET) $(COMPILE_DB)
test: $(TEST_TARGET)
	./$(TEST_TARGET)

memcheck: $(APP_TARGET)
	./$(TEST_MEMORY)

.PHONY: all clean test memcheck

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


# Compile database
$(COMPILE_DB):
	# compile_db.py needs target .json file, curdir, compile command, list of source files
	python3 compile_db.py \
		--filename "$@" \
		--curdir "$(CURDIR)" \
		--compile-cmd "$(COMPILE.c)" \
		--srcs $(APP_SRCS)

clean:
	rm -rf $(OBJ_DIR) $(APP_TARGET) $(TEST_TARGET)

# Include compile-time generated dependencies
-include $(APP_OBJS:.o=.d)

