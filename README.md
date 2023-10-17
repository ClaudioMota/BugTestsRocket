# 🐛 Tests 🚀

Welcome to your C testing framework. It was build with focus on simplicity and readability.

## Main features
- Readable: The syntax was designed to be readable and clean.
- Plug and play: Just copy the test.h to your project and be happy (you might also find useful to adapt the Makefile to yours).
- Automatic: Tests are written and executed and that's it. No need to be configuring or tracking test files/functions.

## Example
Here is an example of a test file written using the framework:

`
#include "test.h"

🐛
context("sum")
{
  test("should add the first to the second")
  {
    assert(sum(2, 1) == 3);
  }
}

context("divide")
{
  test("should divide the first by the second")
  {
    assert(divide(6,2) == 3);
  }

  test("should perform integer division")
  {
    refute(divide(3,2) != 1);
  }
}
🚀
`

This repo itself is a working example so feel free to clone it and play around!


## Framework structure
For the framework to work, the project should follow some basic guidelines as standards.

- Tests macros are state machines: Once you call a test macro the state is set, so the macros don't follow your scope structure. See pitfalls section.
- Test code != source code: tests may refer to source hearders, but their must binaries not be the same.
- Each test file is a final executable: That means that test binaries should not be linked to other binaries that implement a main function.
- Test output directory: All tests must be output to a test binary directory, so that the test runner can detect and run them automatically.
- `tests/test.c` is the runner: This is just a convention, you should write a main function for running the all tests on this file.

## Tests runner
The test runner is a bootstrap executable that is responsible for running all other tests and making sense of their results.
For standards it is adviced to be the `tests/test.c` and it should look like that:

`
#include "test.h"

int main(int numArgs, char** args)
{
  char** files;
  int fileCount = findTestFilesRecursively(&files);
  int failureCount = runTestsForFiles(numArgs, args, 2, files);
  if(files)
  {
    for(int i = 0; i < fileCount; i++)
      free(files[i]);
    free(files);
  }
  return failureCount;
}
`

You can always change it for adding the functionalities your project require.

## Pitfalls
As the test macros are state machines and will be set in any point you call them, any aparent scope structure is just syntax suggar. For example:
`
#include "test.h"

🐛
context("A")
{
  test("test 1")
  {
    assert(true);
  }

  test("test 3")
  {
    refute(false);
  }
}

context("B")
{
  test("test 2")
  {
    assert(true);
  }
}
🚀
`
Has the same test semmantic of:
`
#include "test.h"

🐛
context("A")
{
  test("test 1")
    assert(true);

  test("test 3")
    refute(false);

  context("B")
    test("test 2")
      assert(true);
}
🚀
`
Or even:
`
#include "test.h"

🐛
context("A")
test("test 1")
assert(true);
test("test 3")
refute(false);
context("B")
test("test 2")
assert(true);
🚀
`