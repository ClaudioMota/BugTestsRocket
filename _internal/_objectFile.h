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
