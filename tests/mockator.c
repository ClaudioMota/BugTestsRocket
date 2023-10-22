#include "test.h"

int main(int, char**)
{
  FunctionDescriptor functions[] = {
    {"int", "getRandomInput", ""},
    {"void", "_ZN7MyClass15internalProcessEv", "void*"}
  };

  createMocks(
    "build/libExample.a",
    "build/libExampleTest.a",
    "build/mocks.c",
    sizeof(functions)/sizeof(FunctionDescriptor), functions
  );
  
  return 0;
}