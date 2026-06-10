; ----------------------------------------------------
; test_error.asm : Version con errores para pruebas
; ----------------------------------------------------

SECTION .text
GLOBAL main

main:
    MOV EAX, 10
    JMP etiqueta_que_no_existe    ; ERROR: etiqueta no definida
    MOVXX EBX, 20                 ; ERROR: instrucción inválida
    MOV EAX, @5                   ; ERROR: carácter @ no válido
    RET
