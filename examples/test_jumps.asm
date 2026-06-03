; ============================================================
; test_jumps.asm
; Prueba de saltos y referencias adelantadas
; ============================================================

SECTION .text
GLOBAL main

main:
    MOV EAX, 0
    MOV EBX, 10

loop:
    CMP EAX, EBX
    JGE fin

    INC EAX
    JMP loop

fin:
    MOV EAX, 1
    RET
