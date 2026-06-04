# Reporte Técnico — IA-32 Assembler & Mini Linker

## Universidad Nacional Autónoma de México
### Estructura y Programación de Computadoras 2026-2
### Ing. Adara Mercado Martínez

---

## 1. Introducción

Este proyecto implementa un sistema completo de traducción y construcción
de programas para un subconjunto de la arquitectura IA-32. El sistema
incluye un ensamblador de una pasada, un ensamblador de dos pasadas,
un generador de archivos objeto y un mini linker, todos implementados
manualmente en C sin el uso de herramientas automáticas de parsing.

El objetivo principal fue comprender internamente cómo funcionan los
ensambladores, los archivos objeto, los enlazadores y la generación
de código máquina, implementando cada componente desde cero.

---

## 2. Arquitectura del Sistema

El sistema está compuesto por los siguientes módulos que forman
un pipeline de traducción:

    ASM
     ↓
    Lexer        — tokenización del código fuente
     ↓
    Parser       — construcción del AST
     ↓
    Assembler    — ensamblado de una o dos pasadas
     ↓
    Encoder      — generación de código máquina IA-32
     ↓
    Object       — escritura del archivo objeto
     ↓
    Linker       — enlazado y generación del binario final

---

## 3. Módulos Implementados

### 3.1 Lexer (`src/lexer.c`)

El lexer lee el archivo fuente carácter por carácter y produce
una lista de tokens. Reconoce los siguientes tipos:

- Registros: EAX, EBX, ECX, EDX, ESI, EDI, EBP, ESP
- Instrucciones: MOV, ADD, SUB, JMP, CALL, RET, IMUL, IDIV, etc.
- Directivas: SECTION, GLOBAL, EXTERN, DB, DW, DD, RESB, RESW, RESD, ORG, EQU
- Números decimales y hexadecimales (prefijo 0x)
- Identificadores y etiquetas
- Operadores: +, -, *, ',', [, ], :
- Comentarios: ignorados desde ; hasta fin de línea

### 3.2 Parser (`src/parser.c`)

El parser toma los tokens del lexer y construye una lista de
nodos AST. Cada nodo representa una instrucción, directiva
o etiqueta con sus operandos.

Modos de direccionamiento soportados:

- Inmediato: MOV EAX, 10
- Registro a registro: MOV EAX, EBX
- Memoria directa: MOV EAX, [1000]
- Base + desplazamiento: MOV EAX, [EBP+4]
- Base + índice: MOV EAX, [EBX+ECX]
- Base + índice escalado: MOV EAX, [EBX+ECX*4]
- Base + índice escalado + desplazamiento: MOV EAX, [EBX+ECX*4+8]

Durante las pruebas se detectaron y corrigieron dos bugs:

- Loop infinito al parsear identificadores como .TEXT en directivas
- Fallo al parsear referencias simbólicas dentro de corchetes como [resultado_suma]

### 3.3 Tabla de Símbolos (`src/symbols.c`)

Diccionario que asocia nombres con direcciones. Soporta:

- Símbolos locales, globales y externos
- Detección de redefiniciones
- Símbolos pendientes de definición
- Crecimiento dinámico del arreglo interno

### 3.4 Ensamblador (`src/assembler.c`)

Implementa dos estrategias de ensamblado:

**Una pasada:** lee el archivo una sola vez generando código
parcial. Las referencias adelantadas se resuelven mediante
fixups al final de la pasada. Cuando se encuentra una referencia
a un símbolo no definido, se emite un placeholder de 4 bytes
y se registra un fixup que indica dónde parchear cuando el
símbolo sea resuelto.

**Dos pasadas:** primera pasada construye la tabla de símbolos
completa calculando todos los offsets, segunda pasada genera
el código definitivo sin necesidad de fixups ya que todos los
símbolos están disponibles desde el inicio.

### 3.5 Encoder IA-32 (`src/encoder.c`)

Genera los bytes de código máquina para cada instrucción.
Implementa:

- Construcción del byte ModRM: [mod(2)][reg(3)][rm(3)]
- Construcción del byte SIB: [escala(2)][índice(3)][base(3)]
- Escalas soportadas: 1, 2, 4, 8
- Codificación de desplazamientos de 8 y 32 bits
- Codificación de inmediatos de 32 bits en little endian
- Detección automática de cuándo se necesita byte SIB

La correctitud del encoder fue validada comparando la salida
byte a byte contra NASM para instrucciones simples, obteniendo
resultados idénticos.

### 3.6 Formato Objeto (`src/object.c`)

Formato objeto propio identificado con la firma OBJ. Estructura:

    [ Encabezado   ]  — firma, versión, conteos y offsets
    [ Secciones    ]  — tabla de .text, .data, .bss
    [ Símbolos     ]  — tabla de símbolos con flags global/extern
    [ Relocaciones ]  — tabla de parches pendientes (R32 y PC32)
    [ Bytes .text  ]  — código máquina generado
    [ Bytes .data  ]  — datos inicializados

El encabezado actúa como índice permitiendo acceso directo
a cualquier sección sin lectura secuencial.

### 3.7 Mini Linker (`src/linker.c`)

Proceso de enlazado en 5 pasos:

1. **Carga** — lee todos los archivos objeto del disco
2. **Fusión** — concatena secciones .text, .data y .bss de todos los objetos
3. **Tabla global** — construye diccionario de símbolos globales con offsets finales
4. **Verificación** — confirma que todos los símbolos externos están resueltos
5. **Relocaciones** — aplica los parches R32 y PC32 en el binario fusionado

---

## 4. Instrucciones de Compilación

    make        # compilar
    make clean  # limpiar
    make test   # ejecutar prueba básica

---

## 5. Instrucciones de Uso

    # Ensamblar (genera archivo objeto)
    ./assembler -a archivo.asm -o archivo.o

    # Ensamblar con una pasada
    ./assembler -1 -a archivo.asm -o archivo.o

    # Linkear uno o varios objetos
    ./assembler -l modulo_a.o modulo_b.o -o binario

    # Ensamblar y linkear en un paso
    ./assembler -al archivo.asm -o binario

    # Modo verbose (muestra tokens, AST y código generado)
    ./assembler -v -al archivo.asm -o binario

    # Imprimir contenido de archivo objeto
    ./assembler -p archivo.o

---

## 6. Casos de Prueba

| Archivo | Descripción | Resultado |
|---|---|---|
| test_mov.asm | Todos los modos de direccionamiento MOV | ✅ PASS |
| test_jumps.asm | Saltos y referencias adelantadas | ✅ PASS |
| test_call.asm | CALL, RET y manejo de pila | ✅ PASS |
| test_extern.asm | Símbolos externos y relocaciones | ✅ PASS |
| test_sib.asm | Byte SIB con todas las escalas | ✅ PASS |
| test_multimodule_a.asm | Módulo A: define y exporta funciones | ✅ PASS |
| test_multimodule_b.asm | Módulo B: usa funciones externas | ✅ PASS |
| Comparación con NASM | Validación byte a byte | ✅ Idénticos |
| Compilación limpia | Sin warnings con -Wall -Wextra | ✅ 0 warnings |

---

## 7. Organización del Equipo

| Integrante | Módulo principal |
|---|---|
| Integrante 1 | Lexer y tokens |
| Integrante 2 | Parser y representación interna |
| Integrante 3 | Ensamblador de una y dos pasadas |
| Integrante 4 | Encoder IA-32, ModRM y SIB |

---

## 8. Dificultades encontradas

Durante el desarrollo se encontraron las siguientes dificultades:

**Parsing de directivas con identificadores**
La directiva SECTION .text fallaba porque .TEXT era tokenizado
como identificador y el parser no tenía caso para ese tipo en
parse_operand, causando un loop infinito. Se resolvió agregando
el caso faltante con avance forzado del token.

**Direccionamiento simbólico en memoria**
Instrucciones como MOV [resultado_suma], EAX fallaban porque
parse_memory solo aceptaba registros o números como contenido
de corchetes. Se resolvió agregando soporte para identificadores
como direcciones de memoria.

**Validación contra NASM**
La comparación directa contra NASM con archivos que usan SECTION
mostró diferencias porque NASM genera encabezados de sección
adicionales en formato bin. La solución fue comparar instrucciones
aisladas sin directivas de sección, confirmando que los opcodes
generados son correctos.

---

## 9. Conclusiones

El proyecto logró implementar exitosamente un sistema completo de
ensamblado y enlazado para un subconjunto de la arquitectura IA-32.

Los objetivos alcanzados fueron:

- Implementación de un lexer y parser manuales sin herramientas automáticas
- Ensamblador funcional de una y dos pasadas con manejo de fixups
- Encoder IA-32 con soporte completo para ModRM y SIB
- Formato objeto propio con secciones, símbolos y relocaciones
- Mini linker capaz de enlazar múltiples módulos
- Validación de correctitud byte a byte contra NASM

El desarrollo del proyecto permitió comprender en profundidad
cómo funciona internamente la traducción de código ensamblador
a código máquina, el rol de los archivos objeto como unidades
de compilación independientes, y el proceso de enlazado que
resuelve referencias entre módulos y genera el binario final.

---

## 10. Referencias

- Intel 64 and IA-32 Architectures Software Developer's Manual, Volume 2
- NASM — Netwide Assembler Documentation
- Linkers and Loaders — John R. Levine, 1999
