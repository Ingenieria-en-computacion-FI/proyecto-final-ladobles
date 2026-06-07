SECTION .text
GLOBAL _start

_start:
    MOV AL, 255
    MOV BL, 10
    MOV CL, 5
    MOVZX EAX, AL
    MOVZX EBX, BL
    MOVZX ECX, CL
    RET
