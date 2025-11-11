# --- Variáveis de Compilação ---
CC = gcc
# Flags do compilador
CFLAGS = -Wall -Wextra -std=c99 -fPIE
# Flags do linker (bibliotecas)
LDFLAGS = -lraylib -lm -lpthread -ldl -lrt -lX11 -lcurl

# --- Caminhos ---
# Onde incluir os .h
INCLUDE_DIRS = -Iinclude -Iinclude/tela -Iinclude/batalha_data -Iinclude/AI -Iinclude/json
# Onde estão os .c
SRC_DIR = src
# Onde salvar os .o
OBJ_DIR = object
# Nome do executável final
TARGET = main

# --- Arquivos Fonte (src) ---
# Encontra automaticamente TODOS os arquivos .c dentro de SRC_DIR e suas subpastas
SOURCES = $(wildcard $(SRC_DIR)/*.c $(SRC_DIR)/**/*.c)

# --- Arquivos Objeto (object) ---
# Converte a lista de "src/telas/menu.c" para "object/telas/menu.o"
OBJECTS = $(SOURCES:$(SRC_DIR)/%.c=$(OBJ_DIR)/%.o)

# --- NOVO: Lista de todos os headers ---
# Isto garante que se um .h mudar, os .o serão recompilados.
HEADERS = $(wildcard include/*.h include/tela/*.h include/batalha_data/*.h include/AI/*.h include/json/*.h)

# --- Regras ---

# Regra padrão (o que acontece quando você digita "make")
all: $(TARGET)

# Regra para ligar (linkar) o executável final
$(TARGET): $(OBJECTS)
	@echo "Ligando o executável $(TARGET)..."
	$(CC) $(OBJECTS) -o $(TARGET) $(LDFLAGS)
	@echo "Pronto! Execute com: ./$(TARGET)"

# --- MODIFICADO: A regra de compilação agora depende dos HEADERS ---
$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c $(HEADERS)
	@echo "Compilando $< -> $@"
	# Cria a subpasta (ex: object/telas) ANTES de compilar.
	# Isso é essencial para funcionar depois de um "clean".
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) $(INCLUDE_DIRS) -c $< -o $@

# Regra de limpeza
clean:
	@echo "Limpando arquivos compilados..."
	# CORREÇÃO: Remove a pasta "object" inteira e o executável "main"
	# Isso garante que todos os .o antigos em subpastas sejam apagados.
	rm -rf $(OBJ_DIR) $(TARGET)

# Regra "phony" para dizer que "all" e "clean" não são nomes de arquivos
.PHONY: all clean