#include "exampleCalc.h"

int sum(int a, int b)
{
  return a + b;
}

int multiply(int a, int b)
{
  return a * b;
}

int subtract(int a, int b)
{
  return a - b;
}

bool divide(int a, int b, int* result)
{
  if(b == 0) return false;
  *result = a/b;
  return true;
}

bool divRemainder(int a, int b, int* result)
{
  *result = a%b; // BUG (raises division by zero)
  return true;
}