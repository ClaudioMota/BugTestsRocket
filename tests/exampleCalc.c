#include "test.h"
#include "exampleCalc.h"

🐛
context("sum")
{
  test("sums the first with the second")
  {
    assert(sum(1, 2) == 3);
    assert(sum(1, -2) == -1);
  }
}

context("subtract")
{
  test("subtracts the first by the second")
  {
    // As the implementation is wrong this test will fail
    assert(subtract(1, 2) == -1);
    assert(subtract(1, -1) == 2);
  }
}

context("multiply")
{
  test("multiplies the first by the second")
  {
    assert(multiply(5, 3) == 15);
    assert(multiply(3, 0) == 0);
  }
}

context("divide")
{
  test("divides the first by the second")
  {
    int result;
    assert(divide(5, 2, &result));
    assert(result == 2);
    assert(divide(150, 10, &result));
    assert(result == 15);
  }

  test("retrieves false on division by zero")
  {
    int result;
    refute(divide(5, 0, &result));
    refute(divide(0, 0, &result));
  }
}

context("divRemainder")
{
  test("divides the first by the second")
  {
    int result;
    assert(divRemainder(5, 2, &result));
    assert(result == 1);
    assert(divRemainder(150, 10, &result));
    assert(result == 0);
  }

  test("retrieves false on division by zero")
  {
    // As the implementation raises this test will fail
    int result;
    refute(divRemainder(5, 0, &result));
    refute(divRemainder(5, 0, &result));
  }
}
🚀