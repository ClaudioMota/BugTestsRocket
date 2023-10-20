// MIT License

// Copyright (c) 2023 Claudio Mota Oliveira (cmoliveira1000@gmail.com)

// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:

// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.

// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.

#ifndef TEST_HEADER
#define TEST_HEADER 1

#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <signal.h>
#include <ctype.h>

#define 🐛 beginTests
#define 🚀 endTests

#define _TEST_HELPER_BLOCK_SIZE 1024
#define assert(boolean) _assert(_currentEnv, boolean, __LINE__, #boolean)
#define refute(boolean) _assert(_currentEnv, !(boolean), __LINE__, #boolean)
#define context(name) _context = name;

#define beginTests \
  int _allTests(TestEnvironment* testEnv){ char* _context = ""; if(_context){}; int _testCount = 0; int _testRunning = 0; {

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

#define mock(function, newFunction) _mock(#function, newFunction, _mocks);

#define endTests _finishLastScope() return _testCount; }\
  int main(int numArgs, char** args){\
    _sourceFile = __FILE__;\
    return _testFileMain(numArgs, args, _allTests);\
  }

typedef struct _TestSelect _TestSelect;
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

struct FunctionMock
{
  bool set;
  char name[32];
  void* mockPointer;
};

struct FunctionDescriptor
{
  char returnType[32];
  char name[32];
  char args[256];
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

// Mocks
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

int _readFile(char* path, char** output)
{
  *output = 0;
  FILE* file = fopen(path, "rb");
  if(!file) return 0;

  int capacity = 2000, size = 0;
  *output = malloc(capacity);
  char* buffer = *output;
  int readSize = 0;

  while((readSize = fread(buffer, 1, capacity - size, file)))
  {
    if(readSize + size >= capacity)
    {
      capacity *= 2;
      *output = realloc(*output, capacity);
    }

    size += readSize;
    buffer = *output + size;
  }

  fclose(file);

  return size;
}

bool _writeFile(char* path, char* fileContent, int fileSize)
{
  FILE* file = fopen(path, "wb");
  if(!file) return false;

  bool ret = fwrite(fileContent, 1, fileSize, file) == fileSize;

  fclose(file);

  return ret;
}

char* _findSymbol(char* fileContent, int fileSize, char* symbol)
{
  int symbolLength = strlen(symbol);
  for(int i = 0; i < fileSize; i++)
  {
    bool missed = false;
    for(int j = 0; j <= symbolLength && i + j <= fileSize; j++)
      if(fileContent[i+j] != symbol[j]) missed = true;
    if(!missed) return fileContent + i;
  }
  return 0;
}

int bR000000000000()
{
  return 3423414;
}

int _replaceAllSymbols(char* fileContent, int fileSize, char* symbol, char* replacement)
{
  int symbolLength = strlen(symbol);
  int ocurrences = 0;
  char* auxPointer = fileContent;

  while((auxPointer = _findSymbol(auxPointer, fileSize, symbol)))
  {
    //if(ocurrences == 0)
      //memcpy(auxPointer, replacement, symbolLength);
    auxPointer++;
    ocurrences++;
  }

  return ocurrences;
}

void _makeMockable(char* fileContent, int fileSize, FILE* mocksFile, FunctionDescriptor function, int index)
{
  char* prefix = "bR";
  int minNameLength = strlen(prefix);

  if(strlen(function.name) <= minNameLength)
  {
    printf("Could not make function %s mockable: the function name must be bigger than %i\n", function.name, minNameLength);
    return;
  }

  int maxNameLength = strlen(function.name);
  char indexStr[32];
  char replacement[maxNameLength+1];
  memset(indexStr, 0, sizeof(indexStr));
  memset(replacement, '0', strlen(function.name));
  memcpy(replacement, prefix, minNameLength);

  replacement[strlen(function.name)] = '\0';
  sprintf(indexStr, "%x", index);

  if(strlen(indexStr) + minNameLength > maxNameLength)
  {
    printf("Could not make function %s mockable: mocked name is bigger than function name (%s%x)\n", function.name, prefix, index);
    return;
  }

  memcpy(replacement + minNameLength, indexStr, strlen(indexStr));
  
  int count = _replaceAllSymbols(fileContent, fileSize, function.name, replacement);
  printf("->%i\n", count);

  fprintf(mocksFile, "%s %s(%s);\n", function.returnType, replacement, function.args);
  fprintf(mocksFile, "%s (*_mocked_%s)(%s) = %s;\n", function.returnType, function.name, function.args, replacement);
}

int _writeArgs(FILE* file, char* args)
{
  int argsCount = 0;
  int argsSize = strlen(args);
  char arg[argsSize + 1];
  int a = 0;
  for(int i = 0; i <= argsSize; i++)
  {
    if(!isspace(args[i]) || !args[i])
    {
      if(args[i] == '\0' || args[i] == ',')
      {
        if(a > 0)
        {
          arg[a] = '\0';
          if(argsCount > 0) fprintf(file, ",");
          fprintf(file, "%s a%i", arg, argsCount++);
        }
        a = 0;
      }
      else
        arg[a++] = args[i];
    }
  }
  return argsCount;
}

bool _makeMockables(char* libPath, char* mockFilePath, int functionCount, FunctionDescriptor* functions)
{
  bool ret = true;
  char* fileContent;
  int fileSize = _readFile(libPath, &fileContent);

  if(!fileContent)
  {
    printf("could not open file %s\n", libPath);
    return false;
  }

  FILE* mockFile = fopen(mockFilePath, "wb");
  if(mockFile) 
  {
    fprintf(mockFile, "#include <stdbool.h>\n"); 
    fprintf(mockFile, "typedef struct FunctionMock{ bool set; char name[32]; void* mockPointer; } FunctionMock;\n");
    for(int i = 0; i < functionCount; i++)
      _makeMockable(fileContent, fileSize, mockFile, functions[i], i);
    for(int i = 0; i < functionCount; i++)
    {
      fprintf(mockFile, "%s %s(", functions[i].returnType, functions[i].name);
      int argsCount = _writeArgs(mockFile, functions[i].args);
      fprintf(mockFile, "){ return _mocked_%s(", functions[i].name);
      for(int a = 0; a < argsCount; a++)
        if(a)
          fprintf(mockFile, ", a%i", a);
        else
          fprintf(mockFile, "a%i", a);
      fprintf(mockFile, "); }\n");
    }
    fprintf(mockFile, "FunctionMock _mocks[] = {\n");
    for(int i = 0; i < functionCount; i++)
    {
      fprintf(mockFile, "  {true, \"%s\", &_mocked_%s},\n", functions[i].name, functions[i].name);
    }
    fprintf(mockFile, "  {false, \"\", 0}\n};\n");

    fclose(mockFile);

    if(!_writeFile(libPath, fileContent, fileSize))
    {
      printf("Failed to write mocks to lib %s\n", libPath);
      ret = false;
    }
  }
  else
  {
    printf("Could not write to mocks file %s\n", mockFilePath);
    return false;
  }

  free(fileContent);

  return ret;
}

#ifdef _WIN32
#include <windows.h>

int _listFiles(char* path, char** output)
{
  char basePath[MAX_PATH];
	char wildPath[MAX_PATH];
	int fileCount = 0;
	WIN32_FIND_DATA ffd;
	HANDLE hFind = INVALID_HANDLE_VALUE;

	strcpy(basePath, path);
  if(path[strlen(path-1)] != '/')
    strcat(basePath, "/");
  strcpy(wildPath, basePath);
	strcat(wildPath,"*");

	int basePathLength = strlen(basePath);
	hFind = FindFirstFile(wildPath, &ffd);
	if (INVALID_HANDLE_VALUE == hFind)
	  return 0;

	do
	{
	  if(strcmp(".", ffd.cFileName) != 0 && strcmp("..", ffd.cFileName) != 0)
	  {
      if(output)
      {
        output[fileCount] = malloc(basePathLength + strlen(ffd.cFileName) + 2);
	      strcpy(output[fileCount], basePath);
        strcat(output[fileCount], ffd.cFileName);
      }
      fileCount++;
	  }
	}
	while (FindNextFile(hFind, &ffd) != 0);

	FindClose(hFind);

	return fileCount;
}
#else
#include <dirent.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

bool _isDirectory(char* path)
{
  struct stat stats;
  if(stat(path, &stats) == 0)
  {
    if(S_ISDIR(stats.st_mode)) return true;
  }
  return false;
}

int _listFiles(char* path, char** output)
{
  int fileCount = 0;
  DIR *dir;
	struct dirent *ent;
  int pathLength = strlen(path);

	if ((dir = opendir (path)) != 0) 
  {
	  while ((ent = readdir (dir)) != 0)
    {
      if(strcmp(".", ent->d_name) != 0 && strcmp("..", ent->d_name) != 0)
      {
        if(output)
        {
          output[fileCount] = malloc(pathLength + strlen(ent->d_name) + 2);
          strcpy(output[fileCount], path);
          if(path[pathLength-1] != '\\' && path[pathLength-1] != '/')
            strcat(output[fileCount], "/");
          strcat(output[fileCount], ent->d_name);
        }
        fileCount++; 
      }
	  }
	  closedir (dir);
	}
  
  return fileCount;
}
#endif
#endif
