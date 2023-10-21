// This content is part of test.h
// Main testing functionalities
typedef struct _TestSelect _TestSelect;
typedef struct TestEnvironment TestEnvironment;

enum _TestSelectMode
{
  _TEST_SELECT_MODE_NONE = 0,
  _TEST_SELECT_MODE_INDEX = 0b001,
  _TEST_SELECT_MODE_MODULE = 0b010,
  _TEST_SELECT_MODE_LINE = 0b100
};

struct _TestSelect
{
  int mode;
  int index, line;
  char* name;
};

struct TestEnvironment
{
  char* testContext;
  int testIndex;
  char* testDescription;
  int testLine;

  _TestSelect selection;

  void* helperPointer;
  void* helperBlock[_TEST_HELPER_BLOCK_SIZE];
};

void _ignore(TestEnvironment* env);
void _defaultTestPass(TestEnvironment* env);
void _defaultFailure(TestEnvironment* env, int line, char* expr);

void (*setupFunction)(TestEnvironment* env) = _ignore;
void (*cleanFunction)(TestEnvironment* env) = _ignore;
void (*onFail)(TestEnvironment* env, int line, char* expr) = _defaultFailure;
void (*onTestPass)(TestEnvironment* env) = _defaultTestPass;
void (*onRaise)(int) = (void*)_ignore;

int __numArgsCopy;
char** _argsCopy;
char* _sourceFile;
TestEnvironment* _currentEnv = 0;
extern FunctionMock _mocks[];

char** _copyArgs(int numArgs, char** args)
{
  __numArgsCopy = numArgs;
  char** ret = malloc(sizeof(char*)*numArgs);

  for(int i = 0; i < numArgs; i++)
  {
    ret[i] = malloc(sizeof(char)*(strlen(args[i])+1));
    strcpy(ret[i], args[i]);
  }

  _argsCopy = ret;
  return ret;
}

void _freeArgsCopy()
{
  if(!_argsCopy) return;
  for(int i = 0; i < __numArgsCopy; i++)
  {
    free(_argsCopy[i]);
  }
  free(_argsCopy);
  _argsCopy = 0;
}

void _ignore(TestEnvironment* env){}

bool _shouldRunTest(TestEnvironment* env)
{
  int mode = env->selection.mode;
  if(mode == _TEST_SELECT_MODE_NONE) return false;
  if((mode & _TEST_SELECT_MODE_INDEX) && env->testIndex != env->selection.index)
    return false;
  if((mode & _TEST_SELECT_MODE_MODULE) && !strstr(_sourceFile, env->selection.name) && (!env->testContext || strcmp(env->testContext, env->selection.name) != 0))
    return false;
  if((mode & _TEST_SELECT_MODE_LINE) && env->testLine != env->selection.line)
    return false;

  return true;
}

void _defaultTestPass(TestEnvironment* env)
{
  printf(".");
  cleanFunction(env);
}

void _defaultFailure(TestEnvironment* env, int line, char* expr)
{
  if(env->testContext)
    printf("\n[FAIL] on \"%s\" test \"%s\" failed %s:%i (%s)\n", env->testContext, env->testDescription, _sourceFile, line, expr);
  else
    printf("\n[FAIL] test \"%s\" failed %s:%i (%s)\n", env->testDescription, _sourceFile, line, expr);
  
  cleanFunction(env);
  _freeArgsCopy();
  exit(0);
}

void _defaultRaiseHandler(int signum)
{
  char* signalStr = "SIGOTHER";
  switch (signum)
  {
    case SIGABRT: signalStr = "SIGABRT"; break;
    case SIGFPE: signalStr = "SIGFPE"; break;
    case SIGILL: signalStr = "SIGILL"; break;
    case SIGINT: signalStr = "SIGINT"; break;
    case SIGSEGV: signalStr = "SIGSEGV"; break;
    case SIGTERM: signalStr = "SIGTERM"; break;
  }
  onRaise(signum);
  onFail(_currentEnv, _currentEnv->testLine, signalStr);
}

void _assert(TestEnvironment* env, bool assertion, int line, char* expr)
{
  if(!assertion) onFail(env, line, expr);
}

_TestSelect _getArgsSelection(int numArgs, char** args)
{
  _TestSelect ret = {0};
  for(int i = 1; i < numArgs; i++)
  {
    if(strcmp(args[i], "--module") == 0)
    {
      if(i+1 < numArgs)
      {
        ret.mode |= _TEST_SELECT_MODE_MODULE;
        ret.name = args[i+1];
      }
      i++;
    }
    else if(strcmp(args[i], "--index") == 0)
    {
      if(i+1 < numArgs)
      {
        ret.mode |= _TEST_SELECT_MODE_INDEX;
        ret.index = atoi(args[i+1]);
      }
      i++;
    }
    else if(strcmp(args[i], "--line") == 0)
    {
      if(i+1 < numArgs)
      {
        ret.mode |= _TEST_SELECT_MODE_LINE;
        ret.line = atoi(args[i+1]);
      }
      i++;
    }
    else
    {
      char* division = strstr(args[i], ":");
      if(division)
      {
        *division = 0;
        ret.line = atoi(division+1);
        ret.mode |= _TEST_SELECT_MODE_LINE;
      }
      ret.mode |= _TEST_SELECT_MODE_MODULE;
      ret.name = args[i];
    }
  }
  return ret;
}

int _doRunTest(char* testProgram)
{
  fflush(NULL);
  FILE* output = popen(testProgram, "r");
  if(!output) return 0;
  int value;
  while((value = fgetc(output)))
  {
    if(value == EOF) break;
    printf("%c", (char)value);
  }

  int status = pclose(output);
  if(status == -1) return -1;

  return WEXITSTATUS(status);
}

void _mock(char* functionName, void* function, FunctionMock* mocks)
{
  void** mockPointer = 0;
  for(int i = 0; i < mocks[i].set; i++)
  {
    if(strcmp(mocks[i].name, functionName) == 0)
    {
      mockPointer = mocks[i].mockPointer;
      break;
    }
  }

  char message[256];
  strcpy(message, "Could not mock function ");
  strcat(message, functionName);
  if(!mockPointer) onFail(_currentEnv, _currentEnv->testLine, message);

  *mockPointer = function;
}

// Allocates and sets to output a list with all test files in the directory and subdirectories of the test runner
// Returns the count of files
bool _isDirectory(char* file);
int _listFiles(char* dir, char** output);
void _getDir(char* file, char* output)
{
  char* aux, *last = file + strlen(file);
  aux = file;
  while(aux)
  {
    last = aux;
    aux = strstr(aux, "/");
    if(aux) aux++;
  }
  memcpy(output, file, last - file);
  output[last - file] = '\0';
}

int findAllTestFiles(char* testRunnerPath, char*** output)
{
  int index = 0, count = 0, capacity = 0, filteredCount = 0;
  *output = 0;

  char* baseDirPath = malloc(strlen(testRunnerPath) + 1);
  _getDir(testRunnerPath, baseDirPath);
  
  if(!_isDirectory(baseDirPath)) return 0;
  capacity = 1;
  *output = malloc(sizeof(char*)*capacity);
  (*output)[count++] = baseDirPath;
  while(index < count)
  {
    if(_isDirectory((*output)[index]))
    {
      int dirFileCount = _listFiles((*output)[index], 0);
      while(count + dirFileCount >= capacity)
      {
        capacity *= 2;
        *output = realloc(*output, sizeof(char*)*capacity);
      }
      _listFiles((*output)[index], *output + count);
      count += dirFileCount;
    }

    index++;
  }
  
  for(int i = 0; i < count; i++)
  {
    if(_isDirectory((*output)[i]) || strcmp((*output)[i], testRunnerPath) == 0)
    {
      free((*output)[i]);
      filteredCount++;
    }
    else
    {
      (*output)[i-filteredCount] = (*output)[i];
    }
  }

  return count - filteredCount;
}

int runTestsForFile(int numArgs, char** args, char* file)
{
  int failures = 0;
  args = _copyArgs(numArgs, args);
  _TestSelect selection = _getArgsSelection(numArgs, args);

  char fixedParams[1024] = {0};
  if((selection.mode & _TEST_SELECT_MODE_LINE))
    sprintf(fixedParams, " --line %i", selection.line);
  if((selection.mode & _TEST_SELECT_MODE_MODULE))
    sprintf(fixedParams + strlen(fixedParams), " --module \"%s\"", selection.name);

  char program[(strlen(file)+64)*4];
  strcpy(program, file);
  int count = _doRunTest(program);
  for(int j = 0; j < count; j++)
  {
    char strIndex[64] = {0};
    strcpy(program, file);
    sprintf(strIndex, " --index %i", j);
    strcat(program, fixedParams);
    strcat(program, strIndex);
    if(!_doRunTest(program))
      failures++;
  }

  printf("\n");
  _freeArgsCopy();
  return failures;
}

int runAllTests(int numArgs, char** args)
{
  char** files = 0;
  int fileCount = findAllTestFiles(args[0], &files);
  
  int failures = 0;

  for(int i = 0; i < fileCount; i++)
    failures += runTestsForFile(numArgs, args, files[i]);

  if(files)
  {
    for(int i = 0; i < fileCount; i++)
      free(files[i]);
    free(files);
  }
  
  return failures;
}

int _testFileMain(int numArgs, char** args, int (*_allTests)(TestEnvironment*))
{
  args = _copyArgs(numArgs, args);
  TestEnvironment testEnv = {0};
  _currentEnv = &testEnv;
  int signals[] = {SIGABRT, SIGFPE, SIGILL, SIGINT, SIGSEGV, SIGTERM};
  for(int i = 0; i < sizeof(signals)/sizeof(int); i++)
    signal(signals[i], _defaultRaiseHandler);
  int _testCount = _allTests(&testEnv);
  memset(&testEnv, 0, sizeof(TestEnvironment));
  testEnv.selection = _getArgsSelection(numArgs, args);
  _allTests(&testEnv);
  _freeArgsCopy();
  return _testCount;
}
