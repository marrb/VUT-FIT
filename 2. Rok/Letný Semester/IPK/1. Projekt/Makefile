PROJ=hinfosvc
CFLAGS=-std=gnu99 -Wall -Wextra -Werror
CC=gcc
LIBS=-lm
RM=rm -f
FILES=server.c

$(PROJ) : $(FILES)
	$(CC) $(CFLAGS) -o $(PROJ) $(FILES) $(LIBS)
	
clean:
	$(RM) *.o $(PROJ)