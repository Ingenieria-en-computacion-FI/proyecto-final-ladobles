#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../include/symbols.h"

/* Crea una nueva tabla de simbolos */
SymbolTable *symbol_table_create() {
    SymbolTable *table = (SymbolTable *)malloc(sizeof(SymbolTable));
    if (!table) return NULL;

    table->capacity = 64;
    table->count    = 0;
    table->symbols  = (Symbol *)malloc(table->capacity * sizeof(Symbol));
    if (!table->symbols) {
        free(table);
        return NULL;
    }
    return table;
}

/* Libera la memoria de la tabla */
void symbol_table_destroy(SymbolTable *table) {
    if (table) {
        free(table->symbols);
        free(table);
    }
}

/* Busca un simbolo por nombre */
Symbol *symbol_find(SymbolTable *table, const char *name) {
    for (int i = 0; i < table->count; i++) {
        if (strcmp(table->symbols[i].name, name) == 0)
            return &table->symbols[i];
    }
    return NULL;
}

/* Agrega un simbolo a la tabla */
int symbol_add(SymbolTable *table, const char *name,
               SectionType section, int offset,
               int is_global, int is_extern) {

    /* Verificar si ya existe */
    Symbol *existing = symbol_find(table, name);
    if (existing) {
        if (existing->defined && !is_extern) {
            fprintf(stderr, "Error: simbolo redefinido '%s'\n", name);
            return -1;
        }
        /* Actualizar simbolo existente */
        existing->offset    = offset;
        existing->section   = section;
        existing->is_global = is_global;
        existing->defined   = 1;
        return 0;
    }

    /* Expandir capacidad si es necesario */
    if (table->count >= table->capacity) {
        table->capacity *= 2;
        table->symbols   = (Symbol *)realloc(table->symbols,
                           table->capacity * sizeof(Symbol));
        if (!table->symbols) return -1;
    }

    /* Agregar nuevo simbolo */
    Symbol *sym = &table->symbols[table->count++];
    strncpy(sym->name, name, 63);
    sym->name[63]   = '\0';
    sym->offset     = offset;
    sym->section    = section;
    sym->is_global  = is_global;
    sym->is_extern  = is_extern;
    sym->defined    = !is_extern;
    return 0;
}

/* Marca un simbolo como global */
void symbol_mark_global(SymbolTable *table, const char *name) {
    Symbol *sym = symbol_find(table, name);
    if (sym) {
        sym->is_global = 1;
    } else {
        /* Crear simbolo global pendiente de definicion */
        symbol_add(table, name, SECTION_UNKNOWN, 0, 1, 0);
        Symbol *new_sym = symbol_find(table, name);
        if (new_sym) new_sym->defined = 0;
    }
}

/* Marca un simbolo como externo */
void symbol_mark_extern(SymbolTable *table, const char *name) {
    Symbol *sym = symbol_find(table, name);
    if (sym) {
        sym->is_extern = 1;
        sym->defined   = 0;
    } else {
        symbol_add(table, name, SECTION_UNKNOWN, 0, 0, 1);
    }
}

/* Imprime la tabla de simbolos para depuracion */
void symbol_table_print(SymbolTable *table) {
    const char *section_names[] = { "TEXT", "DATA", "BSS", "UNKNOWN" };
    printf("\n=== TABLA DE SIMBOLOS ===\n");
    printf("%-20s %-8s %-10s %-8s %-8s %-8s\n",
           "NOMBRE", "OFFSET", "SECCION", "GLOBAL", "EXTERN", "DEFINIDO");
    printf("-----------------------------------------------------------\n");
    for (int i = 0; i < table->count; i++) {
        Symbol *sym = &table->symbols[i];
        printf("%-20s %-8d %-10s %-8s %-8s %-8s\n",
               sym->name,
               sym->offset,
               section_names[sym->section],
               sym->is_global ? "si" : "no",
               sym->is_extern ? "si" : "no",
               sym->defined   ? "si" : "no");
    }
    printf("=========================\n\n");
}
