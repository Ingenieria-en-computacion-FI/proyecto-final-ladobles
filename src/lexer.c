#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "../include/lexer.h"

/* Registros soportados */
static const char *registers[] = {
    "EAX", "EBX", "ECX", "EDX",
    "ESI", "EDI", "EBP", "ESP",
    NULL
};

/* Instrucciones soportadas */
static const char *instructions[] = {
    "MOV", "PUSH", "POP", "LEA",
    "ADD", "SUB", "INC", "DEC", "CMP", "NEG", "MUL", "DIV", "IMUL", "IDIV",
    "AND", "OR", "XOR", "NOT",
    "JMP", "JE", "JNE", "JG", "JL", "JGE", "JLE",
    "CALL", "RET", "NOP", "INT",
    NULL
};

/* Directivas soportadas */
static const char *directives[] = {
    "SECTION", "GLOBAL", "EXTERN",
    "DB", "DW", "DD",
    "RESB", "RESW", "RESD",
    "ORG", "EQU",
    NULL
};

/* Verifica si una palabra esta en una lista */
static int in_list(const char *word, const char **list) {
    for (int i = 0; list[i] != NULL; i++) {
        if (strcmp(word, list[i]) == 0)
            return 1;
    }
    return 0;
}

/* Lee el contenido completo de un archivo */
static char *read_file(const char *filename) {
    FILE *file = fopen(filename, "r");
    if (!file) {
        fprintf(stderr, "Error: no se pudo abrir el archivo %s\n", filename);
        return NULL;
    }

    fseek(file, 0, SEEK_END);
    long size = ftell(file);
    rewind(file);

    char *buffer = (char *)malloc(size + 1);
    if (!buffer) {
        fprintf(stderr, "Error: memoria insuficiente\n");
        fclose(file);
        return NULL;
    }

    fread(buffer, 1, size, file);
    buffer[size] = '\0';
    fclose(file);
    return buffer;
}

/* Crea un nuevo lexer a partir de un archivo */
Lexer *lexer_create(const char *filename) {
    Lexer *lexer = (Lexer *)malloc(sizeof(Lexer));
    if (!lexer) return NULL;

    lexer->source = read_file(filename);
    if (!lexer->source) {
        free(lexer);
        return NULL;
    }

    lexer->pos    = 0;
    lexer->line   = 1;
    lexer->length = strlen(lexer->source);
    return lexer;
}

/* Libera la memoria del lexer */
void lexer_destroy(Lexer *lexer) {
    if (lexer) {
        free(lexer->source);
        free(lexer);
    }
}

/* Salta espacios y tabuladores */
static void skip_whitespace(Lexer *lexer) {
    while (lexer->pos < lexer->length) {
        char c = lexer->source[lexer->pos];
        if (c == ' ' || c == '\t')
            lexer->pos++;
        else
            break;
    }
}

/* Salta comentarios (desde ; hasta fin de linea) */
static void skip_comment(Lexer *lexer) {
    while (lexer->pos < lexer->length &&
           lexer->source[lexer->pos] != '\n') {
        lexer->pos++;
    }
}

/* Obtiene el siguiente token del source */
Token lexer_next_token(Lexer *lexer) {
    Token token;
    token.type = TOKEN_UNKNOWN;
    token.lexeme[0] = '\0';
    token.line = lexer->line;

    skip_whitespace(lexer);

    /* Fin de archivo */
    if (lexer->pos >= lexer->length) {
        token.type = TOKEN_EOF;
        strcpy(token.lexeme, "EOF");
        return token;
    }

    char c = lexer->source[lexer->pos];

    /* Comentario */
    if (c == ';') {
        skip_comment(lexer);
        return lexer_next_token(lexer);
    }

    /* Salto de linea */
    if (c == '\n') {
        token.type = TOKEN_NEWLINE;
        strcpy(token.lexeme, "\\n");
        lexer->pos++;
        lexer->line++;
        return token;
    }

    /* Caracteres simples */
    if (c == ',') { token.type = TOKEN_COMMA;    token.lexeme[0] = c; token.lexeme[1] = '\0'; lexer->pos++; return token; }
    if (c == ':') { token.type = TOKEN_COLON;    token.lexeme[0] = c; token.lexeme[1] = '\0'; lexer->pos++; return token; }
    if (c == '[') { token.type = TOKEN_LBRACKET; token.lexeme[0] = c; token.lexeme[1] = '\0'; lexer->pos++; return token; }
    if (c == ']') { token.type = TOKEN_RBRACKET; token.lexeme[0] = c; token.lexeme[1] = '\0'; lexer->pos++; return token; }
    if (c == '+') { token.type = TOKEN_PLUS;     token.lexeme[0] = c; token.lexeme[1] = '\0'; lexer->pos++; return token; }
    if (c == '-') { token.type = TOKEN_MINUS;    token.lexeme[0] = c; token.lexeme[1] = '\0'; lexer->pos++; return token; }
    if (c == '*') { token.type = TOKEN_STAR;     token.lexeme[0] = c; token.lexeme[1] = '\0'; lexer->pos++; return token; }

    /* Numero decimal o hexadecimal */
    if (isdigit(c)) {
        int i = 0;
        /* Hexadecimal 0x... */
        if (c == '0' && lexer->pos + 1 < lexer->length &&
            (lexer->source[lexer->pos + 1] == 'x' ||
             lexer->source[lexer->pos + 1] == 'X')) {
            token.lexeme[i++] = lexer->source[lexer->pos++]; /* 0 */
            token.lexeme[i++] = lexer->source[lexer->pos++]; /* x */
            while (lexer->pos < lexer->length &&
                   isxdigit(lexer->source[lexer->pos])) {
                token.lexeme[i++] = lexer->source[lexer->pos++];
            }
        } else {
            while (lexer->pos < lexer->length &&
                   isdigit(lexer->source[lexer->pos])) {
                token.lexeme[i++] = lexer->source[lexer->pos++];
            }
        }
        token.lexeme[i] = '\0';
        token.type = TOKEN_NUMBER;
        return token;
    }

    /* Palabra: registro, instruccion, directiva o identificador */
    if (isalpha(c) || c == '_' || c == '.') {
        int i = 0;
        while (lexer->pos < lexer->length &&
               (isalnum(lexer->source[lexer->pos]) ||
                lexer->source[lexer->pos] == '_' ||
                lexer->source[lexer->pos] == '.')) {
            token.lexeme[i++] = toupper(lexer->source[lexer->pos++]);
        }
        token.lexeme[i] = '\0';

        if (in_list(token.lexeme, registers))
            token.type = TOKEN_REGISTER;
        else if (in_list(token.lexeme, instructions))
            token.type = TOKEN_INSTRUCTION;
        else if (in_list(token.lexeme, directives))
            token.type = TOKEN_DIRECTIVE;
        else
            token.type = TOKEN_IDENTIFIER;

        return token;
    }

    /* Caracter desconocido */
    token.type = TOKEN_UNKNOWN;
    token.lexeme[0] = c;
    token.lexeme[1] = '\0';
    lexer->pos++;
    return token;
}

/* Imprime un token en pantalla (util para depuracion) */
void token_print(Token token) {
    const char *type_names[] = {
        "IDENTIFIER", "NUMBER", "REGISTER", "INSTRUCTION",
        "DIRECTIVE", "COMMA", "COLON", "LBRACKET", "RBRACKET",
        "PLUS", "MINUS", "STAR", "NEWLINE", "EOF", "UNKNOWN"
    };
    printf("Linea %3d | %-12s | %s\n",
           token.line,
           type_names[token.type],
           token.lexeme);
}
