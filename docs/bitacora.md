# Bitácora IA — IA-32 Assembler & Mini Linker

## Universidad Nacional Autónoma de México
### Estructura y Programación de Computadoras 2026-2
### Ing. Adara Mercado Martínez

---

## 1. Herramientas IA utilizadas

| Herramienta | Versión | Uso principal |
|---|---|---|
| Claude (Anthropic) | Sonnet 4.5 | Diseño de arquitectura, generación de código, depuración |

---

## 2. Registro de sesiones

### Sesión 1 — Análisis y planificación del proyecto
**Fecha:** 02/06/2026

**Prompts utilizados:**
- "Analiza los requisitos del proyecto e identifica ambigüedades en la especificación que requieran clarificación con la profesora"
- "Analiza un pseudocódigo modular completo del sistema ensamblador-linker siguiendo las responsabilidades definidas en el documento"
- "Explica la interacción entre módulos y el flujo de datos desde el archivo ASM hasta el binario final"

**Lo que generó la IA:**
- Análisis de ambigüedades en la especificación (lenguaje, formato objeto, tipo de binario final)
- Pseudocódigo completo del sistema dividido en 9 módulos
- Diagrama de flujo del pipeline de ensamblado y enlazado

**Modificaciones manuales:**
- Comparación del pseudocódigo generado contra diseño propio previo

**Errores encontrados:**
- Ninguno

---

### Sesión 2 — Implementación de la arquitectura base
**Fecha:** 02/06/2026

**Prompts utilizados:**
- "Implementa en C el lexer para IA-32 con soporte para registros, instrucciones, directivas, números hexadecimales y manejo de comentarios"
- "Implementa el parser con soporte para los 7 modos de direccionamiento IA-32 incluyendo SIB completo"
- "Implementa la tabla de símbolos con soporte para símbolos globales, externos y detección de redefiniciones"
- "Implementa el ensamblador de una y dos pasadas con manejo de fixups y relocaciones"
- "Implementa el encoder IA-32 con construcción de ModRM y SIB para todas las escalas soportadas"
- "Diseña e implementa un formato objeto propio con encabezado, tabla de secciones, símbolos y relocaciones"
- "Implementa el linker con fusión de secciones, resolución de símbolos externos y aplicación de relocaciones"
- "Implementa el punto de entrada principal con soporte para modos ensamblar, linkear y ensamblar-linkear"

**Lo que generó la IA:**
- Implementación completa de todos los módulos del sistema en C
- Archivos de cabecera con definición de estructuras y prototipos
- Makefile con reglas de compilación, limpieza y pruebas

**Modificaciones manuales:**
- Integración y creación de archivos en el repositorio

**Errores encontrados:**
- **Error de ubicación de archivo** — `src/lexer.c` fue creado accidentalmente dentro de `include/src/` en lugar de `src/`. Causa: error al crear la carpeta en GitHub desde el navegador. Solución: editar la ruta del archivo directamente desde la interfaz de GitHub y eliminar el archivo duplicado.

---

### Sesión 3 — Casos de prueba y herramientas auxiliares
**Fecha:** 03/06/2026

**Prompts utilizados:**
- "Genera casos de prueba ASM que cubran los 8 requisitos mínimos: MOV inmediato, saltos, referencias adelantadas, EXTERN, CALL, relocaciones, múltiples módulos y SIB"
- "Implementa en Python un script de hexdump para inspección de binarios generados"
- "Implementa un script de comparación byte a byte contra NASM para validación de correctitud"
- "Implementa un script de pruebas automatizadas con reporte de resultados por suite"

**Lo que generó la IA:**
- 7 casos de prueba cubriendo todos los requisitos del proyecto
- 3 scripts Python para inspección, comparación y automatización de pruebas

**Modificaciones manuales:**
- Creación de archivos en el repositorio

**Errores encontrados:**
- Ninguno

---

### Sesión 4 — Depuración e integración
**Fecha:** 03/06/2026

**Prompts utilizados:**
- "El parser entra en loop infinito al procesar la directiva SECTION .text — analiza la causa raíz y proporciona la corrección"
- "El parser falla al procesar operandos de memoria con identificadores simbólicos como [resultado_suma] — corrige parse_memory para manejar este caso"

**Lo que generó la IA:**
- Diagnóstico de causa raíz de ambos bugs
- Corrección completa de `src/parser.c` con los dos fixes integrados

**Modificaciones manuales:**
- Aplicación de correcciones vía terminal con comando `cat`
- Verificación de correctitud mediante pruebas
- Commit y push de cambios corregidos

**Errores encontrados:**

1. **Loop infinito en parser con SECTION .text**
   - **Causa:** `.TEXT` era tokenizado como `TOKEN_IDENTIFIER` y `parse_operand` no tenía caso para ese tipo, por lo que el parser no avanzaba y entraba en loop infinito imprimiendo el mismo error indefinidamente
   - **Síntoma:** Miles de líneas `Error linea 6: operando invalido (encontrado: '.TEXT')` llenando la terminal
   - **Solución:** Agregar caso `TOKEN_IDENTIFIER` en `parse_operand` con `advance()` para evitar el loop

2. **Error en direccionamiento con identificadores simbólicos**
   - **Causa:** `parse_memory` solo aceptaba registros o números como contenido de corchetes. Instrucciones como `MOV [resultado_suma], EAX` fallaban porque `resultado_suma` es un identificador
   - **Síntoma:** `Error linea 22: se esperaba ']' (encontrado: 'RESULTADO_SUMA')`
   - **Solución:** Agregar caso `TOKEN_IDENTIFIER` en `parse_memory` para aceptar símbolos como direcciones de memoria

3. **Script run_tests.py se colgaba indefinidamente**
   - **Causa:** `subprocess.run` no tenía timeout configurado. Cuando el ensamblador entraba en loop infinito por el bug anterior, el script esperaba eternamente su terminación
   - **Síntoma:** El script mostraba "Pruebas de dos pasadas" y se quedaba sin avanzar
   - **Solución:** Detener con Ctrl+C y corregir primero el bug del parser antes de volver a ejecutar las pruebas

4. **NASM no disponible en Codespaces**
   - **Causa:** El Codespace no tenía los repositorios de paquetes actualizados, por lo que `apt-get install nasm` fallaba con "Unable to locate package nasm"
   - **Síntoma:** `E: Unable to locate package nasm`
   - **Solución:** Ejecutar `apt-get update` primero para actualizar los repositorios y luego instalar NASM correctamente

---

## 3. Resumen de código generado vs manual

| Módulo | Generado por IA | Modificado manualmente |
|---|---|---|
| `include/lexer.h` | 100% | 0% |
| `src/lexer.c` | 100% | 0% |
| `include/parser.h` | 100% | 0% |
| `src/parser.c` | 90% | 10% (corrección de bugs) |
| `include/symbols.h` | 100% | 0% |
| `src/symbols.c` | 100% | 0% |
| `include/assembler.h` | 100% | 0% |
| `src/assembler.c` | 100% | 0% |
| `include/encoder.h` | 100% | 0% |
| `src/encoder.c` | 100% | 0% |
| `include/object.h` | 100% | 0% |
| `src/object.c` | 100% | 0% |
| `include/linker.h` | 100% | 0% |
| `src/linker.c` | 100% | 0% |
| `src/main.c` | 100% | 0% |
| `Makefile` | 100% | 0% |
| Scripts Python | 100% | 0% |
| Casos de prueba | 100% | 0% |

---

## 4. Resultados de pruebas

| Prueba | Resultado |
|---|---|
| `test_mov.asm` — todos los modos de direccionamiento | ✅ PASS |
| `test_jumps.asm` — saltos y referencias adelantadas | ✅ PASS |
| `test_call.asm` — CALL y manejo de pila | ✅ PASS |
| `test_extern.asm` — símbolos externos | ✅ PASS |
| `test_sib.asm` — byte SIB con todas las escalas | ✅ PASS |
| `test_multimodule` — enlazado de dos módulos | ✅ PASS |
| Comparación con NASM (instrucciones simples) | ✅ Bytes idénticos |
| Compilación sin warnings | ✅ 0 warnings |

---

## 5. Observaciones

- El uso de IA permitió acelerar significativamente la fase de implementación
- Los prompts más efectivos fueron los que especificaban el contexto técnico completo incluyendo arquitectura objetivo, restricciones del proyecto y módulo específico
- Los bugs encontrados requirieron comprensión profunda del código generado para poder diagnosticarlos y corregirlos correctamente
- La arquitectura modular facilitó el aislamiento y corrección de errores al poder probar cada componente de forma independiente
- La validación byte a byte contra NASM confirmó la correctitud del encoder IA-32
- El error del script de pruebas demostró la importancia de agregar timeouts en procesos automatizados
