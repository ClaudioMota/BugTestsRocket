#include "exampleMock.h"

#include <stdlib.h>
#include <time.h>

static int init = 0;

int getRandomInput();

int takeDecision()
{
  switch(getRandomInput())
  {
    case 0: return DECISION_A;
    case 1: return DECISION_B;
  }

  return DECISION_PANIC;
}

int getRandomInput()
{
  if(!init) srand(time(0));
  return rand() % 11;
}