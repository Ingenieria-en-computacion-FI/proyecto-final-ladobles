; ============================================================
; test_sib.asm
; Prueba especifica del byte SIB con todas las escalas
; ============================================================

SECTION .text
GLOBAL main

main:
    ; Preparar registros base e indice
    MOV EBX, 100
    MOV ECX, 2

    ; SIB escala 1: [EBX + ECX*1]
    MOV EAX, [EBX+ECX*1]

    ; SIB escala 2: [EBX + ECX*2]
    MOV EAX, [EBX+ECX*2]

    ; SIB escala 4: [EBX + ECX*4]
    MOV EAX, [EBX+ECX*4]

    ; SIB escala 8: [EBX + ECX*8]
    MOV EAX, [EBX+ECX*8]

    ; SIB con desplazamiento de 8 bits
    MOV EAX, [EBX+ECX*4+8]

    ; SIB con desplazamiento de 32 bits
    MOV EAX, [EBX+ECX*4+1000]

    ; SIB con diferentes registros
    MOV EAX, [EDI+ESI*4+16]
    MOV EBX, [EBP+EDX*2+8]

    RET
