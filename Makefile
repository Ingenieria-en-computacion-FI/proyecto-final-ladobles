# ============================================================
# Makefile - IA-32 Assembler & Mini Linker
# ============================================================

CC      = gcc
CFLAGS  = -Wall -Wextra -g -Iinclude
SRC_DIR = src
OBJ_DIR = build

# Archivos fuente y objetos
SRCS = $(SRC_DIR)/lexer.c     \
       $(SRC_DIR)/parser.c    \
       $(SRC_DIR)/symbols.c   \
       $(SRC_DIR)/encoder.c   \
       $(SRC_DIR)/assembler.c \
       $(SRC_DIR)/object.c    \
       $(SRC_DIR)/linker.c    \
       $(SRC_DIR)/main.c

OBJS = $(patsubst $(SRC_DIR)/%.c, $(OBJ_DIR)/%.o, $(SRCS))

TARGET = assembler

# ============================================================
# Reglas principales
# ============================================================

all: $(OBJ_DIR) $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) -o $@ $^
	@echo "Compilado: $(TARGET)"

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c
	$(CC) $(CFLAGS) -c $< -o $@

$(OBJ_DIR):
	mkdir -p $(OBJ_DIR)

# ============================================================
# Limpieza
# ============================================================

clean:
	rm -rf $(OBJ_DIR) $(TARGET)
	@echo "Limpieza completa"

# ============================================================
# Pruebas rapidas
# ============================================================

test: all
	@echo "Ejecutando pruebas..."
	./assembler -v -al examples/test_mov.asm -o examples/test_mov.bin

.PHONY: all clean test
