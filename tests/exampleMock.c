#include "test.h"
#include "exampleMock.h"

static int expectedInput = 0;
int getExpectedInput()
{
  return expectedInput;
}

🐛
context("takeDecision")
{
  mock(getRandomInput, getExpectedInput);

  test("makes decision A when the random input is 0")
  { 
    expectedInput = 0;
    assert(takeDecision() == DECISION_A);
  }

  test("makes decision B when the random input is 1")
  {
    expectedInput = 1;
    assert(takeDecision() == DECISION_B);
  }

  test("panics when the random input is other value")
  {
    expectedInput = 5;
    assert(takeDecision() == DECISION_PANIC);
  }
}
🚀