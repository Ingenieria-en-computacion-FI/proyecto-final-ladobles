#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../include/parser.h"

Parser *parser_create(Lexer *lexer) {
    Parser *parser = (Parser *)malloc(sizeof(Parser));
    if (!parser) return NULL;
    parser->lexer       = lexer;
    parser->error_count = 0;
    parser->current     = lexer_next_token(lexer);
    parser->next        = lexer_next_token(lexer);
    return parser;
}

void parser_destroy(Parser *parser) {
    if (parser) free(parser);
}

static void advance(Parser *parser) {
    parser->current = parser->next;
    parser->next    = lexer_next_token(parser->lexer);
}

static void skip_newlines(Parser *parser) {
    while (parser->current.type == TOKEN_NEWLINE)
        advance(parser);
}

static void parse_error(Parser *parser, const char *msg) {
    fprintf(stderr, "Error linea %d: %s (encontrado: '%s')\n",
            parser->current.line, msg, parser->current.lexeme);
    parser->error_count++;
    /* Avanzar al siguiente token para continuar detectando errores */
    advance(parser);
}

static Operand parse_memory(Parser *parser) {
    Operand op;
    memset(&op, 0, sizeof(Operand));
    op.type  = OPERAND_MEMORY;
    op.scale = 1;
    
    if (parser->current.type != TOKEN_LBRACKET) {
        parse_error(parser, "se esperaba '['");
        return op;
    }
    advance(parser); /* consumir [ */

    /* Memoria directa con numero: [1000] */
    if (parser->current.type == TOKEN_NUMBER) {
        op.displacement = (int)strtol(parser->current.lexeme, NULL, 0);
        advance(parser);
    }
    /* Memoria directa con identificador: [resultado_suma] */
    else if (parser->current.type == TOKEN_IDENTIFIER) {
        strncpy(op.index, parser->current.lexeme, 15);
        op.scale = 0; /* escala 0 indica simbolo */
        advance(parser);
    }
    /* Base register */
    else if (parser->current.type == TOKEN_REGISTER) {
        strncpy(op.base, parser->current.lexeme, 15);
        advance(parser);

        if (parser->current.type == TOKEN_PLUS) {
            advance(parser);
            if (parser->current.type == TOKEN_REGISTER) {
                strncpy(op.index, parser->current.lexeme, 15);
                advance(parser);
                if (parser->current.type == TOKEN_STAR) {
                    advance(parser);
                    if (parser->current.type == TOKEN_NUMBER) {
                        op.scale = (int)strtol(parser->current.lexeme, NULL, 0);
                        advance(parser);
                    } else {
                        parse_error(parser, "se esperaba un numero para la escala");
                    }
                    if (parser->current.type == TOKEN_PLUS) {
                        advance(parser);
                        if (parser->current.type == TOKEN_NUMBER) {
                            op.displacement = (int)strtol(parser->current.lexeme, NULL, 0);
                            advance(parser);
                        } else {
                            parse_error(parser, "se esperaba un numero para el desplazamiento");
                        }
                    }
                }
            } else if (parser->current.type == TOKEN_NUMBER) {
                op.displacement = (int)strtol(parser->current.lexeme, NULL, 0);
                advance(parser);
            } else {
                parse_error(parser, "se esperaba registro o numero despues de '+'");
            }
        }
    } else {
        parse_error(parser, "se esperaba numero, identificador o registro");
    }

    if (parser->current.type != TOKEN_RBRACKET) {
        parse_error(parser, "se esperaba ']'");
    } else {
        advance(parser);
    }
    return op;
}

static Operand parse_operand(Parser *parser) {
    Operand op;
    memset(&op, 0, sizeof(Operand));
    op.scale = 1;
    
    if (parser->current.type == TOKEN_REGISTER) {
        op.type = OPERAND_REGISTER;
        strncpy(op.base, parser->current.lexeme, 15);
        advance(parser);
    } else if (parser->current.type == TOKEN_NUMBER) {
        op.type      = OPERAND_IMMEDIATE;
        op.immediate = (int)strtol(parser->current.lexeme, NULL, 0);
        advance(parser);
    } else if (parser->current.type == TOKEN_LBRACKET) {
        op = parse_memory(parser);
    } else if (parser->current.type == TOKEN_IDENTIFIER) {
        op.type = OPERAND_IMMEDIATE;
        strncpy(op.base, parser->current.lexeme, 15);
        advance(parser);
    } else {
        parse_error(parser, "operando invalido");
        /* Devolver un operando dummy para continuar */
        op.type = OPERAND_IMMEDIATE;
        op.immediate = 0;
    }
    return op;
}

static ASTNode parse_line(Parser *parser) {
    ASTNode node;
    memset(&node, 0, sizeof(ASTNode));
    node.line = parser->current.line;
    
    /* Verificar si es una etiqueta (IDENTIFIER seguido de COLON) */
    if (parser->current.type == TOKEN_IDENTIFIER &&
        parser->next.type == TOKEN_COLON) {
        strncpy(node.label, parser->current.lexeme, 63);
        advance(parser);
        advance(parser);
        node.type = NODE_LABEL;
        if (parser->current.type == TOKEN_NEWLINE ||
            parser->current.type == TOKEN_EOF) {
            return node;
        }
    }
    
    /* Procesar directiva */
    if (parser->current.type == TOKEN_DIRECTIVE) {
        node.type = NODE_DIRECTIVE;
        strncpy(node.mnemonic, parser->current.lexeme, 15);
        advance(parser);
        while (parser->current.type != TOKEN_NEWLINE &&
               parser->current.type != TOKEN_EOF) {
            if (parser->current.type == TOKEN_COMMA) {
                advance(parser);
                continue;
            }
            if (node.operand_count < 3) {
                node.operands[node.operand_count++] = parse_operand(parser);
            } else {
                parse_error(parser, "demasiados operandos para directiva");
                /* Avanzar hasta el final de la linea */
                while (parser->current.type != TOKEN_NEWLINE &&
                       parser->current.type != TOKEN_EOF) {
                    advance(parser);
                }
                break;
            }
        }
        return node;
    }
    
    /* Procesar instruccion */
    if (parser->current.type == TOKEN_INSTRUCTION) {
        node.type = NODE_INSTRUCTION;
        strncpy(node.mnemonic, parser->current.lexeme, 15);
        advance(parser);
        
        /* Consumir operandos */
        while (parser->current.type != TOKEN_NEWLINE &&
               parser->current.type != TOKEN_EOF) {
            if (parser->current.type == TOKEN_COMMA) {
                advance(parser);
                continue;
            }
            if (node.operand_count < 3) {
                node.operands[node.operand_count++] = parse_operand(parser);
            } else {
                parse_error(parser, "demasiados operandos para instruccion");
                /* Avanzar hasta el final de la linea */
                while (parser->current.type != TOKEN_NEWLINE &&
                       parser->current.type != TOKEN_EOF) {
                    advance(parser);
                }
                break;
            }
        }
        return node;
    }
    
    /* Saltar tokens que no nos interesan (como NEWLINE) */
    if (parser->current.type != TOKEN_NEWLINE &&
        parser->current.type != TOKEN_EOF) {
        parse_error(parser, "token inesperado");
    } else {
        advance(parser);
    }
    
    return node;
}

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

void ast_node_print(ASTNode *node) {
    const char *type_names[] = { "INSTRUCCION", "DIRECTIVA", "ETIQUETA" };
    printf("Linea %3d | %-12s | etiqueta=%-10s | mnemonico=%s\n",
           node->line,
           type_names[node->type],
           node->label,
           node->mnemonic);
}
