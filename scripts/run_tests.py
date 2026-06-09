#!/usr/bin/env python3
# ============================================================
# run_tests.py
# Ejecuta todos los casos de prueba y reporta resultados
# Uso: python3 scripts/run_tests.py
# ============================================================

import sys
import os
import subprocess

# ============================================================
# Colores para la terminal
# ============================================================
GREEN  = "\033[92m"
RED    = "\033[91m"
YELLOW = "\033[93m"
BLUE   = "\033[94m"
RESET  = "\033[0m"

def run_command(cmd):
    try:
        result = subprocess.run(
            cmd, shell=True,
            capture_output=True,
            text=True
        )
        return result.returncode, result.stdout, result.stderr
    except Exception as e:
        return -1, "", str(e)

def print_header(title):
    print()
    print(f"{BLUE}{'=' * 60}{RESET}")
    print(f"{BLUE}  {title}{RESET}")
    print(f"{BLUE}{'=' * 60}{RESET}")

def print_result(name, passed, output="", error=""):
    if passed:
        print(f"  {GREEN}✓ PASS{RESET} — {name}")
    else:
        print(f"  {RED}✗ FAIL{RESET} — {name}")
        if error:
            for line in error.strip().split("\n"):
                print(f"         {YELLOW}{line}{RESET}")

# ============================================================
# Definicion de pruebas
# ============================================================

TESTS_SINGLE = [
    {
        "name":  "MOV — todos los modos de direccionamiento",
        "cmd":   "./assembler -al examples/test_mov.asm -o /tmp/test_mov.bin",
    },
    {
        "name":  "Saltos y referencias adelantadas",
        "cmd":   "./assembler -al examples/test_jumps.asm -o /tmp/test_jumps.bin",
    },
    {
        "name":  "CALL y manejo de pila",
        "cmd":   "./assembler -al examples/test_call.asm -o /tmp/test_call.bin",
    },
    {
        "name":  "Simbolos externos y relocaciones",
        "cmd":   "./assembler -a examples/test_extern.asm -o /tmp/test_extern.o && ./assembler -p /tmp/test_extern.on",
    },
    {
        "name":  "SIB — todas las escalas",
        "cmd":   "./assembler -al examples/test_sib.asm -o /tmp/test_sib.bin",
    },
]

TESTS_ONE_PASS = [
    {
        "name":  "MOV una pasada",
        "cmd":   "./assembler -1 -al examples/test_mov.asm -o /tmp/test_mov_1p.bin",
    },
    {
        "name":  "Saltos una pasada",
        "cmd":   "./assembler -1 -al examples/test_jumps.asm -o /tmp/test_jumps_1p.bin",
    },
    {
        "name":  "CALL una pasada",
        "cmd":   "./assembler -1 -al examples/test_call.asm -o /tmp/test_call_1p.bin",
    },
]

TESTS_MULTIMODULE = [
    {
        "name":  "Multimodulo — ensamblar modulo A",
        "cmd":   "./assembler -a examples/test_multimodule_a.asm -o /tmp/mod_a.o",
    },
    {
        "name":  "Multimodulo — ensamblar modulo B",
        "cmd":   "./assembler -a examples/test_multimodule_b.asm -o /tmp/mod_b.o",
    },
    {
        "name":  "Multimodulo — linkear A y B",
        "cmd":   "./assembler -l /tmp/mod_a.o /tmp/mod_b.o -o /tmp/multimodule.bin",
    },
]

TESTS_VERBOSE = [
    {
        "name":  "Verbose — tokens y AST visibles",
        "cmd":   "./assembler -v -al examples/test_mov.asm -o /tmp/test_verbose.bin",
    },
]

# ============================================================
# Ejecutar pruebas
# ============================================================

def run_suite(title, tests):
    print_header(title)
    passed = 0
    failed = 0

    for test in tests:
        code, out, err = run_command(test["cmd"])
        ok = (code == 0)
        print_result(test["name"], ok, out, err)
        if ok:
            passed += 1
        else:
            failed += 1

    return passed, failed

def main():
    # Verificar que el ensamblador existe
    if not os.path.exists("./assembler"):
        print(f"{RED}Error: no se encontro './assembler'")
        print(f"Ejecuta 'make' primero{RESET}")
        sys.exit(1)

    total_passed = 0
    total_failed = 0

    suites = [
        ("Pruebas de dos pasadas",   TESTS_SINGLE),
        ("Pruebas de una pasada",    TESTS_ONE_PASS),
        ("Pruebas multimodulo",      TESTS_MULTIMODULE),
        ("Pruebas verbose",          TESTS_VERBOSE),
    ]

    for title, tests in suites:
        p, f = run_suite(title, tests)
        total_passed += p
        total_failed += f

    # Resumen final
    total = total_passed + total_failed
    print()
    print(f"{BLUE}{'=' * 60}{RESET}")
    print(f"{BLUE}  RESUMEN FINAL{RESET}")
    print(f"{BLUE}{'=' * 60}{RESET}")
    print(f"  Total:   {total}")
    print(f"  {GREEN}Pasaron: {total_passed}{RESET}")
    print(f"  {RED}Fallaron: {total_failed}{RESET}")

    if total_failed == 0:
        print(f"\n  {GREEN}✓ Todas las pruebas pasaron{RESET}")
    else:
        print(f"\n  {RED}✗ Hay pruebas fallando{RESET}")

    print()
    sys.exit(0 if total_failed == 0 else 1)

if __name__ == "__main__":
    main()
