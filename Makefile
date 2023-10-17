CC=gcc
C_FLAGS=-Wall -fPIC
INCLUDE_PATH= -Isource
C_SOURCES=$(shell find source/ -type f -iname "*.c")
C_TEST_SOURCES=$(shell find tests/ -type f -iname "*.c")
C_OBJECTS=$(foreach x, $(basename $(C_SOURCES)), build/$(x).o)
C_TEST_OBJECTS=$(foreach x, $(basename $(C_TEST_SOURCES)), build/$(x))

# Builds the example library
lib: build/libExample.a

# Builds and runs the tests
test: build/libExample.a $(C_TEST_OBJECTS)
	build/tests/test

build/libExample.a: prepare $(C_OBJECTS)
	ar rcs $@ $(C_OBJECTS) 

build/%.o : %.c
	$(CC) $(C_FLAGS) $(INCLUDE_PATH) -c $< -o $@

build/tests/%: tests/%.c
	$(CC) $(C_FLAGS) $(INCLUDE_PATH) -g -Itests -Lbuild $< -o $@ -lExample

prepare:
	mkdir -p build/source
	mkdir -p build/tests

clean:
	rm -r build



