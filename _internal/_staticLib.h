// This file must be included through test.h only
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

typedef struct _StaticLib _StaticLib;
typedef struct _GlobalSymbol _GlobalSymbol;
typedef struct _StaticLibFile _StaticLibFile;
typedef struct _StaticLibHeader _StaticLibHeader;

struct _GlobalSymbol
{
  char name[256];
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

bool _staticLibRead(_StaticLib* out, char* fileName)
{
  memset(out, 0, sizeof(_StaticLib));
  FILE *file = fopen(fileName, "rb");
  if(!file) return false;
  
  bool ok = true;
  fseek(file, 0, SEEK_SET);
  ok = fread(out, sizeof(_StaticLibHeader), 1, file) == 1;
  ok = ok && memcmp(out->header.sig, "!<arch>\n", 6) == 0;

  if(ok)
  {
    _staticLibSwapIfLittleEndian(&out->header.globalSymbolCount);
    int size = sizeof(_GlobalSymbol)*out->header.globalSymbolCount;

    out->globalSymbols = malloc(size);
    memset(out->globalSymbols, 0, size);

    for(int i = 0; i < out->header.globalSymbolCount; i++)
    {
      _GlobalSymbol* symbol = &out->globalSymbols[i];
      fread(&symbol->fileOffset, sizeof(int), 1, file);
      _staticLibSwapIfLittleEndian(&symbol->fileOffset);
    }

    for(int i = 0; i < out->header.globalSymbolCount; i++)
    {
      char* name = out->globalSymbols[i].name;
      int c;
      while(c = fgetc(file)) *(name++) = c;
    }

    int c;
    while((c = getc(file)) != EOF)
    {
      ungetc(c, file);
      int i = out->fileCount++;
      out->files = realloc(out->files, sizeof(_StaticLibFile)*out->fileCount);
      _StaticLibFile* libFile = &out->files[i];
      memset(libFile, 0, sizeof(_StaticLibFile));
      fread(libFile->fileInfo, sizeof(libFile->fileInfo), 1, file);
      libFile->contentSize = atoi(&libFile->fileInfo[48]);
      libFile->content = malloc(libFile->contentSize);
      ok = fread(libFile->content, libFile->contentSize, 1, file) == 1;
      printf("%48s %lli\n", libFile->fileInfo, libFile->contentSize);
    }
  }

  fclose(file);

  return ok;
}

void _staticLibFree(_StaticLib* lib)
{
  for(int i = 0; i < lib->fileCount; i++)
    if(lib->files[i].content) free(lib->files[i].content);
  if(lib->files) free(lib->files);
  if(lib->globalSymbols) free(lib->globalSymbols);
}
