# Bitácora IA — IA-32 Assembler & Mini Linker

## Universidad Nacional Autónoma de México
### Estructura y Programación de Computadoras 2026-2
### Profesora: Ing. Adara Mercado Martínez
### Alumnos:
### Arcos Garduño Itzcoatl Ayax
### García Aguilar José Iván
### Martínez Chavez Alexis
### Martínez Méndez Cedrik Alexis
### Motta Reyes Emmanuel Alberto

---

## 1. Herramientas IA utilizadas

| Herramienta | Versión | Uso principal |
|---|---|---|
| Claude (Anthropic) | Sonnet 4.6 | Revisión de diseño, generación de código, depuración |

---

## 2. Diseño previo del equipo

Antes de utilizar herramientas de IA, el equipo elaboró un pseudocódigo
completo del sistema dividido en 9 módulos:

1. Estructura general del sistema (main)
2. Lexer y tokenización
3. Parser y representación interna
4. Tabla de símbolos
5. Ensamblador de una pasada con fixups
6. Ensamblador de dos pasadas
7. Encoder IA-32 con ModRM y SIB
8. Formato objeto propio
9. Mini Linker

Este pseudocódigo fue elaborado por el equipo como punto de partida.
Posteriormente se utilizó IA para revisar la arquitectura, identificar
posibles problemas de diseño y sugerir mejoras antes de proceder a la
implementación.

La IA identificó los siguientes aspectos a refinar en el diseño original:
- Separar claramente las responsabilidades entre el encoder y el assembler
- Agregar el campo `defined` a la tabla de símbolos para distinguir
  símbolos pendientes de los ya resueltos
- Incluir soporte explícito para relocaciones R32 y PC32 desde el diseño

---

## 3. Registro de sesiones

### Sesión 1 — Revisión de arquitectura y planificación
**Fecha:** 02/06/2026

**Contexto:** El equipo presentó su pseudocódigo inicial a la IA para
revisión y retroalimentación antes de comenzar la implementación.

**Prompts utilizados:**
- "Revisa este pseudocódigo del sistema ensamblador-linker e identifica
  posibles problemas de diseño o módulos incompletos"
- "El equipo propone esta estructura de archivos, ¿qué ajustes
  recomiendas para cumplir con los requisitos del proyecto?"
- "Explica cómo debe fluir la información entre los módulos desde
  el archivo ASM hasta el binario final"

**Lo que generó la IA:**
- Retroalimentación sobre el diseño propuesto por el equipo
- Sugerencias de refinamiento para la tabla de símbolos y el encoder
- Confirmación del flujo de datos entre módulos

**Modificaciones manuales:**
- El equipo incorporó las sugerencias al diseño final antes de codificar

**Errores encontrados:**
- Ninguno

---

### Sesión 2 — Implementación de la arquitectura base
**Fecha:** 02/06/2026

**Contexto:** Con el diseño validado, el equipo utilizó IA para acelerar
la implementación de los módulos base siguiendo la arquitectura acordada.

**Prompts utilizados:**
- "Basándote en el diseño del equipo, implementa en C el lexer para IA-32
  con soporte para registros de 32 y 8 bits, instrucciones, directivas
  y manejo de comentarios"
- "Implementa el parser siguiendo el diseño del equipo con soporte para
  los 7 modos de direccionamiento IA-32 incluyendo SIB completo"
- "Implementa la tabla de símbolos con soporte para símbolos globales,
  externos y detección de redefiniciones"
- "Implementa el ensamblador de una y dos pasadas con manejo de fixups
  y relocaciones"
- "Implementa el encoder IA-32 con construcción de ModRM y SIB, registros
  de 32 y 8 bits, y soporte para MOVZX"
- "Diseña e implementa el formato objeto acordado por el equipo con
  encabezado, tabla de secciones, símbolos y relocaciones"
- "Implementa el linker con fusión de secciones, resolución de símbolos
  externos y aplicación de relocaciones"
- "Implementa el punto de entrada principal con soporte para modos
  ensamblar, linkear, ensamblar-linkear y salida hexadecimal --hex"

**Lo que generó la IA:**
- Implementación completa de los 8 módulos del sistema en C
- Archivos de cabecera con definición de estructuras y prototipos
- Makefile con reglas de compilación, limpieza y pruebas
- Soporte para registros de 8 bits (AL, AH, BL, BH, CL, CH, DL, DH)
- Instrucción MOVZX para extensión de byte a 32 bits sin signo
- Opción --hex para generación de archivo hexadecimal legible

**Modificaciones manuales:**
- Creación y organización de archivos en el repositorio de GitHub
- El equipo revisó cada módulo contra el diseño original

**Errores encontrados:**
- Error de ubicación de archivo: src/lexer.c fue creado accidentalmente
  dentro de include/src/ en lugar de src/. Solución: corrección manual
  de la ruta desde la interfaz de GitHub

---

### Sesión 3 — Casos de prueba y herramientas auxiliares
**Fecha:** 03/06/2026

**Contexto:** El equipo definió los casos de prueba necesarios y utilizó
IA para implementarlos junto con las herramientas de validación.

**Prompts utilizados:**
- "El equipo definió estas 8 categorías de prueba, implementa los archivos
  ASM correspondientes: MOV inmediato, saltos, referencias adelantadas,
  EXTERN, CALL, relocaciones, múltiples módulos y SIB"
- "Implementa en Python un script de hexdump para inspección de binarios"
- "Implementa un script de comparación byte a byte contra NASM para
  validación de correctitud del encoder"
- "Implementa un script de pruebas automatizadas con reporte de resultados"

**Lo que generó la IA:**
- 8 casos de prueba cubriendo todos los requisitos del proyecto
- 3 scripts Python para inspección, comparación y automatización

**Modificaciones manuales:**
- El equipo revisó y ajustó los casos de prueba para cubrir sus
  escenarios específicos

**Errores encontrados:**
- Ninguno en esta sesión

---

### Sesión 4 — Depuración e integración
**Fecha:** 03/06/2026

**Contexto:** Al ejecutar las pruebas, el equipo identificó dos bugs
en el parser y los reportó a la IA para diagnóstico y corrección.

**Prompts utilizados:**
- "El parser entra en loop infinito al procesar SECTION .text, el equipo
  identificó que el problema está en parse_operand, confirma y corrige"
- "MOV [resultado_suma], EAX falla porque parse_memory no acepta
  identificadores simbólicos, proporciona la corrección"

**Lo que generó la IA:**
- Confirmación del diagnóstico del equipo
- Corrección completa de src/parser.c con los dos fixes integrados

**Modificaciones manuales:**
- Aplicación de correcciones vía terminal en Codespaces
- Verificación manual de correctitud mediante pruebas
- Commit y push de cambios

**Errores encontrados:**

1. **Loop infinito en parser con SECTION .text**
   - Causa: .TEXT tokenizado como TOKEN_IDENTIFIER sin caso en
     parse_operand, el parser no avanzaba
   - Síntoma: miles de líneas "Error linea 6: operando invalido
     (encontrado: '.TEXT')"
   - Solución: agregar caso TOKEN_IDENTIFIER con advance() forzado

2. **Error en direccionamiento con identificadores simbólicos**
   - Causa: parse_memory solo aceptaba registros o números en corchetes
   - Síntoma: Error linea 22: se esperaba ']'
     (encontrado: 'RESULTADO_SUMA')
   - Solución: agregar caso TOKEN_IDENTIFIER en parse_memory

3. **Script run_tests.py se colgaba**
   - Causa: subprocess.run sin timeout cuando el ensamblador
     entraba en loop
   - Solución: detener con Ctrl+C y corregir el parser primero

4. **NASM no disponible en Codespaces**
   - Causa: repositorios de paquetes no actualizados
   - Solución: apt-get update antes de instalar NASM

---

### Sesión 5 — Validación final y documentación
**Fecha:** 06/06/2026

**Contexto:** El equipo realizó la validación final del proyecto
comparando la salida del ensamblador contra NASM y preparó la
documentación de entrega.

**Prompts utilizados:**
- "Genera el reporte técnico completo del proyecto en formato Word"
- "Genera una guía de explicación del proyecto que cubra objetivo,
  funcionamiento módulo a módulo y resultados esperados"
- "Actualiza la bitácora con todas las sesiones del proyecto"

**Lo que generó la IA:**
- Reporte técnico en formato .docx con todas las secciones requeridas
- Guía de explicación con analogías y ejemplos visuales
- Bitácora completa del proyecto

**Modificaciones manuales:**
- El equipo revisó y ajustó el contenido de los documentos
- Actualización del apartado de organización del equipo en el reporte

**Errores encontrados:**
- Conflicto de Git al hacer push desde Codespaces nuevo
- Causa: repositorio remoto con commits más recientes
- Solución: git rebase --abort seguido de git push --force

---

## 4. Resumen de código generado vs manual

| Módulo | Diseñado por el equipo | Generado por IA | Ajustado manualmente |
|---|---|---|---|
| include/lexer.h | 100% | Implementación | 0% |
| src/lexer.c | 100% | Implementación | 5% |
| include/parser.h | 100% | Implementación | 0% |
| src/parser.c | 100% | Implementación | 15% (bugs) |
| include/symbols.h | 100% | Implementación | 0% |
| src/symbols.c | 100% | Implementación | 0% |
| include/assembler.h | 100% | Implementación | 0% |
| src/assembler.c | 100% | Implementación | 0% |
| include/encoder.h | 100% | Implementación | 10% |
| src/encoder.c | 100% | Implementación | 10% |
| include/object.h | 100% | Implementación | 0% |
| src/object.c | 100% | Implementación | 0% |
| include/linker.h | 100% | Implementación | 0% |
| src/linker.c | 100% | Implementación | 0% |
| src/main.c | 100% | Implementación | 5% |
| Makefile | 100% | Implementación | 0% |
| Scripts Python | 80% | Implementación | 20% |
| Casos de prueba | 100% | Implementación | 5% |

---

## 5. Resultados de pruebas

| Prueba | Resultado |
|---|---|
| test_mov.asm — todos los modos de direccionamiento | PASS |
| test_jumps.asm — saltos y referencias adelantadas | PASS |
| test_call.asm — CALL y manejo de pila | PASS |
| test_extern.asm — símbolos externos | PASS |
| test_sib.asm — byte SIB con todas las escalas | PASS |
| test_multimodule — enlazado de dos módulos | PASS |
| test_reg8.asm — registros de 8 bits y MOVZX | PASS |
| Comparación con NASM (instrucciones simples) | Bytes idénticos |
| Compilación sin warnings con -Wall -Wextra | 0 warnings |

---

## 6. Observaciones finales

- El proceso más efectivo fue que el equipo diseñara primero y la IA
  implementara después, en lugar de delegar el diseño completamente
- Los prompts más efectivos especificaban el contexto técnico completo
  y referenciaban el diseño previo del equipo
- Los bugs encontrados fueron identificados por el equipo durante las
  pruebas y confirmados por la IA en el diagnóstico
- La validación byte a byte contra NASM fue propuesta por el equipo
  como criterio de correctitud objetivo
- La arquitectura modular elegida por el equipo facilitó la depuración
  al poder aislar y probar cada componente de forma independiente
