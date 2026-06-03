#ifndef ENCODER_H
#define ENCODER_H

#include "parser.h"
#include "symbols.h"

/* Numeros de registro IA-32 */
#define REG_EAX 0
#define REG_ECX 1
#define REG_EDX 2
#define REG_EBX 3
#define REG_ESP 4
#define REG_EBP 5
#define REG_ESI 6
#define REG_EDI 7

/* Construccion de bytes ModRM y SIB */
unsigned char build_modrm(unsigned char mod,
                           unsigned char reg,
                           unsigned char rm);

unsigned char build_sib(unsigned char scale,
                         unsigned char index,
                         unsigned char base);

/* Obtiene el numero de registro a partir de su nombre */
int reg_number(const char *name);

/* Calcula el tamanio en bytes de una instruccion */
int get_instruction_size(ASTNode *node);

/* Retorna el nombre del simbolo pendiente de una instruccion */
const char *get_pending_symbol(ASTNode *node);

/* Retorna el offset dentro de la instruccion donde va el simbolo */
int get_pending_offset(ASTNode *node);

/* Codifica una instruccion y escribe los bytes en buf
   Retorna el numero de bytes escritos, o -1 si hay simbolo pendiente */
int encode_instruction(ASTNode *node, SymbolTable *symbols,
                       unsigned char *buf, int current_offset);

#endif /* ENCODER_H */
