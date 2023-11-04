# Tutorial zero to hero! (🐛 ->  🚀)

This tutorial was made for enabling someone with low knowledge to get tests running from ground up with the following steps:
- Setup single source file project
- Build a static lib
- Mock the static lib
- Make tests
- Run the tests

## Dependencies

This tutorial assumes you are confortable with terminal commands, and is using a Unix like terminal.
Also you have the following dependencies on your machine:
- `gcc`

## Setup project
Open the terminal in a folder that will contain our project and create a directory for it:
```bash
mkdir project
cd project
```

Now create the source file for the implementation of the lib:
```bash
mkdir source
touch source/example.c
```

Open `source/example.c` and write the following code to be our implementation:
```c
#include "exampleMock.h"

#include <stdlib.h>
#include <time.h>

static int init = 0;

int getRandomInput();

// Retrieves a random number generated with rand
int sumRandomNumber(int a)
{
  switch(getRandomInput())
  {
    case 0: return DECISION_A;
    case 1: return DECISION_B;
  }

  return DECISION_PANIC;
}

// Retrieves a random number generated with rand
void fetch()
{
  if(!init) srand(time(0));
  return rand();
}
```

