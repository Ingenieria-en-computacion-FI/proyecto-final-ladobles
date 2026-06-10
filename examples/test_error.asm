; ----------------------------------------------------
; test_erro.asm : Version con errores para pruebas
; ----------------------------------------------------

SECTION .text
GLOBAL main

main:
    MOV EAX, 10
    MOV EBX, 4

    ; ERROR 1: Mnemónico desconocido (MOOV en lugar de MOV)
    MOOV ECX, 5

    ADD EAX, EBX
    SUB EBX, 1

    CMP EAX, 20
    JE ETQ_IGUAL
    JNE ETQ_NOIGUAL

ETQ_IGUAL:
    OR EAX, EBX
    JMP CONT1

ETQ_NOIGUAL:
    XOR EBX, EBX

CONT1:
    MOV [EBX], EAX
    MOV ECX, [EBX+4]

    ; ERROR 2: Registro inválido (EBX7 no existe)
    MOV EBX7, 0

    MOV ESI, 2
    LEA ECX, [EBX+ESI*4+32]
    MOVZX EDX, AL

    PUSH EAX
    PUSH EBX
    CALL SUBRUTINA
    POP EBX
    POP EAX

    MOV ECX, 5

LOOP_INI:
    DEC ECX
    CMP ECX, 0
    JG LOOP_CONT
    JMP LOOP_FIN

LOOP_CONT:
    INC EAX
    ; ERROR 3: Etiqueta no definida
    JMP ETIQUETA_FALSA

LOOP_FIN:
    TEST EAX, EBX
    CMP EAX, 0
    JE FIN
    JMP SALTO_TEST

SUBRUTINA:
    PUSH EBP
    MOV EBP, ESP
    MOV EAX, 66
    POP EBP
    RET

SALTO_TEST:
    XCHG ECX, EDX
    JMP FIN

FIN:
    JMP FIN

SECTION .data
DATA1:
    DD 12345678
DATA2:
    DD 100
