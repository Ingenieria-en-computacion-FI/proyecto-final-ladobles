; ============================================================
; test_mov.asm
; Prueba basica de instrucciones MOV e inmediatos
; ============================================================

SECTION .text
GLOBAL main

main:
    ; MOV inmediato
    MOV EAX, 1
    MOV EBX, 2
    MOV ECX, 3
    MOV EDX, 4

    ; MOV registro a registro
    MOV EAX, EBX
    MOV ECX, EDX

    ; MOV memoria directa
    MOV EAX, [1000]

    ; MOV base + desplazamiento
    MOV EAX, [EBP+4]
    MOV EBX, [EBP+8]

    ; MOV base + indice
    MOV EAX, [EBX+ECX]

    ; MOV base + indice escalado
    MOV EAX, [EBX+ECX*4]

    ; MOV base + indice escalado + desplazamiento (SIB completo)
    MOV EAX, [EBX+ECX*4+8]

    ; RET
    RET
