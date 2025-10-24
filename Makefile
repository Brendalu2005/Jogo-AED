CC = gcc
EXEC = main
SRCDIR = src
OBJDIR = object
INCDIR = include

CFLAGS = -Wall -Wextra -std=c99 -I$(INCDIR)

LDFLAGS = -lraylib -lm -lpthread -ldl -lrt -lX11

SRC = $(wildcard $(SRCDIR)/*.c)

OBJ = $(patsubst $(SRCDIR)/%.c, $(OBJDIR)/%.o, $(SRC))

RM = rm -f
all: $(EXEC)

$(EXEC): $(OBJ)
	@echo "Ligando o executável $(EXEC)..."
	$(CC) $(OBJ) -o $(EXEC) $(LDFLAGS)
	@echo "Pronto! Execute com: ./$(EXEC)"

$(OBJDIR)/%.o: $(SRCDIR)/%.c
	@mkdir -p $(OBJDIR)
	@echo "Compilando $< -> $@"
	$(CC) $(CFLAGS) -c $< -o $@
clean:
	@echo "Limpando arquivos compilados..."
	$(RM) $(OBJDIR)/*.o $(EXEC)

.PHONY: all clean