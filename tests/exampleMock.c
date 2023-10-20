#include "test.h"
#include "exampleMock.h"

static int expectedInput = 0;
int getExpectedInput()
{
  printf("Is calling mock\n");
  return expectedInput;
}

🐛
context("takeDecision")
{
  test("makes decision A when the random input is 0")
  { 
    expectedInput = DECISION_B;
    mock(getRandomInput, getExpectedInput);
    printf("Is calling mock???\n");
    assert(takeDecision() == DECISION_A);
  }

  test("makes decision B when the random input is 1")
  {
    assert(takeDecision() == DECISION_B);
  }

  test("panics when the random input is other value")
  {
    assert(takeDecision() == DECISION_PANIC);
  }
}
🚀