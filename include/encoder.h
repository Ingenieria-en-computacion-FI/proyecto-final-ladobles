#ifndef ENCODER_H
#define ENCODER_H

#include "parser.h"
#include "symbols.h"

#define REG_EAX 0
#define REG_ECX 1
#define REG_EDX 2
#define REG_EBX 3
#define REG_ESP 4
#define REG_EBP 5
#define REG_ESI 6
#define REG_EDI 7
#define REG_AL  8
#define REG_CL  9
#define REG_DL  10
#define REG_BL  11
#define REG_AH  12
#define REG_CH  13
#define REG_DH  14
#define REG_BH  15
#define REG_NONE -1

unsigned char build_modrm(unsigned char mod,
                           unsigned char reg,
                           unsigned char rm);

unsigned char build_sib(unsigned char scale,
                         unsigned char index,
                         unsigned char base);

int reg_is8(int reg);
int reg_is32(int reg);
int reg_number(const char *name);
int get_instruction_size(ASTNode *node);
const char *get_pending_symbol(ASTNode *node);
int get_pending_offset(ASTNode *node);
int encode_instruction(ASTNode *node, SymbolTable *symbols,
                       unsigned char *buf, int current_offset);

#endif /* ENCODER_H */
