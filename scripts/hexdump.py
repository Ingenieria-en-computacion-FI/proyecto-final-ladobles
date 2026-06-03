#!/usr/bin/env python3
# ============================================================
# hexdump.py
# Muestra el contenido de un archivo en formato hexadecimal
# Uso: python3 scripts/hexdump.py <archivo>
# ============================================================

import sys
import os

def hexdump(filename):
    if not os.path.exists(filename):
        print(f"Error: no se encontro el archivo '{filename}'")
        sys.exit(1)

    with open(filename, "rb") as f:
        data = f.read()

    print(f"Archivo: {filename}")
    print(f"Tamanio: {len(data)} bytes")
    print()
    print("Offset    00 01 02 03 04 05 06 07  08 09 0A 0B 0C 0D 0E 0F  ASCII")
    print("-" * 70)

    for i in range(0, len(data), 16):
        chunk = data[i:i+16]

        # Offset
        offset = f"{i:08X}"

        # Bytes en hex
        hex_part1 = " ".join(f"{b:02X}" for b in chunk[:8])
        hex_part2 = " ".join(f"{b:02X}" for b in chunk[8:])

        # Padding si el chunk es menor a 16
        hex_part1 = hex_part1.ljust(23)
        hex_part2 = hex_part2.ljust(23)

        # ASCII
        ascii_part = "".join(
            chr(b) if 32 <= b < 127 else "."
            for b in chunk
        )

        print(f"{offset}  {hex_part1}  {hex_part2}  {ascii_part}")

    print()

if __name__ == "__main__":
    if len(sys.argv) < 2:
        print("Uso: python3 scripts/hexdump.py <archivo>")
        sys.exit(1)

    hexdump(sys.argv[1])
