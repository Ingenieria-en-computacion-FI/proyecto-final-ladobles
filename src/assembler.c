#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../include/assembler.h"
#include "../include/encoder.h"

/* Crea un nuevo ensamblador */
Assembler *assembler_create() {
    Assembler *assembler = (Assembler *)malloc(sizeof(Assembler));
    if (!assembler) return NULL;

    assembler->symbols         = symbol_table_create();
    assembler->text.data       = (unsigned char *)malloc(4096);
    assembler->text.size       = 0;
    assembler->text.capacity   = 4096;
    assembler->data.data       = (unsigned char *)malloc(4096);
    assembler->data.size       = 0;
    assembler->data.capacity   = 4096;
    assembler->bss_size        = 0;
    assembler->fixups          = (Fixup *)malloc(64 * sizeof(Fixup));
    assembler->fixup_count     = 0;
    assembler->fixup_capacity  = 64;
    assembler->relocations     = (Relocation *)malloc(64 * sizeof(Relocation));
    assembler->reloc_count     = 0;
    assembler->reloc_capacity  = 64;
    assembler->current_section = SECTION_TEXT;
    assembler->error_count     = 0;
    return assembler;
}

/* Libera la memoria del ensamblador */
void assembler_destroy(Assembler *assembler) {
    if (assembler) {
        symbol_table_destroy(assembler->symbols);
        free(assembler->text.data);
        free(assembler->data.data);
        free(assembler->fixups);
        free(assembler->relocations);
        free(assembler);
    }
}

/* Emite un byte al buffer actual */
static void emit_byte(Assembler *assembler, unsigned char byte) {
    CodeBuffer *buf = (assembler->current_section == SECTION_TEXT)
                      ? &assembler->text
                      : &assembler->data;

    if (buf->size >= buf->capacity) {
        buf->capacity *= 2;
        buf->data      = (unsigned char *)realloc(buf->data, buf->capacity);
    }
    buf->data[buf->size++] = byte;
}

/* Emite un entero de 32 bits en little endian */
static void emit_int32(Assembler *assembler, int value) {
    emit_byte(assembler, (value)       & 0xFF);
    emit_byte(assembler, (value >> 8)  & 0xFF);
    emit_byte(assembler, (value >> 16) & 0xFF);
    emit_byte(assembler, (value >> 24) & 0xFF);
}

/* Agrega un fixup a la lista */
static void add_fixup(Assembler *assembler, int offset,
                      const char *name, FixupType type) {
    if (assembler->fixup_count >= assembler->fixup_capacity) {
        assembler->fixup_capacity *= 2;
        assembler->fixups = (Fixup *)realloc(assembler->fixups,
                            assembler->fixup_capacity * sizeof(Fixup));
    }
    Fixup *f = &assembler->fixups[assembler->fixup_count++];
    f->offset_in_code = offset;
    strncpy(f->symbol_name, name, 63);
    f->type    = type;
    f->section = assembler->current_section;
}

/* Agrega una relocacion a la lista */
static void add_relocation(Assembler *assembler, int offset,
                           const char *name, RelocType type) {
    if (assembler->reloc_count >= assembler->reloc_capacity) {
        assembler->reloc_capacity *= 2;
        assembler->relocations = (Relocation *)realloc(assembler->relocations,
                                 assembler->reloc_capacity * sizeof(Relocation));
    }
    Relocation *r = &assembler->relocations[assembler->reloc_count++];
    r->offset = offset;
    strncpy(r->symbol_name, name, 63);
    r->type = type;
}

/* Procesa una directiva */
static void process_directive(Assembler *assembler, ASTNode *node) {
    if (strcmp(node->mnemonic, "SECTION") == 0) {
        if (node->operand_count > 0) {
            char *name = node->operands[0].base;
            if (strstr(name, "TEXT"))      assembler->current_section = SECTION_TEXT;
            else if (strstr(name, "DATA")) assembler->current_section = SECTION_DATA;
            else if (strstr(name, "BSS"))  assembler->current_section = SECTION_BSS;
        }
    }
    else if (strcmp(node->mnemonic, "GLOBAL") == 0) {
        if (node->operand_count > 0)
            symbol_mark_global(assembler->symbols, node->operands[0].base);
    }
    else if (strcmp(node->mnemonic, "EXTERN") == 0) {
        if (node->operand_count > 0)
            symbol_mark_extern(assembler->symbols, node->operands[0].base);
    }
    else if (strcmp(node->mnemonic, "DB") == 0) {
        for (int i = 0; i < node->operand_count; i++)
            emit_byte(assembler, (unsigned char)node->operands[i].immediate);
    }
    else if (strcmp(node->mnemonic, "DW") == 0) {
        for (int i = 0; i < node->operand_count; i++) {
            int v = node->operands[i].immediate;
            emit_byte(assembler, v & 0xFF);
            emit_byte(assembler, (v >> 8) & 0xFF);
        }
    }
    else if (strcmp(node->mnemonic, "DD") == 0) {
        for (int i = 0; i < node->operand_count; i++)
            emit_int32(assembler, node->operands[i].immediate);
    }
    else if (strcmp(node->mnemonic, "RESB") == 0) {
        if (node->operand_count > 0)
            assembler->bss_size += node->operands[0].immediate;
    }
    else if (strcmp(node->mnemonic, "RESW") == 0) {
        if (node->operand_count > 0)
            assembler->bss_size += node->operands[0].immediate * 2;
    }
    else if (strcmp(node->mnemonic, "RESD") == 0) {
        if (node->operand_count > 0)
            assembler->bss_size += node->operands[0].immediate * 4;
    }
}

/* Resuelve los fixups al final de la pasada unica */
static void resolve_fixups(Assembler *assembler) {
    for (int i = 0; i < assembler->fixup_count; i++) {
        Fixup  *f   = &assembler->fixups[i];
        Symbol *sym = symbol_find(assembler->symbols, f->symbol_name);

        if (!sym || !sym->defined) {
            fprintf(stderr, "Error: simbolo no definido '%s'\n",
                    f->symbol_name);
            assembler->error_count++;
            continue;
        }

        int value;
        if (f->type == FIXUP_RELATIVE)
            value = sym->offset - (f->offset_in_code + 4);
        else
            value = sym->offset;

        /* Parchear los 4 bytes en el buffer */
        assembler->text.data[f->offset_in_code]     =  value        & 0xFF;
        assembler->text.data[f->offset_in_code + 1] = (value >> 8)  & 0xFF;
        assembler->text.data[f->offset_in_code + 2] = (value >> 16) & 0xFF;
        assembler->text.data[f->offset_in_code + 3] = (value >> 24) & 0xFF;
    }
}

/* ============================================================
   ENSAMBLADOR DE UNA PASADA
   ============================================================ */
int assembler_one_pass(Assembler *assembler,
                       ASTNode *nodes, int count) {
    int offset = 0;

    for (int i = 0; i < count; i++) {
        ASTNode *node = &nodes[i];

        /* Registrar etiqueta */
        if (node->label[0] != '\0') {
            symbol_add(assembler->symbols, node->label,
                       assembler->current_section, offset, 0, 0);
        }

        /* Procesar directiva */
        if (node->type == NODE_DIRECTIVE) {
            process_directive(assembler, node);
            continue;
        }

        /* Saltar nodos que solo son etiquetas */
        if (node->type == NODE_LABEL) continue;

        /* Codificar instruccion */
        unsigned char bytes[16];
        int size = encode_instruction(node, assembler->symbols,
                                      bytes, offset);
        if (size < 0) {
            /* Referencia adelantada: emitir placeholder */
            Symbol *sym = symbol_find(assembler->symbols,
                                      get_pending_symbol(node));
            if (sym && sym->is_extern) {
                add_relocation(assembler, offset,
                               get_pending_symbol(node), RELOC_PC32);
            } else {
                add_fixup(assembler, offset + get_pending_offset(node),
                          get_pending_symbol(node), FIXUP_RELATIVE);
            }
            size = get_instruction_size(node);
            memset(bytes, 0, size);
        }

        for (int j = 0; j < size; j++)
            emit_byte(assembler, bytes[j]);
        offset += size;
    }

    resolve_fixups(assembler);
    return assembler->error_count == 0 ? 0 : -1;
}

/* ============================================================
   ENSAMBLADOR DE DOS PASADAS
   ============================================================ */
int assembler_two_pass(Assembler *assembler,
                       ASTNode *nodes, int count) {

    /* --- PRIMERA PASADA: construir tabla de simbolos --- */
    int offset = 0;
    for (int i = 0; i < count; i++) {
        ASTNode *node = &nodes[i];

        if (node->label[0] != '\0') {
            symbol_add(assembler->symbols, node->label,
                       assembler->current_section, offset, 0, 0);
        }

        if (node->type == NODE_DIRECTIVE) {
            if (strcmp(node->mnemonic, "SECTION") == 0)
                process_directive(assembler, node);
            else if (strcmp(node->mnemonic, "GLOBAL") == 0)
                process_directive(assembler, node);
            else if (strcmp(node->mnemonic, "EXTERN") == 0)
                process_directive(assembler, node);
            continue;
        }

        if (node->type == NODE_LABEL) continue;

        offset += get_instruction_size(node);
    }

    /* --- SEGUNDA PASADA: generar codigo definitivo --- */
    offset = 0;
    assembler->current_section = SECTION_TEXT;

    for (int i = 0; i < count; i++) {
        ASTNode *node = &nodes[i];

        if (node->type == NODE_DIRECTIVE) {
            process_directive(assembler, node);
            continue;
        }

        if (node->type == NODE_LABEL) continue;

        unsigned char bytes[16];
        int size = encode_instruction(node, assembler->symbols,
                                      bytes, offset);
        if (size < 0) {
            /* Simbolo externo: generar relocacion */
            add_relocation(assembler, offset + get_pending_offset(node),
                           get_pending_symbol(node), RELOC_PC32);
            size = get_instruction_size(node);
            memset(bytes, 0, size);
        }

        for (int j = 0; j < size; j++)
            emit_byte(assembler, bytes[j]);
        offset += size;
    }

    return assembler->error_count == 0 ? 0 : -1;
}

/* Imprime el codigo generado en hexadecimal */
void assembler_print_code(Assembler *assembler) {
    printf("\n=== CODIGO GENERADO (.text) ===\n");
    for (int i = 0; i < assembler->text.size; i++) {
        printf("%02X ", assembler->text.data[i]);
        if ((i + 1) % 16 == 0) printf("\n");
    }
    printf("\n==============================\n");
}
