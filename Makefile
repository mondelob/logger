# Makefile
# 2017-11-30

# Compiler
CC=gcc
CC_FLAGS=-Wall -std=c99 -pthread -lm
CC_DEPS=-Ilib
CC_LIBRARY=-c
CC_OUTPUT=-o

# Targets
LIBRARY=obj obj/yalog.o
TARGETS=bin obj obj/test.o bin/test

# Rules
lib: $(LIBRARY)
all: $(TARGETS)

obj:
	@mkdir -p obj/

bin:
	@mkdir -p bin/

obj/yalog.o: lib/yalog.c
	$(CC) $(CC_LIBRARY) $(CC_OUTPUT) $@ $< $(CC_FLAGS)

obj/test.o: test.c
	$(CC) $(CC_DEPS) $(CC_FLAGS) $(CC_LIBRARY) $(CC_OUTPUT) $@ $<

bin/test: obj/test.o obj/yalog.o
	$(CC) $(CC_OUTPUT) $@ $? $(CC_FLAGS)

clean:
	@rm -rf bin/
	@rm -rf obj/
