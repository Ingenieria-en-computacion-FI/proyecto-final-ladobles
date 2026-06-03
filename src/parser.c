#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../include/parser.h"

/* Crea un nuevo parser */
Parser *parser_create(Lexer *lexer) {
    Parser *parser = (Parser *)malloc(sizeof(Parser));
    if (!parser) return NULL;

    parser->lexer       = lexer;
    parser->error_count = 0;
    parser->current     = lexer_next_token(lexer);
    parser->next        = lexer_next_token(lexer);
    return parser;
}

/* Libera la memoria del parser */
void parser_destroy(Parser *parser) {
    if (parser) free(parser);
}

/* Avanza al siguiente token */
static void advance(Parser *parser) {
    parser->current = parser->next;
    parser->next    = lexer_next_token(parser->lexer);
}

/* Salta saltos de linea */
static void skip_newlines(Parser *parser) {
    while (parser->current.type == TOKEN_NEWLINE)
        advance(parser);
}

/* Reporta un error */
static void parse_error(Parser *parser, const char *msg) {
    fprintf(stderr, "Error linea %d: %s (encontrado: '%s')\n",
            parser->current.line, msg, parser->current.lexeme);
    parser->error_count++;
}

/* Parsea un operando de memoria: [base + index*scale + disp] */
static Operand parse_memory(Parser *parser) {
    Operand op;
    memset(&op, 0, sizeof(Operand));
    op.type  = OPERAND_MEMORY;
    op.scale = 1;

    advance(parser); /* consumir [ */

    /* Memoria directa: [1000] */
    if (parser->current.type == TOKEN_NUMBER) {
        op.displacement = (int)strtol(parser->current.lexeme, NULL, 0);
        advance(parser);
    }
    /* Base register */
    else if (parser->current.type == TOKEN_REGISTER) {
        strncpy(op.base, parser->current.lexeme, 15);
        advance(parser);

        /* [BASE + ...] */
        if (parser->current.type == TOKEN_PLUS) {
            advance(parser); /* consumir + */

            /* [BASE + INDEX...] */
            if (parser->current.type == TOKEN_REGISTER) {
                strncpy(op.index, parser->current.lexeme, 15);
                advance(parser);

                /* [BASE + INDEX * SCALE] */
                if (parser->current.type == TOKEN_STAR) {
                    advance(parser); /* consumir * */
                    op.scale = (int)strtol(parser->current.lexeme, NULL, 0);
                    advance(parser);

                    /* [BASE + INDEX * SCALE + DISP] */
                    if (parser->current.type == TOKEN_PLUS) {
                        advance(parser); /* consumir + */
                        op.displacement = (int)strtol(parser->current.lexeme, NULL, 0);
                        advance(parser);
                    }
                }
            }
            /* [BASE + DISP] */
            else if (parser->current.type == TOKEN_NUMBER) {
                op.displacement = (int)strtol(parser->current.lexeme, NULL, 0);
                advance(parser);
            }
        }
    }

    /* Consumir ] */
    if (parser->current.type != TOKEN_RBRACKET)
        parse_error(parser, "se esperaba ']'");
    else
        advance(parser);

    return op;
}

/* Parsea un operando */
static Operand parse_operand(Parser *parser) {
    Operand op;
    memset(&op, 0, sizeof(Operand));
    op.scale = 1;

    if (parser->current.type == TOKEN_REGISTER) {
        op.type = OPERAND_REGISTER;
        strncpy(op.base, parser->current.lexeme, 15);
        advance(parser);
    }
    else if (parser->current.type == TOKEN_NUMBER) {
        op.type      = OPERAND_IMMEDIATE;
        op.immediate = (int)strtol(parser->current.lexeme, NULL, 0);
        advance(parser);
    }
    else if (parser->current.type == TOKEN_LBRACKET) {
        op = parse_memory(parser);
    }
    else {
        parse_error(parser, "operando invalido");
    }

    return op;
}

/* Parsea una linea completa */
static ASTNode parse_line(Parser *parser) {
    ASTNode node;
    memset(&node, 0, sizeof(ASTNode));
    node.line = parser->current.line;

    /* Detectar etiqueta: IDENTIFICADOR : */
    if (parser->current.type == TOKEN_IDENTIFIER &&
        parser->next.type    == TOKEN_COLON) {
        strncpy(node.label, parser->current.lexeme, 63);
        advance(parser); /* consumir identificador */
        advance(parser); /* consumir : */
        node.type = NODE_LABEL;

        /* Puede haber instruccion en la misma linea despues de la etiqueta */
        if (parser->current.type == TOKEN_NEWLINE ||
            parser->current.type == TOKEN_EOF)
            return node;
    }

    /* Directiva */
    if (parser->current.type == TOKEN_DIRECTIVE) {
        node.type = NODE_DIRECTIVE;
        strncpy(node.mnemonic, parser->current.lexeme, 15);
        advance(parser);

        /* Leer operandos de la directiva */
        while (parser->current.type != TOKEN_NEWLINE &&
               parser->current.type != TOKEN_EOF) {
            if (parser->current.type == TOKEN_COMMA) {
                advance(parser);
                continue;
            }
            node.operands[node.operand_count++] = parse_operand(parser);
        }
        return node;
    }

    /* Instruccion */
    if (parser->current.type == TOKEN_INSTRUCTION) {
        node.type = NODE_INSTRUCTION;
        strncpy(node.mnemonic, parser->current.lexeme, 15);
        advance(parser);

        /* Leer operandos */
        if (parser->current.type != TOKEN_NEWLINE &&
            parser->current.type != TOKEN_EOF) {
            node.operands[node.operand_count++] = parse_operand(parser);

            while (parser->current.type == TOKEN_COMMA) {
                advance(parser); /* consumir , */
                node.operands[node.operand_count++] = parse_operand(parser);
            }
        }
        return node;
    }

    return node;
}

/* Parsea el archivo completo y retorna un arreglo de nodos */
ASTNode *parser_parse(Parser *parser, int *node_count) {
    int capacity = 256;
    int count    = 0;
    ASTNode *nodes = (ASTNode *)malloc(capacity * sizeof(ASTNode));
    if (!nodes) return NULL;

    skip_newlines(parser);

    while (parser->current.type != TOKEN_EOF) {
        if (count >= capacity) {
            capacity *= 2;
            nodes = (ASTNode *)realloc(nodes, capacity * sizeof(ASTNode));
        }

        nodes[count++] = parse_line(parser);
        skip_newlines(parser);
    }

    *node_count = count;
    return nodes;
}

/* Imprime un nodo para depuracion */
void ast_node_print(ASTNode *node) {
    const char *type_names[] = { "INSTRUCCION", "DIRECTIVA", "ETIQUETA" };
    printf("Linea %3d | %-12s | etiqueta=%-10s | mnemonico=%s\n",
           node->line,
           type_names[node->type],
           node->label,
           node->mnemonic);
}
