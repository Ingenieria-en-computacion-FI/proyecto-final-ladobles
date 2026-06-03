#ifndef LINKER_H
#define LINKER_H

#include "object.h"

/* Simbolo global resuelto por el linker */
typedef struct {
    char         name[64];      /* nombre del simbolo */
    unsigned int final_offset;  /* offset final en el binario */
    unsigned int section;       /* seccion donde vive */
    int          defined;       /* esta definido */
} GlobalSymbol;

/* Tabla global de simbolos del linker */
typedef struct {
    GlobalSymbol *symbols;      /* arreglo de simbolos */
    int           count;        /* cuantos hay */
    int           capacity;     /* capacidad */
} GlobalSymbolTable;

/* Informacion de un objeto cargado */
typedef struct {
    ObjectFile  *obj;           /* archivo objeto */
    unsigned int text_base;     /* donde empieza su .text en el binario */
    unsigned int data_base;     /* donde empieza su .data en el binario */
    unsigned int bss_base;      /* donde empieza su .bss en el binario */
} LoadedObject;

/* Estructura principal del linker */
typedef struct {
    LoadedObject     *objects;      /* objetos cargados */
    int               obj_count;    /* cuantos objetos hay */
    int               obj_capacity; /* capacidad */
    GlobalSymbolTable globals;      /* tabla global de simbolos */
    unsigned char    *text;         /* seccion .text fusionada */
    unsigned int      text_size;    /* tamanio de .text */
    unsigned char    *data;         /* seccion .data fusionada */
    unsigned int      data_size;    /* tamanio de .data */
    unsigned int      bss_size;     /* tamanio de .bss */
    int               error_count;  /* errores encontrados */
} Linker;

/* Funciones del linker */
Linker *linker_create();
void    linker_destroy(Linker *linker);
int     linker_add_object(Linker *linker, const char *filename);
int     linker_link(Linker *linker);
int     linker_write_binary(Linker *linker, const char *filename);
void    linker_print(Linker *linker);

#endif /* LINKER_H */
