#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../include/linker.h"

/* ============================================================
   CREACION Y DESTRUCCION
   ============================================================ */

Linker *linker_create() {
    Linker *linker = (Linker *)malloc(sizeof(Linker));
    if (!linker) return NULL;

    linker->objects              = (LoadedObject *)malloc(16 * sizeof(LoadedObject));
    linker->obj_count            = 0;
    linker->obj_capacity         = 16;
    linker->globals.symbols      = (GlobalSymbol *)malloc(256 * sizeof(GlobalSymbol));
    linker->globals.count        = 0;
    linker->globals.capacity     = 256;
    linker->text                 = NULL;
    linker->text_size            = 0;
    linker->data                 = NULL;
    linker->data_size            = 0;
    linker->bss_size             = 0;
    linker->error_count          = 0;
    return linker;
}

void linker_destroy(Linker *linker) {
    if (linker) {
        for (int i = 0; i < linker->obj_count; i++)
            obj_destroy(linker->objects[i].obj);
        free(linker->objects);
        free(linker->globals.symbols);
        free(linker->text);
        free(linker->data);
        free(linker);
    }
}

/* ============================================================
   CARGA DE ARCHIVOS OBJETO
   ============================================================ */

int linker_add_object(Linker *linker, const char *filename) {
    ObjectFile *obj = obj_read(filename);
    if (!obj) {
        fprintf(stderr, "Error: no se pudo leer '%s'\n", filename);
        return -1;
    }

    if (linker->obj_count >= linker->obj_capacity) {
        linker->obj_capacity *= 2;
        linker->objects = (LoadedObject *)realloc(linker->objects,
                          linker->obj_capacity * sizeof(LoadedObject));
    }

    LoadedObject *lo = &linker->objects[linker->obj_count++];
    lo->obj       = obj;
    lo->text_base = 0;
    lo->data_base = 0;
    lo->bss_base  = 0;
    printf("Objeto cargado: %s\n", filename);
    return 0;
}

/* ============================================================
   BUSQUEDA EN TABLA GLOBAL
   ============================================================ */

static GlobalSymbol *global_find(GlobalSymbolTable *table,
                                  const char *name) {
    for (int i = 0; i < table->count; i++) {
        if (strcmp(table->symbols[i].name, name) == 0)
            return &table->symbols[i];
    }
    return NULL;
}

static int global_add(GlobalSymbolTable *table, const char *name,
                       unsigned int offset, unsigned int section) {
    GlobalSymbol *existing = global_find(table, name);
    if (existing && existing->defined) {
        fprintf(stderr, "Error: simbolo global duplicado '%s'\n", name);
        return -1;
    }

    if (table->count >= table->capacity) {
        table->capacity *= 2;
        table->symbols = (GlobalSymbol *)realloc(table->symbols,
                         table->capacity * sizeof(GlobalSymbol));
    }

    GlobalSymbol *sym = &table->symbols[table->count++];
    strncpy(sym->name, name, 63);
    sym->name[63]      = '\0';
    sym->final_offset  = offset;
    sym->section       = section;
    sym->defined       = 1;
    return 0;
}

/* ============================================================
   PROCESO DE ENLAZADO
   ============================================================ */

int linker_link(Linker *linker) {

    /* --- Paso 1: calcular tamanios totales --- */
    unsigned int total_text = 0;
    unsigned int total_data = 0;
    unsigned int total_bss  = 0;

    for (int i = 0; i < linker->obj_count; i++) {
        total_text += linker->objects[i].obj->header.size_text;
        total_data += linker->objects[i].obj->header.size_data;
        total_bss  += linker->objects[i].obj->header.size_bss;
    }

    /* Reservar buffers fusionados */
    linker->text      = (unsigned char *)calloc(total_text, 1);
    linker->data      = (unsigned char *)calloc(total_data, 1);
    linker->text_size = total_text;
    linker->data_size = total_data;
    linker->bss_size  = total_bss;

    /* --- Paso 2: fusionar secciones y calcular bases --- */
    unsigned int text_offset = 0;
    unsigned int data_offset = 0;
    unsigned int bss_offset  = 0;

    for (int i = 0; i < linker->obj_count; i++) {
        LoadedObject *lo  = &linker->objects[i];
        ObjectFile   *obj = lo->obj;

        lo->text_base = text_offset;
        lo->data_base = data_offset;
        lo->bss_base  = bss_offset;

        if (obj->header.size_text > 0)
            memcpy(linker->text + text_offset,
                   obj->text_data,
                   obj->header.size_text);

        if (obj->header.size_data > 0)
            memcpy(linker->data + data_offset,
                   obj->data_data,
                   obj->header.size_data);

        text_offset += obj->header.size_text;
        data_offset += obj->header.size_data;
        bss_offset  += obj->header.size_bss;
    }

    /* --- Paso 3: construir tabla global de simbolos --- */
    for (int i = 0; i < linker->obj_count; i++) {
        LoadedObject *lo  = &linker->objects[i];
        ObjectFile   *obj = lo->obj;

        for (unsigned int j = 0; j < obj->header.num_symbols; j++) {
            ObjSymbol *sym = &obj->symbols[j];
            if (!sym->is_global || !sym->defined || sym->is_extern)
                continue;

            unsigned int final_offset;
            if (sym->section == OBJ_SEC_TEXT)
                final_offset = lo->text_base + sym->value;
            else if (sym->section == OBJ_SEC_DATA)
                final_offset = lo->data_base + sym->value;
            else
                final_offset = lo->bss_base + sym->value;

            if (global_add(&linker->globals, sym->name,
                           final_offset, sym->section) < 0) {
                linker->error_count++;
            }
        }
    }

    /* --- Paso 4: verificar simbolos externos resueltos --- */
    for (int i = 0; i < linker->obj_count; i++) {
        ObjectFile *obj = linker->objects[i].obj;
        for (unsigned int j = 0; j < obj->header.num_symbols; j++) {
            ObjSymbol *sym = &obj->symbols[j];
            if (!sym->is_extern) continue;
            if (!global_find(&linker->globals, sym->name)) {
                fprintf(stderr,
                        "Error: simbolo externo no resuelto '%s'\n",
                        sym->name);
                linker->error_count++;
            }
        }
    }

    if (linker->error_count > 0) return -1;

    /* --- Paso 5: aplicar relocaciones --- */
    for (int i = 0; i < linker->obj_count; i++) {
        LoadedObject *lo  = &linker->objects[i];
        ObjectFile   *obj = lo->obj;

        for (unsigned int j = 0; j < obj->header.num_relocations; j++) {
            ObjRelocation *rel = &obj->relocations[j];
            GlobalSymbol  *sym = global_find(&linker->globals,
                                             rel->symbol_name);
            if (!sym) {
                fprintf(stderr,
                        "Error: simbolo no encontrado en relocacion '%s'\n",
                        rel->symbol_name);
                linker->error_count++;
                continue;
            }

            unsigned int patch_offset = lo->text_base + rel->offset;
            int value;

            if (rel->type == OBJ_RELOC_R32) {
                value = (int)sym->final_offset;
            } else {
                /* PC32: relativo al siguiente byte despues del campo */
                value = (int)sym->final_offset -
                        (int)(patch_offset + 4);
            }

            /* Escribir valor en little endian */
            linker->text[patch_offset]     =  value        & 0xFF;
            linker->text[patch_offset + 1] = (value >> 8)  & 0xFF;
            linker->text[patch_offset + 2] = (value >> 16) & 0xFF;
            linker->text[patch_offset + 3] = (value >> 24) & 0xFF;
        }
    }

    printf("Enlazado exitoso: %d objetos procesados\n",
           linker->obj_count);
    return 0;
}

/* ============================================================
   ESCRITURA DEL BINARIO FINAL
   ============================================================ */

int linker_write_binary(Linker *linker, const char *filename) {
    FILE *file = fopen(filename, "wb");
    if (!file) {
        fprintf(stderr, "Error: no se pudo crear '%s'\n", filename);
        return -1;
    }

    /* Escribir .text */
    if (linker->text_size > 0)
        fwrite(linker->text, 1, linker->text_size, file);

    /* Escribir .data */
    if (linker->data_size > 0)
        fwrite(linker->data, 1, linker->data_size, file);

    /* Escribir .bss como ceros */
    if (linker->bss_size > 0) {
        unsigned char *zeros = (unsigned char *)calloc(linker->bss_size, 1);
        fwrite(zeros, 1, linker->bss_size, file);
        free(zeros);
    }

    fclose(file);
    printf("Binario final escrito: %s\n", filename);
    return 0;
}

/* ============================================================
   IMPRESION PARA DEPURACION
   ============================================================ */

void linker_print(Linker *linker) {
    printf("\n=== LINKER ===\n");
    printf("Objetos cargados: %d\n", linker->obj_count);
    printf("Tamanio .text:    %d bytes\n", linker->text_size);
    printf("Tamanio .data:    %d bytes\n", linker->data_size);
    printf("Tamanio .bss:     %d bytes\n", linker->bss_size);

    printf("\n--- SIMBOLOS GLOBALES ---\n");
    for (int i = 0; i < linker->globals.count; i++) {
        GlobalSymbol *s = &linker->globals.symbols[i];
        printf("  %-20s offset=0x%08X\n", s->name, s->final_offset);
    }

    printf("\n--- CODIGO FINAL (.text) ---\n");
    for (unsigned int i = 0; i < linker->text_size; i++) {
        printf("%02X ", linker->text[i]);
        if ((i + 1) % 16 == 0) printf("\n");
    }
    printf("\n==============\n");
}
