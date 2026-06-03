#ifndef ASSEMBLER_H
#define ASSEMBLER_H

#include "symbols.h"
#include "parser.h"

/* Tipos de fixup para referencias adelantadas */
typedef enum {
    FIXUP_RELATIVE,  /* direccion relativa: saltos y calls */
    FIXUP_ABSOLUTE   /* direccion absoluta: datos */
} FixupType;

/* Estructura que representa un fixup */
typedef struct {
    int       offset_in_code;   /* donde parchear en el buffer de codigo */
    char      symbol_name[64];  /* simbolo pendiente de resolver */
    FixupType type;             /* tipo de fixup */
    int       section;          /* seccion donde esta el fixup */
} Fixup;

/* Tipos de relocacion para el archivo objeto */
typedef enum {
    RELOC_R32,    /* direccion absoluta de 32 bits */
    RELOC_PC32    /* direccion relativa al PC de 32 bits */
} RelocType;

/* Estructura que representa una relocacion */
typedef struct {
    int       offset;           /* offset en el codigo donde aplicar */
    char      symbol_name[64];  /* simbolo al que apunta */
    RelocType type;             /* tipo de relocacion */
} Relocation;

/* Buffer de codigo generado */
typedef struct {
    unsigned char *data;        /* bytes generados */
    int            size;        /* cuantos bytes hay */
    int            capacity;    /* capacidad del buffer */
} CodeBuffer;

/* Estructura principal del ensamblador */
typedef struct {
    SymbolTable  *symbols;          /* tabla de simbolos */
    CodeBuffer    text;             /* seccion .text */
    CodeBuffer    data;             /* seccion .data */
    int           bss_size;         /* tamanio de .bss */
    Fixup        *fixups;           /* lista de fixups */
    int           fixup_count;      /* cuantos fixups hay */
    int           fixup_capacity;   /* capacidad de fixups */
    Relocation   *relocations;      /* lista de relocaciones */
    int           reloc_count;      /* cuantas relocaciones hay */
    int           reloc_capacity;   /* capacidad de relocaciones */
    int           current_section;  /* seccion actual */
    int           error_count;      /* errores encontrados */
} Assembler;

/* Funciones del ensamblador */
Assembler *assembler_create();
void       assembler_destroy(Assembler *assembler);
int        assembler_one_pass(Assembler *assembler,
                              ASTNode *nodes, int count);
int        assembler_two_pass(Assembler *assembler,
                              ASTNode *nodes, int count);
void       assembler_print_code(Assembler *assembler);

#endif /* ASSEMBLER_H */
