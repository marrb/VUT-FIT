PROJ=ifj21
CFLAGS=-std=gnu99 -Wall -Wextra -Werror
CC=gcc
LIBS=-lm
RM=rm -f
FILES=main.c scanner.c error.c parser.c statement_parser.c symtable.c

$(PROJ) : $(FILES)
	$(CC) $(CFLAGS) -o $(PROJ) $(FILES) $(LIBS)
	
clean:
	$(RM) *.o $(PROJ)