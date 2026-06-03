; ============================================================
; test_multimodule_b.asm
; Modulo B: usa las funciones definidas en el modulo A
; ============================================================

SECTION .text
GLOBAL  main
EXTERN  suma
EXTERN  resta
EXTERN  multiplica

main:
    ; Preparar pila
    PUSH EBP
    MOV  EBP, ESP

    ; Llamar suma(10, 20)
    PUSH 20
    PUSH 10
    CALL suma
    ADD  ESP, 8
    MOV  [resultado_suma], EAX

    ; Llamar resta(30, 15)
    PUSH 15
    PUSH 30
    CALL resta
    ADD  ESP, 8
    MOV  [resultado_resta], EAX

    ; Llamar multiplica(5, 6)
    PUSH 6
    PUSH 5
    CALL multiplica
    ADD  ESP, 8
    MOV  [resultado_mult], EAX

    ; Restaurar pila y retornar
    POP  EBP
    RET

SECTION .data

resultado_suma:
    DD 0

resultado_resta:
    DD 0

resultado_mult:
    DD 0
