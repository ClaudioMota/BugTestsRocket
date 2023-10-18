# 🐛 Tests 🚀

Welcome to your C testing framework built focusing simplicity and readability.

## Main characteristics
- Readable: The syntax was designed to be readable and clean.
- Plug and play: Just copy the `test.h` to your project, follow this guide and be happy (you might also find useful to adapt the Makefile to yours).
- Automatic: Tests are written and executed and that's it. No need to be configuring or tracking test files/functions.
- Simple: It follows the KISS philosophy, keeping the framework to effectively two to three files of reasonable sizes.

## Example
Here is an example of a full test file written using the framework:

```C
#include "test.h"

🐛
context("sum")
{
  test("adds the first to the second")
  {
    assert(sum(2, 1) == 3);
  }
}

context("divide")
{
  test("divides the first by the second")
  {
    assert(divide(6,2) == 3);
  }

  test("performs integer division")
  {
    refute(divide(3,2) != 1);
  }
}
🚀
```

This repo itself is a working example so feel free to clone it and play around!


## Framework structure
For the framework to work, the project should follow some basic guidelines and standards.

- Tests macros are state machines: Once you call a test macro the state is set, so the macros don't follow your scope structure. See `Pitfalls` section.
- Test code != source code: tests may refer to source hearders, but their binaries must not be the same.
- Each test file is a final executable: Test binaries should not be linked to other binaries that implement a main function.
- Test output directory: All tests must be output to a test binary only directory, so that the test runner can detect and run them automatically.
- `tests/test.c` is the runner: This is just a convention, this file will have a main function that will run all tests. See `Tests runner` section.
- 🐛 Tests(only) 🚀: Helper functions must be written outside the 🐛 🚀 guards as the whole framework is just hacky syntax suggar.

### API
The framework provides (through `test.h`) a set of functions and macros for you to write your tests.
The file `test.h` has a naming convention in which identifiers prefixed with underscore (`_`) are meant to be private and used only inside the header itself.
All other public macros and functions are designed to be used by the end user when writing tests.
The available public API is:

```C
// Macro that must be present before the tests definition
#define 🐛 beginTests

// Sets the context for the following tests. It may be useful for describing the function or subsytem being tested
#define context(name) _context = name;

// Macro that sets up the current test. The description should state what the test does.
#define test(description)

// Asserts that a boolean expression is true failling the test otherwise
#define assert(booleanExpr)
// Asserts that a boolean expression is false failling the test otherwise
#define refute(boolean)

// Macro that must be present after tests definition
#define 🚀 endTests

// Holds information about the current state of the test environment
struct TestEnvironment
{
  char* testContext;
  int testIndex;
  char* testDescription;
  int testLine;

  _TestSelect selection;

  // Helpers for instantiating data to tests
  void* helperPointer;
  void* helperBlock[_TEST_HELPER_BLOCK_SIZE];
};

// A function for setting up the test environment before execution.
// May be useful to instatiate helper data that will be used in multiple tests, for example.
void (*setupFunction)(TestEnvironment* env) = _ignore;
// A function for cleaning up the things done in the setupFunction step.
void (*cleanFunction)(TestEnvironment* env) = _ignore;
// Function to be called when a test fail. If changed make sure the new fuction calls `_defaultFailure`.
// May be useful to write to a file or doing some extra processing.
void (*onFail)(TestEnvironment* env, int line, char* expr) = _defaultFailure;
// Function to be called when a test pass. If changed make sure the new fuction calls `_defaultTestPass`.
void (*onTestPass)(TestEnvironment* env) = _defaultTestPass;
// Function to be called when a raise occurs
void (*onRaise)(int) = (void*)_ignore;

// Recursively finds all files inside of the same dir of the file given as input
// As of the convetion of the framework, all file in the test binary dir will be test files
// ignores the testRunner file itself
// Will allocate an array and assign to output. The memory of all indexes and output itself must be freed afterwards
// Returns the number of files found
int findAllTestFiles(char* testRunnerPath, char*** output)

// Runs all tests for a given test executable passing the args given as input
// Returns the number of failed tests
int runTestsForFile(int numArgs, char** args, char* file);

// Searches all tests using findAllTestFiles function and applies runTestsForFile on each of them
// Expects as parameters the same parameters of the main function
// Returns the number of failed tests
// Should be called in `test.c` main function
int runAllTests(int numArgs, char** args);
```

### Tests runner
The test runner is a bootstrap executable that is responsible for running all other tests and making sense of their results.
For standards it is recommended to be the `tests/test.c` file and it should look something like:

```C
#include "test.h"

int main(int numArgs, char** args)
{
  return runAllTests(numArgs, args);
}
```

You can always change it for adding pre/post tests processing or any other functionality your project requires.

### Runner parameters

The accepted parameters for the test runner are the following:

```bash
build/tests/test [options]
--module PARTIAL_PATH_OR_CONTEXT # Runs tests of all matching files/contexts
--line LINE_NUMBER               # Runs all tests that are defined at the given line
PARTIAL_PATH:LINE_NUMBER         # Same as --module PARTIAL_PATH --line LINE_NUMBER       
```

## Building and running this repo
This repo was designed to be a simple yet complete showcase of the framework.
It implements a `libExample` for being used as subject of the tests.

### Dependencies
- make
- gcc
- git

Run the following commands for building and runnning the tests:
```bash
# Downloads the project
git clone https://github.com/ClaudioMota/BugTestsRocket.git
# Sets the working directory
cd BugTestsRocket
# Builds the project and runs the tests
make test
# After that you can run the tests directly by typing:
build/tests/test
```
The output of the tests should be something like:
```bash
build/tests/test
.
[FAIL] on "subtract" test "subtracts the first by the second" failed tests/exampleCalc.c:19 (subtract(1, 2) == -1)
....
[FAIL] on "divRemainder" test "retrieves false on division by zero" failed tests/exampleCalc.c:63 (SIGFPE)
```

## Pitfalls
### Scopes and tests structure
As the test macros are state machines and will be set in any point you call them, any aparent scope structure is just syntax suggar. For example:
```C
#include "test.h"

🐛
context("A")
{
  test("test 1")
  {
    assert(true);
  }

  context("B")
  {
    test("test 2")
    {
      assert(true);
    }
  }

  test("test 3")
  {
    refute(false);
  }
}

🚀
```
Has the same test semantic of:
```C
#include "test.h"

🐛
context("A")
  test("test 1")
    assert(true);
context("B")
  test("test 2")
    refute(false);
  test("test 3")
    assert(true);
🚀
```
So `test 3` is part of context `B` despite the scope looking something different.
So be aware of the structure of your tests, and write it in a way that is suitable for you.
