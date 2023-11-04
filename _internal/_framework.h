// This content is part of test.h
// Main testing functionalities
typedef struct _TestSelect _TestSelect;
typedef struct _TestContext _TestContext;
typedef struct TestEnvironment TestEnvironment;
typedef struct FunctionMock FunctionMock;
typedef struct FunctionDescriptor FunctionDescriptor;

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

struct _TestContext
{
  bool set;
  FunctionMock* mocksPointer;
  void** mocksSnapshot;
  void (*setupFunction)();
  void (*cleanFunction)();
  void (*onFail)(char* file, int line, char* expr);
  void (*onTestPass)();
  void (*onRaise)(int);
};

struct TestEnvironment
{
  _TestContext globalContext;
  char* _candidateContext;
  char* testContext;
  int testIndex;
  char* testDescription;
  int testLine;

  _TestSelect selection;

  void* _helperBlockIndex;
  void* helperMemoryBlock[_TEST_HELPER_BLOCK_SIZE];
};

struct FunctionMock
{
  bool set;
  int calls;
  void* mockPointer;
  const char* name;
  void* original;
};

struct FunctionDescriptor
{
  char returnType[32];
  char name[_BTR_MAX_NAME_SIZE];
  char args[_BTR_MAX_NAME_SIZE];
  char* implementation;
};

void _ignore();
void _defaultTestPass();
void _defaultFailure(char* file, int line, char* expr);

void (*setupFunction)() = _ignore;
void (*cleanFunction)() = _ignore;
void (*onFail)(char* file, int line, char* expr) = _defaultFailure;
void (*onTestPass)() = _defaultTestPass;
void (*onRaise)(int) = (void(*)(int))_ignore;

int __numArgsCopy;
char** _argsCopy;
char* _sourceFile;
TestEnvironment* testEnv = 0;
extern FunctionMock _mocks[];

void _snapShotGlobalMocks()
{
  if(!testEnv->globalContext.mocksPointer || testEnv->globalContext.mocksSnapshot) return;
  int count = 0;  
  while(testEnv->globalContext.mocksPointer[count].set){count++;}
  count++;
  testEnv->globalContext.mocksSnapshot = (void**)malloc(sizeof(void*)*(count));
  for(int i = 0; i < count-1; i++)
    testEnv->globalContext.mocksSnapshot[i] = *((void**)testEnv->globalContext.mocksPointer[i].mockPointer);
}

void _recoverGlobalMocksSnapShot()
{
  if(!testEnv->globalContext.mocksSnapshot || !testEnv->globalContext.mocksPointer) return;
  int count = 0;  
  while(testEnv->globalContext.mocksPointer[count].set){count++;}
  count++;
  for(int i = 0; i < count-1; i++)
    *((void**)testEnv->globalContext.mocksPointer[i].mockPointer) = testEnv->globalContext.mocksSnapshot[i];
}

void _maybeSetGlobalContext()
{
  if(!testEnv->globalContext.set)
  {
    testEnv->globalContext.set = true;
    testEnv->globalContext.setupFunction = setupFunction;
    testEnv->globalContext.cleanFunction = cleanFunction;
    testEnv->globalContext.onFail = onFail;
    testEnv->globalContext.onTestPass = onTestPass;
    testEnv->globalContext.onRaise = onRaise;

    _snapShotGlobalMocks();
  }
}

void _setContext(char* contextName)
{
  _maybeSetGlobalContext();
  setupFunction = testEnv->globalContext.setupFunction;
  cleanFunction = testEnv->globalContext.cleanFunction;
  onFail = testEnv->globalContext.onFail;
  onTestPass = testEnv->globalContext.onTestPass;
  onRaise = testEnv->globalContext.onRaise;
  testEnv->_candidateContext = contextName;
  _recoverGlobalMocksSnapShot();
}

void _initializeTest(int index, int line, char* description)
{
  _maybeSetGlobalContext();
  testEnv->testIndex = index;
  testEnv->testDescription = description;
  testEnv->testLine = __LINE__;
  testEnv->testContext = testEnv->_candidateContext;
}

char** _copyArgs(int numArgs, char** args)
{
  __numArgsCopy = numArgs;
  char** ret = (char**)malloc(sizeof(char*)*numArgs);

  for(int i = 0; i < numArgs; i++)
  {
    ret[i] = (char*)malloc(sizeof(char)*(strlen(args[i])+1));
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

void _ignore(){}

bool _shouldRunTest(int index, int line, char* context)
{
  int mode = testEnv->selection.mode;
  if(mode == _TEST_SELECT_MODE_NONE) return false;
  if(!((mode & _TEST_SELECT_MODE_INDEX) || (mode & _TEST_SELECT_MODE_LINE)))
    return false;
  if((mode & _TEST_SELECT_MODE_INDEX) && index != testEnv->selection.index)
    return false;
  if((mode & _TEST_SELECT_MODE_MODULE) && !strstr(_sourceFile, testEnv->selection.name) && (!context || strcmp(context, testEnv->selection.name) != 0))
    return false;
  if((mode & _TEST_SELECT_MODE_LINE) && line != testEnv->selection.line)
    return false;

  return true;
}

void _defaultTestPass()
{
  printf(".");
  cleanFunction();
}

void _defaultFailure(char* file, int line, char* expr)
{
  printf("\n[FAIL] on \"%s\" test \"%s\" failed %s:%i (%s)\n", testEnv->testContext, testEnv->testDescription, file, line, expr);
  
  void (*noLoopClean)() = cleanFunction;
  cleanFunction = _ignore;
  noLoopClean();
  _freeArgsCopy();
  exit(0);
}

void _defaultRaiseHandler(int signum)
{
  char* signalStr = _C_STRING_LITERAL("SIGOTHER");
  switch (signum)
  {
    case SIGABRT: signalStr = _C_STRING_LITERAL("SIGABRT"); break;
    case SIGFPE: signalStr = _C_STRING_LITERAL("SIGFPE"); break;
    case SIGILL: signalStr = _C_STRING_LITERAL("SIGILL"); break;
    case SIGINT: signalStr = _C_STRING_LITERAL("SIGINT"); break;
    case SIGSEGV: signalStr = _C_STRING_LITERAL("SIGSEGV"); break;
    case SIGTERM: signalStr = _C_STRING_LITERAL("SIGTERM"); break;
  }
  onRaise(signum);
  onFail(_sourceFile, testEnv->testLine, signalStr);
}

void _assert(char* file, int line, bool assertion, char* expr)
{
  if(!assertion) onFail(file, line, expr);
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

  char* baseDirPath = (char*)malloc(strlen(testRunnerPath) + 1);
  _getDir(testRunnerPath, baseDirPath);
  
  if(!_isDirectory(baseDirPath)) return 0;
  capacity = 1;
  *output = (char**)malloc(sizeof(char*)*capacity);
  (*output)[count++] = baseDirPath;
  while(index < count)
  {
    if(_isDirectory((*output)[index]))
    {
      int dirFileCount = _listFiles((*output)[index], 0);
      while(count + dirFileCount >= capacity)
      {
        capacity *= 2;
        *output = (char**)realloc(*output, sizeof(char*)*capacity);
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

int _testFileMain(int numArgs, char** args, int (*_allTests)())
{
  args = _copyArgs(numArgs, args);
  TestEnvironment _testEnv;
  testEnv = &_testEnv;
  void** snapShot;

  int signals[] = {SIGABRT, SIGFPE, SIGILL, SIGINT, SIGSEGV, SIGTERM};
  for(unsigned int i = 0; i < sizeof(signals)/sizeof(int); i++)
    signal(signals[i], _defaultRaiseHandler);

  _testEnv = (TestEnvironment){0};
  _testEnv._helperBlockIndex = &_testEnv.helperMemoryBlock[0];
  _testEnv.testContext = _C_STRING_LITERAL("global");
  _testEnv.testDescription = _C_STRING_LITERAL("setup");
  int _testCount = _allTests();
  snapShot = _testEnv.globalContext.mocksSnapshot;
  if(snapShot) free(snapShot);
  
  _testEnv = (TestEnvironment){0};
  _testEnv._helperBlockIndex = &_testEnv.helperMemoryBlock[0];
  _testEnv.testContext = _C_STRING_LITERAL("global");
  _testEnv.testDescription = _C_STRING_LITERAL("setup");
  _testEnv.selection = _getArgsSelection(numArgs, args);
  _allTests();
  snapShot = _testEnv.globalContext.mocksSnapshot;
  if(snapShot) free(snapShot);

  _freeArgsCopy();
  
  return _testCount;
}
