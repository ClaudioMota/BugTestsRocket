// Copyright 2023 Claudio Mota Oliveira (cmoliveira1000@gmail.com)
// Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the “Software”),
// to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense,
// and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:
// The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
// THE SOFTWARE IS PROVIDED “AS IS”, WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
// WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
// Version alpha. See https://github.com/ClaudioMota/BugTestsRocket for updated versions.

#ifndef TEST_HEADER
#define TEST_HEADER 1

#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <signal.h>

#define 🐛 beginTests
#define 🚀 endTests

#define _TEST_HELPER_BLOCK_SIZE 1024
#define assert(boolean) _assert(testEnv, boolean, __LINE__, #boolean)
#define refute(boolean) _refute(testEnv, boolean, __LINE__, #boolean)
#define context(name) _context = name;

#define beginTests \
  int _allTests(_TestEnvironment* testEnv){ char* _context = ""; if(_context){}; int _testCount = 0; int _testRunning = 0; {

#define _finishLastScope() if(_testRunning > 0){ _testRunning--; onTestPass(testEnv); } }

#define test(description) \
  _finishLastScope()\
  testEnv->testIndex = _testCount++;\
  testEnv->testDescription = description;\
  testEnv->testLine = __LINE__;\
  testEnv->testContext = _context;\
  if(_shouldRunTest(testEnv)){\
    if(_testRunning > 0){ printf("\nError nested tests detected %s:%i\n", _sourceFile, __LINE__); }\
    _testRunning++;\
    setupFunction(testEnv);

#define endTests _finishLastScope() return _testCount; }\
  int main(int numArgs, char** args){\
    _sourceFile = __FILE__;\
    return _testFileMain(numArgs, args, _allTests);\
  }

typedef struct _TestSelect _TestSelect;
typedef struct _TestEnvironment _TestEnvironment;

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

struct _TestEnvironment
{
  char* testContext;
  int testIndex;
  char* testDescription;
  int testLine;

  _TestSelect selection;

  void* helperPointer;
  void* helperBlock[_TEST_HELPER_BLOCK_SIZE];
};

void _ignore(_TestEnvironment* env);
void _defaultTestPass(_TestEnvironment* env);
void _defaultFailure(_TestEnvironment* env, int line, char* expr);

void (*setupFunction)(_TestEnvironment* env) = _ignore;
void (*cleanFunction)(_TestEnvironment* env) = _ignore;
void (*onFail)(_TestEnvironment* env, int line, char* expr) = _defaultFailure;
void (*onTestPass)(_TestEnvironment* env) = _defaultTestPass;
void (*onRaise)(int) = (void*)_ignore;

int __numArgsCopy;
char** _argsCopy;
char* _sourceFile;
_TestEnvironment* _currentEnv = 0;

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

void _ignore(_TestEnvironment* env){}

bool _shouldRunTest(_TestEnvironment* env)
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

void _defaultTestPass(_TestEnvironment* env)
{
  printf(".");
  cleanFunction(env);
}

void _defaultFailure(_TestEnvironment* env, int line, char* expr)
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

void _assert(_TestEnvironment* env, bool assertion, int line, char* expr)
{
  if(!assertion) onFail(env, line, expr);
}

void _refute(_TestEnvironment* env, bool assertion, int line, char* expr)
{
  if(assertion) onFail(env, line, expr);
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

int runTest(char* testProgram)
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
  int count = runTest(program);
  for(int j = 0; j < count; j++)
  {
    char strIndex[64] = {0};
    strcpy(program, file);
    sprintf(strIndex, " --index %i", j);
    strcat(program, fixedParams);
    strcat(program, strIndex);
    if(!runTest(program))
      failures++;
  }

  printf("\n");
  _freeArgsCopy();
  return failures;
}

int runTestsForFiles(int numArgs, char** args, int fileCount, char** files)
{
  int failures = 0;

  for(int i = 0; i < fileCount; i++)
    failures += runTestsForFile(numArgs, args, files[i]);

  _freeArgsCopy();
  return failures;
}

int _testFileMain(int numArgs, char** args, int (*_allTests)(_TestEnvironment*))
{
  args = _copyArgs(numArgs, args);
  _TestEnvironment testEnv = {0};
  _currentEnv = &testEnv;
  int signals[] = {SIGABRT, SIGFPE, SIGILL, SIGINT, SIGSEGV, SIGTERM};
  for(int i = 0; i < sizeof(signals)/sizeof(int); i++)
    signal(signals[i], _defaultRaiseHandler);
  int _testCount = _allTests(&testEnv);
  memset(&testEnv, 0, sizeof(_TestEnvironment));
  testEnv.selection = _getArgsSelection(numArgs, args);
  _allTests(&testEnv);
  _freeArgsCopy();
  return _testCount;
}

#endif
