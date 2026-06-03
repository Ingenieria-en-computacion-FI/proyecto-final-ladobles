#ifndef PARSER_H
#define PARSER_H

#include "lexer.h"

/* Tipos de operandos */
typedef enum {
    OPERAND_IMMEDIATE,  /* valor inmediato: MOV EAX, 10 */
    OPERAND_REGISTER,   /* registro: MOV EAX, EBX */
    OPERAND_MEMORY      /* memoria: MOV EAX, [EBX+4] */
} OperandType;

/* Estructura que representa un operando */
typedef struct {
    OperandType type;
    char base[16];      /* registro base: EBX */
    char index[16];     /* registro indice: ECX (para SIB) */
    int  scale;         /* escala: 1, 2, 4, 8 (para SIB) */
    int  displacement;  /* desplazamiento: [EBP+4] -> 4 */
    int  immediate;     /* valor inmediato */
} Operand;

/* Tipos de nodo en el AST */
typedef enum {
    NODE_INSTRUCTION,   /* instruccion: MOV, ADD, JMP... */
    NODE_DIRECTIVE,     /* directiva: SECTION, GLOBAL... */
    NODE_LABEL          /* etiqueta: main:, loop:... */
} NodeType;

/* Estructura que representa una instruccion o directiva */
typedef struct {
    NodeType type;
    char     label[64];         /* etiqueta si tiene */
    char     mnemonic[16];      /* MOV, ADD, SECTION... */
    Operand  operands[3];       /* maximo 3 operandos */
    int      operand_count;     /* cuantos operandos tiene */
    int      line;              /* linea del codigo fuente */
} ASTNode;

/* Estructura del parser */
typedef struct {
    Lexer   *lexer;             /* lexer asociado */
    Token    current;           /* token actual */
    Token    next;              /* token siguiente (lookahead) */
    int      error_count;       /* numero de errores encontrados */
} Parser;

/* Funciones del parser */
Parser  *parser_create(Lexer *lexer);
void     parser_destroy(Parser *parser);
ASTNode *parser_parse(Parser *parser, int *node_count);
void     ast_node_print(ASTNode *node);

#endif /* PARSER_H */
