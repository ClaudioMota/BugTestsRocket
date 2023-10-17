#include "test.h"

int main(int numArgs, char** args)
{
  char* files[] = {"build/tests/exampleCalc"};
  return runTestsForFiles(numArgs, args, sizeof(files)/sizeof(char*), files);
}