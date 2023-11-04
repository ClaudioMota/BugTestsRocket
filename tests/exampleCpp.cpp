#include "test.h"
#include "exampleCpp.h"
#include "mocks.h"

void simulateProcess(MyClass* myClass)
{
  myClass->internalValue *= 10;
}

🐛
context("MyClass::getProcessedValue")
{
  test("processes the value and retrieves it")
  {
    MyClass instance(5);
    assert(instance.getProcessedValue() == 25);
  }

  test("Calls the internal processing function")
  {
    mock(_ZN7MyClass15internalProcessEv, simulateProcess);
    MyClass instance(5);
    assert(instance.getProcessedValue() == 50);
  }
}
🚀