#ifndef OBJECT_H
#define OBJECT_H

#include "assembler.h"

/* Firma del formato objeto */
#define OBJ_MAGIC      0x4F424A00  /* "OBJ\0" */
#define OBJ_VERSION    1

/* Tipos de seccion */
#define OBJ_SEC_TEXT   0
#define OBJ_SEC_DATA   1
#define OBJ_SEC_BSS    2

/* Tipos de relocacion */
#define OBJ_RELOC_R32    0   /* absoluta 32 bits */
#define OBJ_RELOC_PC32   1   /* relativa al PC 32 bits */

/* ============================================================
   ESTRUCTURAS DEL FORMATO OBJETO
   ============================================================ */

/* Encabezado del archivo objeto */
typedef struct {
    unsigned int magic;             /* firma OBJ_MAGIC */
    unsigned int version;           /* version del formato */
    unsigned int num_sections;      /* numero de secciones */
    unsigned int num_symbols;       /* numero de simbolos */
    unsigned int num_relocations;   /* numero de relocaciones */
    unsigned int offset_sections;   /* offset a tabla de secciones */
    unsigned int offset_symbols;    /* offset a tabla de simbolos */
    unsigned int offset_relocs;     /* offset a tabla de relocaciones */
    unsigned int offset_text;       /* offset a bytes de .text */
    unsigned int size_text;         /* tamanio de .text */
    unsigned int offset_data;       /* offset a bytes de .data */
    unsigned int size_data;         /* tamanio de .data */
    unsigned int size_bss;          /* tamanio de .bss */
} ObjHeader;

/* Entrada en la tabla de secciones */
typedef struct {
    char         name[8];           /* nombre de la seccion */
    unsigned int type;              /* TEXT, DATA o BSS */
    unsigned int offset;            /* offset en el archivo */
    unsigned int size;              /* tamanio en bytes */
} ObjSection;

/* Entrada en la tabla de simbolos */
typedef struct {
    char         name[64];          /* nombre del simbolo */
    unsigned int value;             /* offset dentro de su seccion */
    unsigned int section;           /* indice de seccion */
    unsigned int is_global;         /* visible para el linker */
    unsigned int is_extern;         /* viene de otro modulo */
    unsigned int defined;           /* esta definido */
} ObjSymbol;

/* Entrada en la tabla de relocaciones */
typedef struct {
    unsigned int offset;            /* donde aplicar la relocacion */
    char         symbol_name[64];   /* simbolo al que apunta */
    unsigned int type;              /* R32 o PC32 */
} ObjRelocation;

/* Archivo objeto completo en memoria */
typedef struct {
    ObjHeader     header;
    ObjSection   *sections;
    ObjSymbol    *symbols;
    ObjRelocation *relocations;
    unsigned char *text_data;
    unsigned char *data_data;
} ObjectFile;

/* Funciones del formato objeto */
int          obj_write(const char *filename, Assembler *assembler);
ObjectFile  *obj_read(const char *filename);
void         obj_destroy(ObjectFile *obj);
void         obj_print(ObjectFile *obj);

#endif /* OBJECT_H */
