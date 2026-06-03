; ============================================================
; test_multimodule_a.asm
; Modulo A: define funciones que usa el modulo B
; ============================================================

SECTION .text
GLOBAL  suma
GLOBAL  resta
GLOBAL  multiplica

; ============================================================
; suma: EAX = primer argumento + segundo argumento
; ============================================================
suma:
    PUSH EBP
    MOV  EBP, ESP

    MOV EAX, [EBP+8]    ; primer argumento
    MOV EBX, [EBP+12]   ; segundo argumento
    ADD EAX, EBX

    POP EBP
    RET

; ============================================================
; resta: EAX = primer argumento - segundo argumento
; ============================================================
resta:
    PUSH EBP
    MOV  EBP, ESP

    MOV EAX, [EBP+8]    ; primer argumento
    MOV EBX, [EBP+12]   ; segundo argumento
    SUB EAX, EBX

    POP EBP
    RET

; ============================================================
; multiplica: EAX = primer argumento * segundo argumento
; ============================================================
multiplica:
    PUSH EBP
    MOV  EBP, ESP

    MOV EAX, [EBP+8]    ; primer argumento
    MOV EBX, [EBP+12]   ; segundo argumento
    IMUL EBX

    POP EBP
    RET
