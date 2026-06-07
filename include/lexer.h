#ifndef LEXER_H
#define LEXER_H

typedef enum {
    TOKEN_IDENTIFIER,
    TOKEN_NUMBER,
    TOKEN_REGISTER,
    TOKEN_INSTRUCTION,
    TOKEN_DIRECTIVE,
    TOKEN_COMMA,
    TOKEN_COLON,
    TOKEN_LBRACKET,
    TOKEN_RBRACKET,
    TOKEN_PLUS,
    TOKEN_MINUS,
    TOKEN_STAR,
    TOKEN_NEWLINE,
    TOKEN_EOF,
    TOKEN_UNKNOWN
} TokenType;

typedef struct {
    TokenType type;
    char lexeme[64];
    int line;
} Token;

typedef struct {
    char *source;
    int pos;
    int line;
    int length;
} Lexer;

Lexer *lexer_create(const char *filename);
void   lexer_destroy(Lexer *lexer);
Token  lexer_next_token(Lexer *lexer);
void   token_print(Token token);

#endif /* LEXER_H */
