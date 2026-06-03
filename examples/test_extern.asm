; ============================================================
; test_extern.asm
; Prueba de simbolos externos y relocaciones
; ============================================================

SECTION .text
GLOBAL  main
EXTERN  printf
EXTERN  exit

main:
    ; Preparar pila
    PUSH EBP
    MOV  EBP, ESP

    ; Llamar a printf con un argumento
    PUSH mensaje
    CALL printf
    ADD  ESP, 4

    ; Llamar a exit(0)
    PUSH 0
    CALL exit

    ; Por si acaso
    POP  EBP
    RET

SECTION .data

mensaje:
    DB 72   ; H
    DB 111  ; o
    DB 108  ; l
    DB 97   ; a
    DB 10   ; \n
    DB 0    ; \0
