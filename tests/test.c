#include "test.h"

int doCreateMocks()
{
  FunctionDescriptor functions[] = {
      {"int", "getRandomInput", "", 0},
      {"void", "_ZN7MyClass15internalProcessEv", "void*", 0}
  };

  return !createMocks(
    "build/libExample.a",
    "build/libExampleTest.a",
    "tests/mocks.h",
    sizeof(functions)/sizeof(FunctionDescriptor), functions
  );
}

int main(int numArgs, char** args)
{
  if(numArgs > 1 && strcmp(args[1], "--generate-mocks") == 0)
    return doCreateMocks();
  else
    return runAllTests(numArgs, args);
}