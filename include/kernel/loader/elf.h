#ifndef ELF_H
#define ELF_H

#include "fs/vfs.h"
#include "memory/MemoryManager.h"
#include "process/task.h"
#include "types.h"

typedef uint64_t    Elf64_Addr;
typedef uint16_t    Elf64_Half;
typedef uint64_t    Elf64_Off;
typedef int32_t     Elf64_Sword;
typedef int64_t     Elf64_Sxword;
typedef uint32_t    Elf64_Word;
typedef uint64_t    Elf64_Lword;
typedef uint64_t    Elf64_Xword;

#define EI_NIDENT 16

typedef struct {
    unsigned char   e_ident[EI_NIDENT]; /* File identification. */
    Elf64_Half  e_type;     /* File type. */
    Elf64_Half  e_machine;  /* Machine architecture. */
    Elf64_Word  e_version;  /* ELF format version. */
    Elf64_Addr  e_entry;    /* Entry point. */
    Elf64_Off   e_phoff;    /* Program header file offset. */
    Elf64_Off   e_shoff;    /* Section header file offset. */
    Elf64_Word  e_flags;    /* Architecture-specific flags. */
    Elf64_Half  e_ehsize;   /* Size of ELF header in bytes. */
    Elf64_Half  e_phentsize;    /* Size of program header entry. */
    Elf64_Half  e_phnum;    /* Number of program header entries. */
    Elf64_Half  e_shentsize;    /* Size of section header entry. */
    Elf64_Half  e_shnum;    /* Number of section header entries. */
    Elf64_Half  e_shstrndx; /* Section name strings section. */
} Elf64_Ehdr;

#define ELFMAG0 0x7F 
#define ELFMAG1 'E'  
#define ELFMAG2 'L'  
#define ELFMAG3 'F'

#define ELFCLASSNONE 0
#define ELFCLASS32 1
#define ELFCLASS64 2

#define ELFDATANONE 0
#define ELFDATA2LSB 1
#define ELFDATA2MSB

enum Elf_Ident {
    EI_MAG0     = 0,
    EI_MAG1     = 1,
    EI_MAG2     = 2,
    EI_MAG3     = 3,
    EI_CLASS    = 4,
    EI_DATA     = 5,
    EI_VERSION  = 6,
    EI_OSABI    = 7,
    EI_ABIVER   = 8, 
    EI_PAD      = 9  
};
} 

#endif
