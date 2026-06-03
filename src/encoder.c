#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../include/encoder.h"

/* ============================================================
   UTILIDADES DE REGISTROS
   ============================================================ */

/* Retorna el numero de registro IA-32 a partir de su nombre */
int reg_number(const char *name) {
    if (strcmp(name, "EAX") == 0) return REG_EAX;
    if (strcmp(name, "ECX") == 0) return REG_ECX;
    if (strcmp(name, "EDX") == 0) return REG_EDX;
    if (strcmp(name, "EBX") == 0) return REG_EBX;
    if (strcmp(name, "ESP") == 0) return REG_ESP;
    if (strcmp(name, "EBP") == 0) return REG_EBP;
    if (strcmp(name, "ESI") == 0) return REG_ESI;
    if (strcmp(name, "EDI") == 0) return REG_EDI;
    return -1;
}

/* ============================================================
   CONSTRUCCION DE MODRM Y SIB
   ============================================================ */

/* Construye el byte ModRM */
unsigned char build_modrm(unsigned char mod,
                           unsigned char reg,
                           unsigned char rm) {
    return (mod << 6) | (reg << 3) | rm;
}

/* Construye el byte SIB */
unsigned char build_sib(unsigned char scale,
                         unsigned char index,
                         unsigned char base) {
    return (scale << 6) | (index << 3) | base;
}

/* Convierte escala numerica a campo ss del SIB */
static unsigned char scale_to_ss(int scale) {
    switch (scale) {
        case 1: return 0;
        case 2: return 1;
        case 4: return 2;
        case 8: return 3;
        default: return 0;
    }
}

/* ============================================================
   CODIFICACION DE OPERANDOS DE MEMORIA
   ============================================================ */

/* Escribe los bytes de direccionamiento de memoria en buf
   Retorna cuantos bytes escribio */
static int encode_memory(Operand *op, int reg_field,
                          unsigned char *buf) {
    int pos = 0;
    int has_base  = op->base[0]  != '\0';
    int has_index = op->index[0] != '\0';
    int has_disp  = op->displacement != 0;
    int base_reg  = has_base  ? reg_number(op->base)  : 0;
    int index_reg = has_index ? reg_number(op->index) : 0;

    /* Memoria directa: [1000] sin registros */
    if (!has_base && !has_index) {
        buf[pos++] = build_modrm(0x00, reg_field, 0x05);
        buf[pos++] =  op->displacement        & 0xFF;
        buf[pos++] = (op->displacement >> 8)  & 0xFF;
        buf[pos++] = (op->displacement >> 16) & 0xFF;
        buf[pos++] = (op->displacement >> 24) & 0xFF;
        return pos;
    }

    /* Determinar mod segun desplazamiento */
    unsigned char mod;
    if (!has_disp)
        mod = 0x00;
    else if (op->displacement >= -128 && op->displacement <= 127)
        mod = 0x01;
    else
        mod = 0x02;

    /* Necesita SIB si hay indice o si la base es ESP */
    int needs_sib = has_index || (has_base && base_reg == REG_ESP);

    if (needs_sib) {
        buf[pos++] = build_modrm(mod, reg_field, 0x04);
        buf[pos++] = build_sib(scale_to_ss(op->scale),
                               has_index ? index_reg : 0x04,
                               base_reg);
    } else {
        buf[pos++] = build_modrm(mod, reg_field, base_reg);
    }

    /* Desplazamiento */
    if (mod == 0x01) {
        buf[pos++] = op->displacement & 0xFF;
    } else if (mod == 0x02) {
        buf[pos++] =  op->displacement        & 0xFF;
        buf[pos++] = (op->displacement >> 8)  & 0xFF;
        buf[pos++] = (op->displacement >> 16) & 0xFF;
        buf[pos++] = (op->displacement >> 24) & 0xFF;
    }

    return pos;
}

/* ============================================================
   SIMBOLOS PENDIENTES
   ============================================================ */

/* Retorna el nombre del simbolo pendiente en una instruccion */
const char *get_pending_symbol(ASTNode *node) {
    for (int i = 0; i < node->operand_count; i++) {
        if (node->operands[i].type == OPERAND_IMMEDIATE &&
            node->operands[i].base[0] != '\0')
            return node->operands[i].base;
    }
    return NULL;
}

/* Retorna el offset dentro de la instruccion donde va el simbolo */
int get_pending_offset(ASTNode *node) {
    /* Para la mayoria de instrucciones el simbolo va al final */
    return get_instruction_size(node) - 4;
}

/* ============================================================
   CALCULO DE TAMANIO DE INSTRUCCIONES
   ============================================================ */

int get_instruction_size(ASTNode *node) {
    const char *m = node->mnemonic;

    /* Instrucciones de un solo byte */
    if (strcmp(m, "RET") == 0) return 1;
    if (strcmp(m, "NOP") == 0) return 1;

    /* PUSH/POP registro */
    if (strcmp(m, "PUSH") == 0 || strcmp(m, "POP") == 0) {
        if (node->operand_count > 0 &&
            node->operands[0].type == OPERAND_REGISTER)
            return 1;
        return 5; /* PUSH imm32 */
    }

    /* Saltos cortos y CALL */
    if (strcmp(m, "JMP")  == 0 || strcmp(m, "CALL") == 0) return 5;
    if (strcmp(m, "JE")   == 0 || strcmp(m, "JNE")  == 0) return 6;
    if (strcmp(m, "JG")   == 0 || strcmp(m, "JL")   == 0) return 6;
    if (strcmp(m, "JGE")  == 0 || strcmp(m, "JLE")  == 0) return 6;

    /* INT */
    if (strcmp(m, "INT")  == 0) return 2;

    /* INC/DEC/NEG/NOT registro */
    if (strcmp(m, "INC")  == 0 || strcmp(m, "DEC") == 0) return 2;
    if (strcmp(m, "NEG")  == 0 || strcmp(m, "NOT") == 0) return 2;

    /* MUL/DIV/IMUL/IDIV */
    if (strcmp(m, "MUL")  == 0 || strcmp(m, "DIV")  == 0) return 2;
    if (strcmp(m, "IMUL") == 0 || strcmp(m, "IDIV") == 0) return 2;

    /* MOV reg, imm32 */
    if (strcmp(m, "MOV") == 0) {
        if (node->operand_count == 2) {
            if (node->operands[0].type == OPERAND_REGISTER &&
                node->operands[1].type == OPERAND_IMMEDIATE)
                return 5;
            /* MOV reg, reg */
            if (node->operands[0].type == OPERAND_REGISTER &&
                node->operands[1].type == OPERAND_REGISTER)
                return 2;
            /* MOV reg, mem o MOV mem, reg */
            return 6;
        }
    }

    /* ADD, SUB, CMP, AND, OR, XOR reg, imm */
    if (strcmp(m, "ADD") == 0 || strcmp(m, "SUB") == 0 ||
        strcmp(m, "CMP") == 0 || strcmp(m, "AND") == 0 ||
        strcmp(m, "OR")  == 0 || strcmp(m, "XOR") == 0) {
        if (node->operand_count == 2 &&
            node->operands[1].type == OPERAND_IMMEDIATE)
            return 6;
        return 2;
    }

    /* LEA */
    if (strcmp(m, "LEA") == 0) return 6;

    return 4; /* tamanio por defecto */
}

/* ============================================================
   CODIFICACION DE INSTRUCCIONES
   ============================================================ */

int encode_instruction(ASTNode *node, SymbolTable *symbols,
                       unsigned char *buf, int current_offset) {
    int pos = 0;
    const char *m = node->mnemonic;
    Operand *op0 = node->operand_count > 0 ? &node->operands[0] : NULL;
    Operand *op1 = node->operand_count > 1 ? &node->operands[1] : NULL;

    /* --- RET --- */
    if (strcmp(m, "RET") == 0) {
        buf[pos++] = 0xC3;
        return pos;
    }

    /* --- NOP --- */
    if (strcmp(m, "NOP") == 0) {
        buf[pos++] = 0x90;
        return pos;
    }

    /* --- INT --- */
    if (strcmp(m, "INT") == 0) {
        buf[pos++] = 0xCD;
        buf[pos++] = op0->immediate & 0xFF;
        return pos;
    }

    /* --- PUSH --- */
    if (strcmp(m, "PUSH") == 0) {
        if (op0->type == OPERAND_REGISTER) {
            buf[pos++] = 0x50 + reg_number(op0->base);
        } else {
            buf[pos++] = 0x68;
            int v = op0->immediate;
            buf[pos++] =  v        & 0xFF;
            buf[pos++] = (v >> 8)  & 0xFF;
            buf[pos++] = (v >> 16) & 0xFF;
            buf[pos++] = (v >> 24) & 0xFF;
        }
        return pos;
    }

    /* --- POP --- */
    if (strcmp(m, "POP") == 0) {
        if (op0->type == OPERAND_REGISTER)
            buf[pos++] = 0x58 + reg_number(op0->base);
        return pos;
    }

    /* --- INC / DEC --- */
    if (strcmp(m, "INC") == 0 && op0->type == OPERAND_REGISTER) {
        buf[pos++] = 0x40 + reg_number(op0->base);
        return pos;
    }
    if (strcmp(m, "DEC") == 0 && op0->type == OPERAND_REGISTER) {
        buf[pos++] = 0x48 + reg_number(op0->base);
        return pos;
    }

    /* --- NEG / NOT --- */
    if (strcmp(m, "NEG") == 0) {
        buf[pos++] = 0xF7;
        buf[pos++] = build_modrm(0x03, 3, reg_number(op0->base));
        return pos;
    }
    if (strcmp(m, "NOT") == 0) {
        buf[pos++] = 0xF7;
        buf[pos++] = build_modrm(0x03, 2, reg_number(op0->base));
        return pos;
    }

    /* --- MUL / DIV / IMUL / IDIV --- */
    if (strcmp(m, "MUL") == 0) {
        buf[pos++] = 0xF7;
        buf[pos++] = build_modrm(0x03, 4, reg_number(op0->base));
        return pos;
    }
    if (strcmp(m, "DIV") == 0) {
        buf[pos++] = 0xF7;
        buf[pos++] = build_modrm(0x03, 6, reg_number(op0->base));
        return pos;
    }
    if (strcmp(m, "IMUL") == 0) {
        buf[pos++] = 0xF7;
        buf[pos++] = build_modrm(0x03, 5, reg_number(op0->base));
        return pos;
    }
    if (strcmp(m, "IDIV") == 0) {
        buf[pos++] = 0xF7;
        buf[pos++] = build_modrm(0x03, 7, reg_number(op0->base));
        return pos;
    }

    /* --- JMP --- */
    if (strcmp(m, "JMP") == 0) {
        Symbol *sym = op0->base[0] ? symbol_find(symbols, op0->base) : NULL;
        buf[pos++] = 0xE9;
        int target = sym ? sym->offset : 0;
        int rel    = target - (current_offset + 5);
        buf[pos++] =  rel        & 0xFF;
        buf[pos++] = (rel >> 8)  & 0xFF;
        buf[pos++] = (rel >> 16) & 0xFF;
        buf[pos++] = (rel >> 24) & 0xFF;
        if (!sym) return -1;
        return pos;
    }

    /* --- CALL --- */
    if (strcmp(m, "CALL") == 0) {
        Symbol *sym = op0->base[0] ? symbol_find(symbols, op0->base) : NULL;
        buf[pos++] = 0xE8;
        int target = sym ? sym->offset : 0;
        int rel    = target - (current_offset + 5);
        buf[pos++] =  rel        & 0xFF;
        buf[pos++] = (rel >> 8)  & 0xFF;
        buf[pos++] = (rel >> 16) & 0xFF;
        buf[pos++] = (rel >> 24) & 0xFF;
        if (!sym || sym->is_extern) return -1;
        return pos;
    }

    /* --- Saltos condicionales --- */
    unsigned char jcc_opcode = 0;
    if      (strcmp(m, "JE")  == 0) jcc_opcode = 0x84;
    else if (strcmp(m, "JNE") == 0) jcc_opcode = 0x85;
    else if (strcmp(m, "JG")  == 0) jcc_opcode = 0x8F;
    else if (strcmp(m, "JL")  == 0) jcc_opcode = 0x8C;
    else if (strcmp(m, "JGE") == 0) jcc_opcode = 0x8D;
    else if (strcmp(m, "JLE") == 0) jcc_opcode = 0x8E;

    if (jcc_opcode) {
        Symbol *sym = op0->base[0] ? symbol_find(symbols, op0->base) : NULL;
        buf[pos++] = 0x0F;
        buf[pos++] = jcc_opcode;
        int target = sym ? sym->offset : 0;
        int rel    = target - (current_offset + 6);
        buf[pos++] =  rel        & 0xFF;
        buf[pos++] = (rel >> 8)  & 0xFF;
        buf[pos++] = (rel >> 16) & 0xFF;
        buf[pos++] = (rel >> 24) & 0xFF;
        if (!sym) return -1;
        return pos;
    }

    /* --- MOV --- */
    if (strcmp(m, "MOV") == 0) {
        /* MOV reg, imm32 */
        if (op0->type == OPERAND_REGISTER &&
            op1->type == OPERAND_IMMEDIATE) {
            buf[pos++] = 0xB8 + reg_number(op0->base);
            int v = op1->immediate;
            buf[pos++] =  v        & 0xFF;
            buf[pos++] = (v >> 8)  & 0xFF;
            buf[pos++] = (v >> 16) & 0xFF;
            buf[pos++] = (v >> 24) & 0xFF;
            return pos;
        }
        /* MOV reg, reg */
        if (op0->type == OPERAND_REGISTER &&
            op1->type == OPERAND_REGISTER) {
            buf[pos++] = 0x89;
            buf[pos++] = build_modrm(0x03,
                                     reg_number(op1->base),
                                     reg_number(op0->base));
            return pos;
        }
        /* MOV reg, mem */
        if (op0->type == OPERAND_REGISTER &&
            op1->type == OPERAND_MEMORY) {
            buf[pos++] = 0x8B;
            pos += encode_memory(op1, reg_number(op0->base), buf + pos);
            return pos;
        }
        /* MOV mem, reg */
        if (op0->type == OPERAND_MEMORY &&
            op1->type == OPERAND_REGISTER) {
            buf[pos++] = 0x89;
            pos += encode_memory(op0, reg_number(op1->base), buf + pos);
            return pos;
        }
    }

    /* --- LEA --- */
    if (strcmp(m, "LEA") == 0) {
        if (op0->type == OPERAND_REGISTER &&
            op1->type == OPERAND_MEMORY) {
            buf[pos++] = 0x8D;
            pos += encode_memory(op1, reg_number(op0->base), buf + pos);
            return pos;
        }
    }

    /* --- ADD / SUB / CMP / AND / OR / XOR --- */
    unsigned char alu_opcode_rr = 0;
    unsigned char alu_opcode_ri = 0;
    unsigned char alu_ext       = 0;

    if      (strcmp(m, "ADD") == 0) { alu_opcode_rr=0x01; alu_opcode_ri=0x81; alu_ext=0; }
    else if (strcmp(m, "SUB") == 0) { alu_opcode_rr=0x29; alu_opcode_ri=0x81; alu_ext=5; }
    else if (strcmp(m, "CMP") == 0) { alu_opcode_rr=0x39; alu_opcode_ri=0x81; alu_ext=7; }
    else if (strcmp(m, "AND") == 0) { alu_opcode_rr=0x21; alu_opcode_ri=0x81; alu_ext=4; }
    else if (strcmp(m, "OR")  == 0) { alu_opcode_rr=0x09; alu_opcode_ri=0x81; alu_ext=1; }
    else if (strcmp(m, "XOR") == 0) { alu_opcode_rr=0x31; alu_opcode_ri=0x81; alu_ext=6; }

    if (alu_opcode_rr) {
        /* reg, reg */
        if (op1->type == OPERAND_REGISTER) {
            buf[pos++] = alu_opcode_rr;
            buf[pos++] = build_modrm(0x03,
                                     reg_number(op1->base),
                                     reg_number(op0->base));
            return pos;
        }
        /* reg, imm32 */
        if (op1->type == OPERAND_IMMEDIATE) {
            buf[pos++] = alu_opcode_ri;
            buf[pos++] = build_modrm(0x03, alu_ext,
                                     reg_number(op0->base));
            int v = op1->immediate;
            buf[pos++] =  v        & 0xFF;
            buf[pos++] = (v >> 8)  & 0xFF;
            buf[pos++] = (v >> 16) & 0xFF;
            buf[pos++] = (v >> 24) & 0xFF;
            return pos;
        }
    }

    fprintf(stderr, "Error: instruccion no soportada '%s' en linea %d\n",
            node->mnemonic, node->line);
    return -1;
}
