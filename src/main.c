#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../include/lexer.h"
#include "../include/parser.h"
#include "../include/assembler.h"
#include "../include/object.h"
#include "../include/linker.h"

/* ============================================================
   USO DEL PROGRAMA
   ============================================================ */

static void print_usage(const char *prog) {
    printf("Uso:\n");
    printf("  %s -a <archivo.asm> -o <salida.o>        Ensamblar\n", prog);
    printf("  %s -l <obj1.o> [obj2.o ...] -o <salida>  Linkear\n",  prog);
    printf("  %s -al <archivo.asm> -o <salida>          Ensamblar y linkear\n", prog);
    printf("  %s -p <archivo.o>                         Imprimir objeto\n", prog);
    printf("\nOpciones:\n");
    printf("  -1    Usar ensamblador de una pasada (por defecto: dos pasadas)\n");
    printf("  -v    Modo verbose (imprime tokens, AST y codigo)\n");
}

/* ============================================================
   ENSAMBLADO
   ============================================================ */

static int assemble(const char *input, const char *output,
                    int one_pass, int verbose) {
    /* Lexer */
    Lexer *lexer = lexer_create(input);
    if (!lexer) return -1;

    if (verbose) {
        printf("\n=== TOKENS ===\n");
        Lexer *tmp = lexer_create(input);
        Token  tok;
        while ((tok = lexer_next_token(tmp)).type != TOKEN_EOF)
            token_print(tok);
        lexer_destroy(tmp);
    }

    /* Parser */
    Parser *parser = parser_create(lexer);
    if (!parser) {
        lexer_destroy(lexer);
        return -1;
    }

    int node_count = 0;
    ASTNode *nodes = parser_parse(parser, &node_count);

    if (verbose) {
        printf("\n=== AST ===\n");
        for (int i = 0; i < node_count; i++)
            ast_node_print(&nodes[i]);
    }

    if (parser->error_count > 0) {
        fprintf(stderr, "Errores de parsing: %d\n", parser->error_count);
        free(nodes);
        parser_destroy(parser);
        lexer_destroy(lexer);
        return -1;
    }

    /* Ensamblador */
    Assembler *assembler = assembler_create();
    if (!assembler) {
        free(nodes);
        parser_destroy(parser);
        lexer_destroy(lexer);
        return -1;
    }

    int result;
    if (one_pass) {
        printf("Usando ensamblador de una pasada...\n");
        result = assembler_one_pass(assembler, nodes, node_count);
    } else {
        printf("Usando ensamblador de dos pasadas...\n");
        result = assembler_two_pass(assembler, nodes, node_count);
    }

    if (verbose) {
        assembler_print_code(assembler);
        symbol_table_print(assembler->symbols);
    }

    if (result < 0) {
        fprintf(stderr, "Errores de ensamblado: %d\n",
                assembler->error_count);
        assembler_destroy(assembler);
        free(nodes);
        parser_destroy(parser);
        lexer_destroy(lexer);
        return -1;
    }

    /* Escribir archivo objeto */
    result = obj_write(output, assembler);

    assembler_destroy(assembler);
    free(nodes);
    parser_destroy(parser);
    lexer_destroy(lexer);
    return result;
}

/* ============================================================
   ENLAZADO
   ============================================================ */

static int link_objects(char **inputs, int count,
                         const char *output, int verbose) {
    Linker *linker = linker_create();
    if (!linker) return -1;

    for (int i = 0; i < count; i++) {
        if (linker_add_object(linker, inputs[i]) < 0) {
            linker_destroy(linker);
            return -1;
        }
    }

    if (linker_link(linker) < 0) {
        fprintf(stderr, "Errores de enlazado: %d\n", linker->error_count);
        linker_destroy(linker);
        return -1;
    }

    if (verbose)
        linker_print(linker);

    int result = linker_write_binary(linker, output);
    linker_destroy(linker);
    return result;
}

/* ============================================================
   MAIN
   ============================================================ */

int main(int argc, char *argv[]) {
    if (argc < 2) {
        print_usage(argv[0]);
        return 1;
    }

    int   one_pass  = 0;
    int   verbose   = 0;
    char *mode      = NULL;
    char *output    = NULL;
    char *inputs[32];
    int   input_count = 0;

    /* Parsear argumentos */
    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "-1")  == 0) { one_pass = 1; }
        else if (strcmp(argv[i], "-v")  == 0) { verbose  = 1; }
        else if (strcmp(argv[i], "-a")  == 0) { mode = "assemble"; }
        else if (strcmp(argv[i], "-l")  == 0) { mode = "link"; }
        else if (strcmp(argv[i], "-al") == 0) { mode = "both"; }
        else if (strcmp(argv[i], "-p")  == 0) { mode = "print"; }
        else if (strcmp(argv[i], "-o")  == 0) {
            if (i + 1 < argc) output = argv[++i];
        }
        else {
            if (input_count < 32)
                inputs[input_count++] = argv[i];
        }
    }

    if (!mode) {
        fprintf(stderr, "Error: debe especificar un modo (-a, -l, -al, -p)\n");
        print_usage(argv[0]);
        return 1;
    }

    /* Modo: solo ensamblar */
    if (strcmp(mode, "assemble") == 0) {
        if (input_count == 0 || !output) {
            fprintf(stderr, "Error: se requiere archivo de entrada y salida\n");
            return 1;
        }
        return assemble(inputs[0], output, one_pass, verbose) == 0 ? 0 : 1;
    }

    /* Modo: solo linkear */
    if (strcmp(mode, "link") == 0) {
        if (input_count == 0 || !output) {
            fprintf(stderr, "Error: se requiere al menos un objeto y salida\n");
            return 1;
        }
        return link_objects(inputs, input_count, output, verbose) == 0 ? 0 : 1;
    }

    /* Modo: ensamblar y linkear */
    if (strcmp(mode, "both") == 0) {
        if (input_count == 0 || !output) {
            fprintf(stderr, "Error: se requiere archivo de entrada y salida\n");
            return 1;
        }
        char obj_tmp[256];
        snprintf(obj_tmp, sizeof(obj_tmp), "%s.o", inputs[0]);

        if (assemble(inputs[0], obj_tmp, one_pass, verbose) < 0)
            return 1;

        char *obj_list[] = { obj_tmp };
        return link_objects(obj_list, 1, output, verbose) == 0 ? 0 : 1;
    }

    /* Modo: imprimir archivo objeto */
    if (strcmp(mode, "print") == 0) {
        if (input_count == 0) {
            fprintf(stderr, "Error: se requiere archivo objeto\n");
            return 1;
        }
        ObjectFile *obj = obj_read(inputs[0]);
        if (!obj) return 1;
        obj_print(obj);
        obj_destroy(obj);
        return 0;
    }

    print_usage(argv[0]);
    return 1;
}
