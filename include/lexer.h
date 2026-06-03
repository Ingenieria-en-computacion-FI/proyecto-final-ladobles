#ifndef LEXER_H
#define LEXER_H

/* Tipos de tokens reconocidos por el lexer */
typedef enum {
    TOKEN_IDENTIFIER,   /* nombres de etiquetas o simbolos */
    TOKEN_NUMBER,       /* numeros decimales o hexadecimales */
    TOKEN_REGISTER,     /* EAX, EBX, ECX, EDX, ESI, EDI, EBP, ESP */
    TOKEN_INSTRUCTION,  /* MOV, ADD, SUB, JMP, etc */
    TOKEN_DIRECTIVE,    /* SECTION, GLOBAL, EXTERN, DB, etc */
    TOKEN_COMMA,        /* , */
    TOKEN_COLON,        /* : */
    TOKEN_LBRACKET,     /* [ */
    TOKEN_RBRACKET,     /* ] */
    TOKEN_PLUS,         /* + */
    TOKEN_MINUS,        /* - */
    TOKEN_STAR,         /* * */
    TOKEN_NEWLINE,      /* salto de linea */
    TOKEN_EOF,          /* fin de archivo */
    TOKEN_UNKNOWN       /* token no reconocido */
} TokenType;

/* Estructura que representa un token */
typedef struct {
    TokenType type;     /* tipo del token */
    char lexeme[64];    /* texto original del token */
    int line;           /* numero de linea donde aparece */
} Token;

/* Estructura que representa el estado del lexer */
typedef struct {
    char *source;       /* contenido completo del archivo */
    int pos;            /* posicion actual en el source */
    int line;           /* linea actual */
    int length;         /* longitud total del source */
} Lexer;

/* Funciones del lexer */
Lexer *lexer_create(const char *filename);
void   lexer_destroy(Lexer *lexer);
Token  lexer_next_token(Lexer *lexer);
void   token_print(Token token);

#endif /* LEXER_H */
