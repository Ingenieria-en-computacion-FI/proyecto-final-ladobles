#ifndef SYMBOLS_H
#define SYMBOLS_H

/* Secciones posibles de un simbolo */
typedef enum {
    SECTION_TEXT,   /* seccion de codigo */
    SECTION_DATA,   /* seccion de datos */
    SECTION_BSS,    /* seccion de datos sin inicializar */
    SECTION_UNKNOWN
} SectionType;

/* Estructura que representa un simbolo */
typedef struct {
    char        name[64];       /* nombre del simbolo */
    int         offset;         /* offset dentro de su seccion */
    SectionType section;        /* seccion donde vive */
    int         is_global;      /* fue declarado con GLOBAL */
    int         is_extern;      /* fue declarado con EXTERN */
    int         defined;        /* ya fue definido o esta pendiente */
} Symbol;

/* Tabla de simbolos */
typedef struct {
    Symbol *symbols;            /* arreglo de simbolos */
    int     count;              /* cuantos simbolos hay */
    int     capacity;           /* capacidad actual del arreglo */
} SymbolTable;

/* Funciones de la tabla de simbolos */
SymbolTable *symbol_table_create();
void         symbol_table_destroy(SymbolTable *table);
int          symbol_add(SymbolTable *table, const char *name,
                        SectionType section, int offset,
                        int is_global, int is_extern);
Symbol      *symbol_find(SymbolTable *table, const char *name);
void         symbol_mark_global(SymbolTable *table, const char *name);
void         symbol_mark_extern(SymbolTable *table, const char *name);
void         symbol_table_print(SymbolTable *table);

#endif /* SYMBOLS_H */
