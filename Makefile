CC=gcc
CPP=g++
C_FLAGS=-Wall -fPIC
INCLUDE_PATH= -Iexample
C_SOURCES=$(shell find example/ -type f -iname "*.c" -o -iname "*.cpp")
C_TEST_SOURCES=$(shell find tests/ -type f -iname "*.c" -o -iname "*.cpp")
C_OBJECTS=$(foreach x, $(basename $(C_SOURCES)), build/$(x).o)
C_TEST_OBJECTS=$(foreach x, $(basename $(C_TEST_SOURCES)), build/$(x))

# Builds the example library
build: build/libExample.a

# Builds tests
build-all: build/libExampleTest.a $(C_TEST_OBJECTS)

# Runs tests
test: build-all
	build/tests/test

build/libExample.a: prepare $(C_OBJECTS)
	ar rcs $@ $(C_OBJECTS)

build/libExampleTest.a: build/libExample.a

build/%Cpp.o : %Cpp.cpp
	$(CPP) $(C_FLAGS) $(INCLUDE_PATH) -c $< -o $@

build/%.o : %.c
	$(CC) $(C_FLAGS) $(INCLUDE_PATH) -c $< -o $@

build/mocks.c: build/tests/test
	build/tests/test --generate-mocks

build/tests/test: tests/test.c
	$(CC) $(C_FLAGS) $(INCLUDE_PATH) -g -Itests $< -o $@

build/tests/%: tests/%.c build/mocks.c
	$(CC) $(C_FLAGS) $(INCLUDE_PATH) -g -Itests -Lbuild build/mocks.c $< -o $@ -lExampleTest

build/tests/%: tests/%.cpp build/mocks.c
	$(CPP) $(C_FLAGS) $(INCLUDE_PATH) -g -Itests -Lbuild build/mocks.c $< -o $@ -lExampleTest

prepare:
	mkdir -p build/example
	mkdir -p build/tests

clean:
	rm -r build



