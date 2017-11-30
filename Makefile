# Makefile
# 2017-11-30

# Compiler
CC=gcc
CC_FLAGS=-Wall -std=c99 -pthread
CC_DEPS=-Ilib
CC_LIBRARY=-c
CC_OUTPUT=-o

# Targets
LIBRARY=obj obj/logger.o
TARGETS=bin obj obj/test.o bin/test

# Rules
lib: $(LIBRARY)
all: $(TARGETS)

obj:
	@mkdir -p obj/

bin:
	@mkdir -p bin/

obj/logger.o: lib/logger.c
	$(CC) $(CC_FLAGS) $(CC_LIBRARY) $(CC_OUTPUT) $@ $<

obj/test.o: test.c
	$(CC) $(CC_FLAGS) $(CC_LIBRARY) $(CC_OUTPUT) $@ $<

bin/test: obj/test.o obj/logger.o
	$(CC) $(CC_FLAGS) $(CC_OUTPUT) $@ $<

clean:
	@rm -rf bin/
	@rm -rf obj/
