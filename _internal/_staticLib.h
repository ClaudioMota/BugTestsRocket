// This content is part of test.h
// Static libraries management

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

    out->globalSymbols = malloc(size);
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

    int c;
    while((c = getc(file)) != EOF)
    {
      ungetc(c, file);
      for(int i = 0; i < out->header.globalSymbolCount; i++)
        if(out->globalSymbols[i].fileOffset == ftell(file))
          out->globalSymbols[i].fileIndex = out->fileCount;
      int i = out->fileCount++;
      out->files = realloc(out->files, sizeof(_StaticLibFile)*out->fileCount);
      _StaticLibFile* libFile = &out->files[i];
      memset(libFile, 0, sizeof(_StaticLibFile));
      fread(libFile->fileInfo, sizeof(libFile->fileInfo), 1, file);
      libFile->contentSize = atoi(&libFile->fileInfo[48]);
      libFile->content = malloc(libFile->contentSize);
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
