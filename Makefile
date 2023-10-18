CC=gcc
C_FLAGS=-Wall -fPIC
INCLUDE_PATH= -Iexample
C_SOURCES=$(shell find example/ -type f -iname "*.c")
C_TEST_SOURCES=$(shell find tests/ -type f -iname "*.c")
C_OBJECTS=$(foreach x, $(basename $(C_SOURCES)), build/$(x).o)
C_TEST_OBJECTS=$(foreach x, $(basename $(C_TEST_SOURCES)), build/$(x))

# Builds the example library
build_all: build/libExample.a $(C_TEST_OBJECTS)

# Builds and runs the tests
test: build_all
	build/tests/test

build/libExample.a: prepare $(C_OBJECTS)
	ar rcs $@ $(C_OBJECTS) 

build/%.o : %.c
	$(CC) $(C_FLAGS) $(INCLUDE_PATH) -c $< -o $@

build/tests/%: tests/%.c
	$(CC) $(C_FLAGS) $(INCLUDE_PATH) -g -Itests -Lbuild $< -o $@ -lExample

prepare:
	mkdir -p build/example
	mkdir -p build/tests

clean:
	rm -r build



