#include "exampleCpp.h"

MyClass::MyClass(int value)
{
  internalValue = value;
}

void MyClass::internalProcess()
{
  internalValue *= 5;
}

int MyClass::getProcessedValue()
{
  internalProcess();
  return internalValue;
}
