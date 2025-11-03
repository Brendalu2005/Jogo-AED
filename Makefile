CC = gcc
EXEC = main
SRCDIR = src
OBJDIR = object
INCDIR = include

INCLUDES = -I$(INCDIR) \
           -I$(INCDIR)/tela \
           -I$(INCDIR)/batalha_data \
           -I$(INCDIR)/AI \
           -I$(INCDIR)/json

CFLAGS = -Wall -Wextra -std=c99 $(INCLUDES)

LDFLAGS = -lraylib -lm -lpthread -ldl -lrt -lX11 -lcurl

SRC = $(shell find $(SRCDIR) -name '*.c')

OBJ = $(patsubst $(SRCDIR)/%.c, $(OBJDIR)/%.o, $(SRC))

RM = rm -f
all: $(EXEC)

$(EXEC): $(OBJ)
	@echo "Ligando o executável $(EXEC)..."
	$(CC) $(OBJ) -o $(EXEC) $(LDFLAGS)
	@echo "Pronto! Execute com: ./$(EXEC)"

$(OBJDIR)/%.o: $(SRCDIR)/%.c
	@mkdir -p $(@D)
	@echo "Compilando $< -> $@"
	$(CC) $(CFLAGS) -c $< -o $@
clean:
	@echo "Limpando arquivos compilados..."
	$(RM) $(OBJDIR)/*.o $(EXEC)

.PHONY: all clean