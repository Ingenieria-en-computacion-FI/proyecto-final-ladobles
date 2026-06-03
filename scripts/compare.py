#!/usr/bin/env python3
# ============================================================
# compare.py
# Compara la salida de nuestro ensamblador contra NASM
# Uso: python3 scripts/compare.py <archivo.asm>
# ============================================================

import sys
import os
import subprocess
import tempfile

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

def hexdump_bytes(data):
    return " ".join(f"{b:02X}" for b in data)

def compare(asm_file):
    if not os.path.exists(asm_file):
        print(f"Error: no se encontro '{asm_file}'")
        sys.exit(1)

    base = os.path.splitext(asm_file)[0]

    # --------------------------------------------------------
    # Ensamblar con nuestro ensamblador
    # --------------------------------------------------------
    our_output = base + "_ours.bin"
    code, out, err = run_command(
        f"./assembler -al {asm_file} -o {our_output}"
    )
    if code != 0:
        print(f"Error en nuestro ensamblador:\n{err}")
        sys.exit(1)

    # --------------------------------------------------------
    # Ensamblar con NASM si esta disponible
    # --------------------------------------------------------
    nasm_output = base + "_nasm.bin"
    code, out, err = run_command(
        f"nasm -f bin {asm_file} -o {nasm_output}"
    )
    nasm_available = (code == 0)

    # --------------------------------------------------------
    # Leer archivos
    # --------------------------------------------------------
    with open(our_output, "rb") as f:
        our_bytes = f.read()

    print(f"Archivo:          {asm_file}")
    print(f"Nuestro output:   {our_output} ({len(our_bytes)} bytes)")
    print()
    print("--- NUESTRO ENSAMBLADOR ---")
    for i in range(0, len(our_bytes), 16):
        chunk = our_bytes[i:i+16]
        print(f"  {i:04X}: {hexdump_bytes(chunk)}")

    if nasm_available:
        with open(nasm_output, "rb") as f:
            nasm_bytes = f.read()

        print()
        print("--- NASM ---")
        for i in range(0, len(nasm_bytes), 16):
            chunk = nasm_bytes[i:i+16]
            print(f"  {i:04X}: {hexdump_bytes(chunk)}")

        # Comparar byte a byte
        print()
        print("--- COMPARACION ---")
        if our_bytes == nasm_bytes:
            print("✓ IDENTICOS - salida correcta")
        else:
            print("✗ DIFERENCIAS encontradas:")
            min_len = min(len(our_bytes), len(nasm_bytes))
            diffs   = 0
            for i in range(min_len):
                if our_bytes[i] != nasm_bytes[i]:
                    print(f"  offset 0x{i:04X}: "
                          f"nuestro={our_bytes[i]:02X} "
                          f"nasm={nasm_bytes[i]:02X}")
                    diffs += 1
            if len(our_bytes) != len(nasm_bytes):
                print(f"  tamanios distintos: "
                      f"nuestro={len(our_bytes)} "
                      f"nasm={len(nasm_bytes)}")
            print(f"  Total diferencias: {diffs}")
    else:
        print()
        print("NASM no disponible, solo se muestra nuestra salida")

    # Limpiar archivos temporales
    for f in [our_output, nasm_output]:
        if os.path.exists(f):
            os.remove(f)

if __name__ == "__main__":
    if len(sys.argv) < 2:
        print("Uso: python3 scripts/compare.py <archivo.asm>")
        sys.exit(1)

    compare(sys.argv[1])
