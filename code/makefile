# Compiler
CC = gcc
# Options de compilation
CFLAGS = -Wall -Wextra -g
# Fichiers source
SRCS = main.c 
HEADER = main.h
# Fichiers objets
OBJS = $(SRCS:.c=.o)
# Exécutable
EXEC = main

# Doxygen configuration file
DOXYFILE = main.c


all: $(EXEC) 

# Règle de construction de l'exécutable
$(EXEC): $(OBJS)
	$(CC) $(CFLAGS) $(OBJS) -o $(EXEC)

	
# Règle de construction des fichiers objets
%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

# Nettoyer les fichiers objets et l'exécutable
clean:
	rm -f $(OBJS) $(EXEC)


	