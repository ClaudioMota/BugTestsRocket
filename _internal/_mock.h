// This content is part of test.h
// Mock functionalities

int _writeArgs(FILE* file, char* args)
{
  int argsCount = 0;
  int argsSize = strlen(args);
  char arg[argsSize + 1];
  int a = 0;
  for(int i = 0; i <= argsSize; i++)
  {
    if((!isspace(args[i]) || a) || !args[i])
    {
      if(args[i] == '\0' || args[i] == ',')
      {
        if(a > 0)
        {
          arg[a] = '\0';
          if(argsCount > 0) fprintf(file, ",");
          if(arg[0] != '.') fprintf(file, "%s a%i", arg, argsCount++);
          else fprintf(file, "%s", arg);
        }
        a = 0;
      }
      else
        arg[a++] = args[i];
    }
  }
  return argsCount;
}

void _getMockedName(char* output, char* functioName)
{
  strcpy(output, "🐛");
  strcat(output, functioName);
  strcat(output, "🚀");
}

FunctionMock* _getMock(char* file, int line, char* functionName, FunctionMock* mocks)
{
  if(!testEnv->globalContext.mocksSnapshot)
  {
    testEnv->globalContext.mocksPointer = mocks;
    if(testEnv->globalContext.set)
      _snapShotGlobalMocks();
  }
  
  FunctionMock* mock = 0;
  for(int i = 0; mocks[i].set; i++)
  {
    if(strcmp(mocks[i].name, functionName) == 0)
    {
      mock = &mocks[i];
      break;
    }
  }

  char message[_BTR_MAX_NAME_SIZE];
  strcpy(message, "Could not mock function ");
  strcat(message, functionName);
  if(!mock) onFail(file, line, message);

  return mock;
}

void _mock(char* file, int line, char* functionName, void* function, FunctionMock* mocks)
{
  FunctionMock* mock = _getMock(file, line, functionName, mocks);
  if(mock) *((void**)mock->mockPointer) = function;
}

void _mockReset(char* file, int line, char* functionName, FunctionMock* mocks)
{
  FunctionMock* mock = _getMock(file, line, functionName, mocks);
  if(mock) *((void**)mock->mockPointer) = mock->original;
}

bool _createMockFile(char* mockFilePath, int functionCount, FunctionDescriptor* functions)
{
  FILE* file = fopen(mockFilePath, "wb");
  if(!file) return false;

  fprintf(file, "// This file was generated by BugTestsRocket test framework\n");
  fprintf(file, "// Link it when building your tests for implementing the mocks\n");
  fprintf(file, "#ifdef __cplusplus\nextern \"C\"{\n"
                "#define _BTR_CONVERT(what, to) reinterpret_cast<to>(what)\n"
                "#else\n"
                "#define _BTR_CONVERT(what, to) ((to)(what))\n"
                "#endif\n");
  fprintf(file, "#include <stdbool.h>\n");
  fprintf(file, "typedef struct {bool set; int calls; void* mockPointer; const char* name; void* original; } FunctionMock;\n");
  fprintf(file, "extern FunctionMock _mocks[];\n");

  for(int i = 0; i < functionCount; i++)
  {
    char mockedName[_BTR_MAX_NAME_SIZE];
    _getMockedName(mockedName, functions[i].name);
    char* implementation = _C_STRING_LITERAL(";");
    if(functions[i].implementation) implementation = functions[i].implementation;
    fprintf(file, "%s %s(%s)%s\n", functions[i].returnType, mockedName, functions[i].args, implementation);
    fprintf(file, "void* _mocked_%s = _BTR_CONVERT(%s, void*);\n", functions[i].name, mockedName);
  }
  for(int i = 0; i < functionCount; i++)
  {
    fprintf(file, "%s %s(", functions[i].returnType, functions[i].name);
    int argsCount = _writeArgs(file, functions[i].args);
    fprintf(file, "){ _mocks[%i].calls++; return _BTR_CONVERT(_mocked_%s, %s (*)(%s))(",
      i, functions[i].name, functions[i].returnType, functions[i].args);
    for(int a = 0; a < argsCount; a++)
      if(a)
        fprintf(file, ", a%i", a);
      else
        fprintf(file, "a%i", a);
  }
  fprintf(file, "FunctionMock _mocks[] = {\n");
  for(int i = 0; i < functionCount; i++)
  {
    char mockedName[_BTR_MAX_NAME_SIZE];
    _getMockedName(mockedName, functions[i].name);
    fprintf(file, "  {true, (int)0, (void*)&_mocked_%s, \"%s\", _BTR_CONVERT(%s, void*)},\n", functions[i].name, functions[i].name, mockedName);
  }
  fprintf(file, "  {false, (int)0, (void*)0, \"\", (void*)0}\n};\n");
  fprintf(file, "#ifdef __cplusplus\n}\n#endif\n"); 

  fclose(file);
  return true;
}

bool createMocks(char* libPath, char* mockableLibPath, char* mockFilePath, int functionCount, FunctionDescriptor* functions)
{
  bool ret = true;
 
  _StaticLib lib;

  if(_staticLibRead(&lib, libPath))
  {
    for(int f = 0; f < functionCount; f++)
    {
      char mockedName[strlen(functions[f].name)+64];
      _getMockedName(mockedName, functions[f].name);
    
      for(int i = 0; i < lib.header.globalSymbolCount; i++)
        if(strcmp(lib.globalSymbols[i].name, functions[f].name) == 0)
          strcpy(lib.globalSymbols[i].name, mockedName);
    
      for(int i = 0; i < lib.fileCount; i++)
      {
        if(memcmp(lib.files[i].fileInfo, "//", 2) != 0)
          if(!_objectFileMockFunction(&lib.files[i], functions[f].name, mockedName))
              printf("Could not mock object file %s. Supported formats are ELF64. Symbols must be relocatable. Maybe try adding --fPIC to your compiler flags?\n",
                    lib.files[i].fileInfo);
      }
    }
    
    ret = _staticLibWrite(&lib, mockableLibPath);
    ret &= _createMockFile(mockFilePath, functionCount, functions);
  }
  else
    ret = false;

  _staticLibFree(&lib);

  return ret;
}
