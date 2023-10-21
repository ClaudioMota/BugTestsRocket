// This content is part of test.h
// Mock functionalities

typedef struct FunctionMock FunctionMock;
typedef struct FunctionDescriptor FunctionDescriptor;

struct FunctionMock
{
  bool set;
  char name[256];
  void* mockPointer;
};

struct FunctionDescriptor
{
  char returnType[32];
  char name[256];
  char args[256];
};

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

void _getMockedName(char* output, char* functioName)
{
  strcpy(output, "🐛");
  strcat(output, functioName);
  strcat(output, "🚀");
}

bool _createMockFile(char* mockFilePath, int functionCount, FunctionDescriptor* functions)
{
  FILE* file = fopen(mockFilePath, "wb");
  if(!file) return false;

  fprintf(file, "#include <stdbool.h>\n"); 
  fprintf(file, "typedef struct FunctionMock{ bool set; char name[256]; void* mockPointer; } FunctionMock;\n");

  for(int i = 0; i < functionCount; i++)
  {
    char mockedName[256];
    _getMockedName(mockedName, functions[i].name);
    fprintf(file, "%s %s(%s);\n", functions[i].returnType, mockedName, functions[i].args);
    fprintf(file, "%s (*_mocked_%s)(%s) = %s;\n", functions[i].returnType, functions[i].name, functions[i].args, mockedName);
  }
  for(int i = 0; i < functionCount; i++)
  {
    fprintf(file, "%s %s(", functions[i].returnType, functions[i].name);
    int argsCount = _writeArgs(file, functions[i].args);
    fprintf(file, "){ return _mocked_%s(", functions[i].name);
    for(int a = 0; a < argsCount; a++)
      if(a)
        fprintf(file, ", a%i", a);
      else
        fprintf(file, "a%i", a);
    fprintf(file, "); }\n");
  }
  fprintf(file, "FunctionMock _mocks[] = {\n");
  for(int i = 0; i < functionCount; i++)
  {
    fprintf(file, "  {true, \"%s\", &_mocked_%s},\n", functions[i].name, functions[i].name);
  }
  fprintf(file, "  {false, \"\", 0}\n};\n");

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
        _objectFileMockFunction(&lib.files[i], functions[f].name, mockedName);
    }
    
    ret = _staticLibWrite(&lib, mockableLibPath);
    ret &= _createMockFile(mockFilePath, functionCount, functions);
  }
  else
    ret = false;

  _staticLibFree(&lib);

  return ret;
}
