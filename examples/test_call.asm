; ============================================================
; test_call.asm
; Prueba de CALL, RET y manejo de pila
; ============================================================

SECTION .text
GLOBAL main

main:
    ; Preparar pila
    PUSH EBP
    MOV  EBP, ESP

    ; Llamar a funcion
    PUSH 10
    PUSH 20
    CALL suma
    ADD  ESP, 8

    ; Restaurar pila
    POP EBP
    RET

; ============================================================
; suma: retorna EAX = primer argumento + segundo argumento
; ============================================================
suma:
    PUSH EBP
    MOV  EBP, ESP

    MOV EAX, [EBP+8]    ; primer argumento
    MOV EBX, [EBP+12]   ; segundo argumento
    ADD EAX, EBX        ; EAX = EAX + EBX

    POP EBP
    RET
