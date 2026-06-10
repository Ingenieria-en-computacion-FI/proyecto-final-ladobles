; ----------------------------------------------------
; test_error.asm : Ejemplo con errores de entrada
; test con errores intencionados
; para probar el manejo de errores del ensamblador/traductor
; ----------------------------------------------------

; --------------- Datos -----------------
DATA1:
    DD 12345678
DATA2:
    DD 100

; --------------- Programa ---------------
INICIO:
    ; ERROR 1: Mnemónico desconocido (MOOV en lugar de MOV)
    MOOV EAX, 10

    ; ERROR 2: Registro inválido (EBX5 no existe)
    MOV EBX5, 4

    ; ERROR 3: Operandos incompatibles de tamaño (EAX 32-bit vs BL 8-bit)
    ADD EAX, BL

    ; ERROR 4: Operando faltante (ADD necesita dos operandos)
    ADD EAX

    CMP EAX, 20
    JE ETQ_IGUAL
    JNE ETQ_NOIGUAL

ETQ_IGUAL:
    ; ERROR 5: Instrucción OR con demasiados operandos
    OR EAX, EBX, ECX

    JMP CONT1

ETQ_NOIGUAL:
    XOR EBX, EBX

CONT1:
    ; ERROR 6: Memoria a memoria no permitida (dos operandos memoria)
    MOV [EAX], [EBX]

    ; ERROR 7: Modo de direccionamiento inválido (ESP no puede ser índice en SIB)
    MOV ESI, 2
    MOV EAX, [EBX+ESP*4+16]

    ; ERROR 8: Valor inmediato inválido (fuera de rango para 32 bits)
    MOV ECX, 9999999999

    ; ERROR 9: Valor hexadecimal malformado (dígito G no válido en hex)
    MOV EDX, 0xABCG

    ; Instrucciones válidas intercaladas
    LEA ECX, [EBX+ESI*4+32]
    MOVZX EDX, AL
    XCHG EAX, EBX

    ; PUSH / POP / CALL / RET
    PUSH EAX
    ; ERROR 10: PUSH sin operando
    PUSH
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
    ; ERROR 11: JMP a etiqueta no definida (LOOP_VACIO no existe en este archivo)
    JMP LOOP_VACIO

LOOP_FIN:
    TEST EAX, EBX
