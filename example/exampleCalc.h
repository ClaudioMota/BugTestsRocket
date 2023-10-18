#ifndef EXAMPLE_CALC
#define EXAMPLE_CALC

#include <stdbool.h>

int sum(int a, int b);
int multiply(int a, int b);
int subtract(int a, int b);
bool divide(int a, int b, int* result);
bool divRemainder(int a, int b, int* result);

#endif