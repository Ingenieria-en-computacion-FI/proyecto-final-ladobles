#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../include/object.h"

/* ============================================================
   ESCRITURA DEL ARCHIVO OBJETO
   ============================================================ */

int obj_write(const char *filename, Assembler *assembler) {
    FILE *file = fopen(filename, "wb");
    if (!file) {
        fprintf(stderr, "Error: no se pudo crear el archivo '%s'\n", filename);
        return -1;
    }

    /* Calcular offsets */
    unsigned int offset = sizeof(ObjHeader);
    unsigned int offset_sections = offset;
    offset += 3 * sizeof(ObjSection); /* .text, .data, .bss */

    unsigned int offset_symbols = offset;
    offset += assembler->symbols->count * sizeof(ObjSymbol);

    unsigned int offset_relocs = offset;
    offset += assembler->reloc_count * sizeof(ObjRelocation);

    unsigned int offset_text = offset;
    offset += assembler->text.size;

    unsigned int offset_data = offset;

    /* Escribir encabezado */
    ObjHeader header;
    memset(&header, 0, sizeof(ObjHeader));
    header.magic           = OBJ_MAGIC;
    header.version         = OBJ_VERSION;
    header.num_sections    = 3;
    header.num_symbols     = assembler->symbols->count;
    header.num_relocations = assembler->reloc_count;
    header.offset_sections = offset_sections;
    header.offset_symbols  = offset_symbols;
    header.offset_relocs   = offset_relocs;
    header.offset_text     = offset_text;
    header.size_text       = assembler->text.size;
    header.offset_data     = offset_data;
    header.size_data       = assembler->data.size;
    header.size_bss        = assembler->bss_size;
    fwrite(&header, sizeof(ObjHeader), 1, file);

    /* Escribir tabla de secciones */
    ObjSection sec;

    memset(&sec, 0, sizeof(ObjSection));
    strncpy(sec.name, ".text", 7);
    sec.type   = OBJ_SEC_TEXT;
    sec.offset = offset_text;
    sec.size   = assembler->text.size;
    fwrite(&sec, sizeof(ObjSection), 1, file);

    memset(&sec, 0, sizeof(ObjSection));
    strncpy(sec.name, ".data", 7);
    sec.type   = OBJ_SEC_DATA;
    sec.offset = offset_data;
    sec.size   = assembler->data.size;
    fwrite(&sec, sizeof(ObjSection), 1, file);

    memset(&sec, 0, sizeof(ObjSection));
    strncpy(sec.name, ".bss", 7);
    sec.type   = OBJ_SEC_BSS;
    sec.offset = 0;
    sec.size   = assembler->bss_size;
    fwrite(&sec, sizeof(ObjSection), 1, file);

    /* Escribir tabla de simbolos */
    for (int i = 0; i < assembler->symbols->count; i++) {
        Symbol    *sym = &assembler->symbols->symbols[i];
        ObjSymbol  osym;
        memset(&osym, 0, sizeof(ObjSymbol));
        strncpy(osym.name, sym->name, 63);
        osym.value     = sym->offset;
        osym.section   = sym->section;
        osym.is_global = sym->is_global;
        osym.is_extern = sym->is_extern;
        osym.defined   = sym->defined;
        fwrite(&osym, sizeof(ObjSymbol), 1, file);
    }

    /* Escribir tabla de relocaciones */
    for (int i = 0; i < assembler->reloc_count; i++) {
        Relocation    *rel = &assembler->relocations[i];
        ObjRelocation  orel;
        memset(&orel, 0, sizeof(ObjRelocation));
        orel.offset = rel->offset;
        strncpy(orel.symbol_name, rel->symbol_name, 63);
        orel.type = rel->type;
        fwrite(&orel, sizeof(ObjRelocation), 1, file);
    }

    /* Escribir bytes de .text */
    if (assembler->text.size > 0)
        fwrite(assembler->text.data, 1, assembler->text.size, file);

    /* Escribir bytes de .data */
    if (assembler->data.size > 0)
        fwrite(assembler->data.data, 1, assembler->data.size, file);

    fclose(file);
    printf("Archivo objeto escrito: %s\n", filename);
    return 0;
}

/* ============================================================
   LECTURA DEL ARCHIVO OBJETO
   ============================================================ */

ObjectFile *obj_read(const char *filename) {
    FILE *file = fopen(filename, "rb");
    if (!file) {
        fprintf(stderr, "Error: no se pudo abrir '%s'\n", filename);
        return NULL;
    }

    ObjectFile *obj = (ObjectFile *)malloc(sizeof(ObjectFile));
    if (!obj) { fclose(file); return NULL; }
    memset(obj, 0, sizeof(ObjectFile));

    /* Leer y validar encabezado */
    fread(&obj->header, sizeof(ObjHeader), 1, file);
    if (obj->header.magic != OBJ_MAGIC) {
        fprintf(stderr, "Error: formato objeto invalido '%s'\n", filename);
        free(obj);
        fclose(file);
        return NULL;
    }

    /* Leer tabla de secciones */
    obj->sections = (ObjSection *)malloc(
                    obj->header.num_sections * sizeof(ObjSection));
    fseek(file, obj->header.offset_sections, SEEK_SET);
    fread(obj->sections, sizeof(ObjSection),
          obj->header.num_sections, file);

    /* Leer tabla de simbolos */
    obj->symbols = (ObjSymbol *)malloc(
                   obj->header.num_symbols * sizeof(ObjSymbol));
    fseek(file, obj->header.offset_symbols, SEEK_SET);
    fread(obj->symbols, sizeof(ObjSymbol),
          obj->header.num_symbols, file);

    /* Leer tabla de relocaciones */
    obj->relocations = (ObjRelocation *)malloc(
                       obj->header.num_relocations * sizeof(ObjRelocation));
    fseek(file, obj->header.offset_relocs, SEEK_SET);
    fread(obj->relocations, sizeof(ObjRelocation),
          obj->header.num_relocations, file);

    /* Leer bytes de .text */
    if (obj->header.size_text > 0) {
        obj->text_data = (unsigned char *)malloc(obj->header.size_text);
        fseek(file, obj->header.offset_text, SEEK_SET);
        fread(obj->text_data, 1, obj->header.size_text, file);
    }

    /* Leer bytes de .data */
    if (obj->header.size_data > 0) {
        obj->data_data = (unsigned char *)malloc(obj->header.size_data);
        fseek(file, obj->header.offset_data, SEEK_SET);
        fread(obj->data_data, 1, obj->header.size_data, file);
    }

    fclose(file);
    return obj;
}

/* ============================================================
   LIBERACION DE MEMORIA
   ============================================================ */

void obj_destroy(ObjectFile *obj) {
    if (obj) {
        free(obj->sections);
        free(obj->symbols);
        free(obj->relocations);
        free(obj->text_data);
        free(obj->data_data);
        free(obj);
    }
}

/* ============================================================
   IMPRESION PARA DEPURACION
   ============================================================ */

void obj_print(ObjectFile *obj) {
    printf("\n=== ARCHIVO OBJETO ===\n");
    printf("Magic:       0x%08X\n", obj->header.magic);
    printf("Version:     %d\n",     obj->header.version);
    printf("Secciones:   %d\n",     obj->header.num_sections);
    printf("Simbolos:    %d\n",     obj->header.num_symbols);
    printf("Relocaciones:%d\n",     obj->header.num_relocations);

    printf("\n--- SECCIONES ---\n");
    for (unsigned int i = 0; i < obj->header.num_sections; i++) {
        ObjSection *s = &obj->sections[i];
        printf("  %-8s offset=%-6d size=%d\n",
               s->name, s->offset, s->size);
    }

    printf("\n--- SIMBOLOS ---\n");
    for (unsigned int i = 0; i < obj->header.num_symbols; i++) {
        ObjSymbol *s = &obj->symbols[i];
        printf("  %-20s value=%-6d global=%-3s extern=%-3s defined=%s\n",
               s->name, s->value,
               s->is_global ? "si" : "no",
               s->is_extern ? "si" : "no",
               s->defined   ? "si" : "no");
    }

    printf("\n--- RELOCACIONES ---\n");
    for (unsigned int i = 0; i < obj->header.num_relocations; i++) {
        ObjRelocation *r = &obj->relocations[i];
        printf("  offset=%-6d simbolo=%-20s tipo=%s\n",
               r->offset, r->symbol_name,
               r->type == OBJ_RELOC_R32 ? "R32" : "PC32");
    }

    printf("\n--- CODIGO .text ---\n");
    for (unsigned int i = 0; i < obj->header.size_text; i++) {
        printf("%02X ", obj->text_data[i]);
        if ((i + 1) % 16 == 0) printf("\n");
    }
    printf("\n======================\n");
}
