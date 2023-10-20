#include "test.h"

int main(int, char**)
{
  FunctionDescriptor functions[] = {
    {"int", "getRandomInput", ""}
  };

  _makeMockables("build/libExample.a", "build/mocks.c", sizeof(functions)/sizeof(FunctionDescriptor), functions);
  return 0;
}