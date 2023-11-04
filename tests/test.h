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

// See https://github.com/ClaudioMota/BugTestsRocket/ for updated versions
// If this file is incomplete try running _internal/mergeHeaders.sh before copying

#ifndef BUG_TESTS_ROCKET_TEST_HEADER
#define BUG_TESTS_ROCKET_TEST_HEADER 1

#ifdef __cplusplus
extern "C"
{
#define _C_STRING_LITERAL(literal) (char*)literal
#define class struct
#define private public
#define protected public
#else
#define _C_STRING_LITERAL(literal) literal
#endif

#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <signal.h>

#define 🐛 beginTests
#define 🚀 endTests

#define _TEST_HELPER_BLOCK_SIZE 10240
#define assert(boolean) _assert(_C_STRING_LITERAL(__FILE__), __LINE__, boolean, _C_STRING_LITERAL(#boolean))
#define assert_called(mockedFunction) assert(mockCalled(mockedFunction) > 0)
#define refute(boolean) _assert(_C_STRING_LITERAL(__FILE__), __LINE__, !(boolean), _C_STRING_LITERAL(#boolean))
#define refute_called(mockedFunction) assert(mockCalled(mockedFunction) == 0)

#define beginTests \
  int _allTests(){ int _testCount = 0; int _testRunning = 0; int _testDefinition = 0; {

#define _finishLastScope() if(_testRunning > 0){ _testRunning--; onTestPass(); } }\
  if(_testDefinition > 0){ _testDefinition--;\
  if(_testDefinition != 0) onFail(_C_STRING_LITERAL(__FILE__), __LINE__, _C_STRING_LITERAL("test scope has been compromised"));}\
  
#define context(name) _finishLastScope() _setContext(_C_STRING_LITERAL(name)); {

#define test(description) \
  _finishLastScope()\
  _testDefinition++;\
  if(_shouldRunTest(_testCount++, __LINE__, testEnv->_candidateContext)){\
    _initializeTest(_testCount-1, __LINE__, _C_STRING_LITERAL(description));\
    _testRunning++;\
    setupFunction();

#define mock(function, newFunction) _mock(_C_STRING_LITERAL(__FILE__), __LINE__, _C_STRING_LITERAL(#function), (void*)newFunction, _mocks)

#define mockReset(function) _mockReset(_C_STRING_LITERAL(__FILE__), __LINE__, _C_STRING_LITERAL(#function), _mocks)

#define mockCalls(function) _getMock(_C_STRING_LITERAL(__FILE__), __LINE__, _C_STRING_LITERAL(#function), _mocks)->calls

#define mockGetOrginal(function) _getMock(_C_STRING_LITERAL(__FILE__), __LINE__, _C_STRING_LITERAL(#function), _mocks)->original

#define testAlloc(type) (type*)(testEnv->_helperBlockIndex += sizeof(type), testEnv->_helperBlockIndex - sizeof(type))

#define endTests _finishLastScope() return _testCount; }\
  int main(int numArgs, char** args){\
    _sourceFile = _C_STRING_LITERAL(__FILE__);\
    return _testFileMain(numArgs, args, _allTests);\
  }

#define _BTR_MAX_NAME_SIZE 512// This content is part of test.h
// Static libraries management

typedef struct _StaticLib _StaticLib;
typedef struct _GlobalSymbol _GlobalSymbol;
typedef struct _StaticLibFile _StaticLibFile;
typedef struct _StaticLibHeader _StaticLibHeader;

struct _GlobalSymbol
{
  char name[_BTR_MAX_NAME_SIZE];
  int fileOffset;
  int fileIndex;
};

struct _StaticLibFile
{
  char fileInfo[60];
  char* content;
  long long contentSize;
};

struct _StaticLibHeader
{
  char sig[8];
  char name[16];
  char date[12];
  char uid[6];
  char gid[6];
  char mode[8];
  char size[10];
  char end[2];

  int globalSymbolCount;
};

struct _StaticLib
{
  _StaticLibHeader header;
  _GlobalSymbol* globalSymbols;
  int fileCount;
  _StaticLibFile* files;
};

bool _staticLibAmILittleEndian()
{
  int a = 1;
  return ((char*)&a)[0] == 1;
}

unsigned int _staticLibSwapBytes(unsigned int number)
{
  return ((number & 0x000000FF) << 24) |
          ((number & 0x0000FF00) << 8) |
          ((number & 0x00FF0000) >> 8) |
          ((number & 0xFF000000) >> 24);
}

void _staticLibSwapIfLittleEndian(int* bigEndian)
{
  if(_staticLibAmILittleEndian())
    *bigEndian = _staticLibSwapBytes(*bigEndian);
}

bool _staticLibRead(_StaticLib* out, char* path)
{
  memset(out, 0, sizeof(_StaticLib));
  FILE *file = fopen(path, "rb");
  if(!file) return false;
  
  bool ok = true;
  fseek(file, 0, SEEK_SET);
  ok = fread(out, sizeof(_StaticLibHeader), 1, file) == 1;
  ok = ok && memcmp(out->header.sig, "!<arch>\n", 6) == 0;

  if(ok)
  {
    _staticLibSwapIfLittleEndian(&out->header.globalSymbolCount);
    int size = sizeof(_GlobalSymbol)*out->header.globalSymbolCount;

    out->globalSymbols = (_GlobalSymbol*)malloc(size);
    memset(out->globalSymbols, 0, size);

    for(int i = 0; i < out->header.globalSymbolCount; i++)
    {
      _GlobalSymbol* symbol = &out->globalSymbols[i];
      symbol->fileIndex = 0;
      fread(&symbol->fileOffset, sizeof(int), 1, file);
      _staticLibSwapIfLittleEndian(&symbol->fileOffset);
    }

    for(int i = 0; i < out->header.globalSymbolCount; i++)
    {
      char* name = out->globalSymbols[i].name;
      int c;
      while((c = fgetc(file))) *(name++) = c;
    }

    fseek(file, sizeof(_StaticLibHeader) -sizeof(int) + atoi(out->header.size), SEEK_SET);
    int c;
    while((c = getc(file)) != EOF)
    {
      ungetc(c, file);
      for(int i = 0; i < out->header.globalSymbolCount; i++)
        if(out->globalSymbols[i].fileOffset == ftell(file))
          out->globalSymbols[i].fileIndex = out->fileCount;
      int i = out->fileCount++;
      out->files = (_StaticLibFile*)realloc(out->files, sizeof(_StaticLibFile)*out->fileCount);
      _StaticLibFile* libFile = &out->files[i];
      memset(libFile, 0, sizeof(_StaticLibFile));
      fread(libFile->fileInfo, sizeof(libFile->fileInfo), 1, file);
      libFile->contentSize = atoi(&libFile->fileInfo[48]);
      libFile->content = (char*)malloc(libFile->contentSize);
      ok = fread(libFile->content, libFile->contentSize, 1, file) == 1;
    }
  }

  fclose(file);

  return ok;
}

bool _staticLibWrite(_StaticLib* lib, char* path)
{
  FILE *file = fopen(path, "wb");
  if(!file) return false;
  
  char auxNum[32];
  int globalSymbolCount = lib->header.globalSymbolCount;
  unsigned int size = sizeof(int)*(globalSymbolCount+1);
  for(int i = 0; i < globalSymbolCount; i++)
    size += strlen(lib->globalSymbols[i].name) + 1;

  _StaticLibHeader header = lib->header;
  memset(header.size, ' ', sizeof(header.size));
  sprintf(auxNum, "%u", size);
  memcpy(header.size, auxNum, strlen(auxNum));
  
  fseek(file, 0, SEEK_SET);
  _staticLibSwapIfLittleEndian(&header.globalSymbolCount);
  fwrite(&header, sizeof(_StaticLibHeader), 1, file);
  
  for(int i = 0; i < globalSymbolCount; i++)
  {
    _GlobalSymbol symbol = lib->globalSymbols[i];
    symbol.fileOffset = sizeof(_StaticLibHeader) - sizeof(int) + size + symbol.fileIndex*60;
    for(int j = 0; j < symbol.fileIndex; j++)
      symbol.fileOffset += lib->files[j].contentSize;
    _staticLibSwapIfLittleEndian(&symbol.fileOffset);
    fwrite(&symbol.fileOffset, sizeof(int), 1, file);
  }

  for(int i = 0; i < globalSymbolCount; i++)
    fwrite(lib->globalSymbols[i].name, strlen(lib->globalSymbols[i].name) + 1, 1, file);

  for(int i = 0; i < lib->fileCount; i++)
  {
    char fileInfo[60];
    _StaticLibFile* libFile = &lib->files[i];
    memcpy(fileInfo, libFile->fileInfo, sizeof(fileInfo));
    memset(fileInfo + 48, ' ', 10);
    sprintf(auxNum, "%lli", libFile->contentSize);
    memcpy(fileInfo + 48, auxNum, strlen(auxNum));
    fwrite(fileInfo, sizeof(libFile->fileInfo), 1, file);
    fwrite(libFile->content, libFile->contentSize, 1, file);
  }

  fclose(file);

  return true;
}

void _staticLibFree(_StaticLib* lib)
{
  for(int i = 0; i < lib->fileCount; i++)
    if(lib->files[i].content) free(lib->files[i].content);
  if(lib->files) free(lib->files);
  if(lib->globalSymbols) free(lib->globalSymbols);
}
// This content is part of test.h
// Object files and symbol management
// Based on https://uclibc.org/docs/elf-64-gen.pdf

typedef struct _ElfRel _ElfRel;
typedef struct _ElfRela _ElfRela;
typedef struct _ElfHeader _ElfHeader;
typedef struct _ElfSectionHeader _ElfSectionHeader;
typedef struct _ElfSymbol _ElfSymbol;

struct _ElfRel
{
  uint64_t r_offset;
  uint64_t r_info;
};

struct _ElfRela
{
  uint64_t r_offset;
  uint64_t r_info;
  int64_t r_addend;
}; 

struct _ElfHeader
{
  unsigned char e_ident[16];
  uint16_t e_type;
  uint16_t e_machine;
  uint32_t e_version;
  uint64_t e_entry;
  uint64_t e_phoff;
  uint64_t e_shoff;
  uint32_t e_flags;
  uint16_t e_ehsize;
  uint16_t e_phentsize;
  uint16_t e_phnum;
  uint16_t e_shentsize;
  uint16_t e_shnum;
  uint16_t e_shstrndx;
};

struct _ElfSectionHeader
{
  uint32_t sh_name;
  uint32_t sh_type;
  uint64_t sh_flags;
  uint64_t sh_addr;
  uint64_t sh_offset;
  uint64_t sh_size;
  uint32_t sh_link;
  uint32_t sh_info;
  uint64_t sh_addralign;
  uint64_t sh_entsize;
};

struct _ElfSymbol
{
  uint32_t st_name;
  uint8_t st_info;
  uint8_t st_other;
  uint16_t st_shndx;
  uint64_t st_value;
  uint64_t st_size;
};

bool _objectFileIsSupportedElf64(_ElfHeader* header)
{
  if(header->e_ident[0] == 0x7f && header->e_ident[1] == 'E' && header->e_ident[2] == 'L' && header->e_ident[3] == 'F' && header->e_ident[4] == 2 &&
     header->e_type == 1 && header->e_entry == 0 && header->e_phoff == 0 && header->e_phentsize == 0 && header->e_phnum == 0 &&
     header->e_shentsize == sizeof(_ElfSectionHeader)
    )
    return true;
  
  // printf("Is this an ELF64 .o??: %x %c%c%c i4: %i t: %i i7: %i e: %li poff: %li psize: %i pnum: %i hsize: %i(%li)\n",
  //  header->e_ident[0], header->e_ident[1], header->e_ident[2],header->e_ident[3],
  //  header->e_ident[4], header->e_type, header->e_ident[7], header->e_entry, header->e_phoff,
  //  header->e_phentsize, header->e_phnum, header->e_shentsize, sizeof(_ElfSectionHeader));
  
  return false;
}

char* _objectFileElfGetString(_StaticLibFile* libFile, _ElfSectionHeader* stringHeader, int nameIndex)
{  
  return &libFile->content[stringHeader->sh_offset + nameIndex];
}

_ElfSymbol* _objectFileElfGetSymbol(_StaticLibFile* libFile, _ElfSectionHeader* symTable, int index)
{
  return (_ElfSymbol*)&libFile->content[symTable->sh_offset + sizeof(_ElfSymbol)*index];
}

bool _objectFileElfIsGlobalFunctionDefinedHere(_ElfSymbol* symbol)
{
  int binding = (symbol->st_info >> 4) & 0xF; // 0 is local
  int type = symbol->st_info & 0xF; // 2 is function
  return binding != 0 && type == 2 && symbol->st_value != 0;
}

void _objectFileMockElfSymbol(_StaticLibFile* libFile, _ElfHeader header, _ElfSectionHeader* sections, _ElfSectionHeader* symbolTable, _ElfSymbol* symbol, char* to)
{
  int offset = sizeof(_ElfHeader), addedBytes = 0;
  char* newContent = (char*)malloc(libFile->contentSize*2);
  memset(newContent, 0, libFile->contentSize*2);
  _ElfSectionHeader* stringTable = &sections[symbolTable->sh_link];
  _ElfSectionHeader* orderedSections[header.e_shnum];
  memset(orderedSections, 0, sizeof(orderedSections));
  int lastMin = -1;
  int lastIndex = -1;
  for(int i = 0; i < header.e_shnum; i++)
  {
    int minIndex;
    for(int j = 0; j < header.e_shnum; j++)
    {
      long long off = sections[j].sh_offset;
    
      if(off < lastMin || (off == lastMin && j <= lastIndex)) continue;
      if(!orderedSections[i] || off < (long long)orderedSections[i]->sh_offset)
      {
        minIndex = j;
        orderedSections[i] = &sections[j];
      }
    }

    lastIndex = minIndex;
    lastMin = orderedSections[i]->sh_offset;
  }

  _ElfSymbol aux = *symbol;
  _ElfSymbol* newSymbol = &aux;
  symbol->st_value = 0;
  symbol->st_size = 0;
  symbol->st_shndx = 0;
  symbol->st_info &= 0xF0;

  for(int i = 0; i < header.e_shnum; i++)
  {
    _ElfSectionHeader* current = orderedSections[i];
    if(current->sh_addralign > 1 && offset % current->sh_addralign)
      offset += current->sh_addralign - (offset % current->sh_addralign);
    memcpy(newContent + offset, libFile->content + current->sh_offset, current->sh_size);
    current->sh_offset = offset;
    offset += current->sh_size;
    
    if(current == stringTable)
    {
      int size = strlen(to) + 1;
      newSymbol->st_name = offset-current->sh_offset;
      memcpy(newContent + offset, to, size);
      offset += size;
      addedBytes += size;
      current->sh_size += size;
    }
    else if(current == symbolTable)
    {
      int size = sizeof(_ElfSymbol);
      memcpy(newContent + offset, newSymbol, size);
      newSymbol = (_ElfSymbol*)(newContent + offset);
      offset += size;
      addedBytes += size;
      current->sh_size += size;
    }
  }

  if(offset % 8) offset += 8 - (offset % 8);

  header.e_shoff = offset;
  for(int i = 0; i < header.e_shnum; i++)
  {
    memcpy(newContent + offset, &sections[i], sizeof(_ElfSectionHeader));
    offset += sizeof(_ElfSectionHeader);
  }

  memcpy(newContent, &header, sizeof(_ElfHeader));
  free(libFile->content);
  libFile->contentSize = offset;
  libFile->content = newContent;
}

bool _objectFileMockElfFunction(_StaticLibFile* libFile, _ElfHeader header, char* from, char* to)
{
  _ElfSectionHeader sections[header.e_shnum];

  for(int i = 0; i < header.e_shnum; i++)
    memcpy(&sections[i], libFile->content + header.e_shoff + sizeof(_ElfSectionHeader)*i, sizeof(_ElfSectionHeader));

  _ElfSectionHeader* symbolTable = 0;
  for(int i = 0; i < header.e_shnum; i++)
    if(sections[i].sh_type == 2) symbolTable = &sections[i];
  
  if(!symbolTable) return true;
  for(unsigned int i = 1; i < symbolTable->sh_size/sizeof(_ElfSymbol); i++)
  {
    _ElfSymbol* symbol = _objectFileElfGetSymbol(libFile, symbolTable, i);
    char* name = _objectFileElfGetString(libFile, &sections[symbolTable->sh_link], symbol->st_name);
    if(name && strcmp(name, from) == 0 && _objectFileElfIsGlobalFunctionDefinedHere(symbol))
    {
      _objectFileMockElfSymbol(libFile, header, sections, symbolTable, symbol, to);
      break;
    }
  }
  
  return true;
}

bool _objectFileMockFunction(_StaticLibFile* libFile, char* from, char* to)
{
  _ElfHeader elfHeader;
  memcpy(&elfHeader, libFile->content, sizeof(_ElfHeader));
  if(_objectFileIsSupportedElf64(&elfHeader))
    return _objectFileMockElfFunction(libFile, elfHeader, from, to);
  return false;
}
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

  fprintf(file, "// This header file was generated by BugTestsRocket test framework\n");
  fprintf(file, "// It must be included after test.h and any eventual required typedefs\n");
  fprintf(file, "#ifdef BUG_TESTS_ROCKET_TEST_HEADER\n");
  fprintf(file, "#ifdef __cplusplus\nextern \"C\"{\n"
                "#define _BTR_CONVERT(what, to) reiterpret_cast<to>(what)\n"
                "#else\n"
                "#define _BTR_CONVERT(what, to) ((to)(what))\n"
                "#endif\n"); 

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
    fprintf(file, "); }\n");
  }
  fprintf(file, "FunctionMock _mocks[] = {\n");
  for(int i = 0; i < functionCount; i++)
  {
    char mockedName[_BTR_MAX_NAME_SIZE];
    _getMockedName(mockedName, functions[i].name);
    fprintf(file, "  {true, (int)0, (void*)&_mocked_%s, \"%s\", _BTR_CONVERT(%s, void*)},\n", functions[i].name, functions[i].name, mockedName);
  }
  fprintf(file, "  {false, (int)0, (void*)0, \"\", (void*)0}\n};\n");
  fprintf(file, "#ifdef __cplusplus\n}\n#endif\n#endif\n"); 

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
        if(memcmp(lib.files[i].fileInfo, "/", 1) != 0)
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
// This content is part of test.h
// Platform specific functions

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
          output[fileCount] = (char*)malloc(pathLength + strlen(ent->d_name) + 2);
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
// Ends test.h
#ifdef __cplusplus
}
#endif
#endif