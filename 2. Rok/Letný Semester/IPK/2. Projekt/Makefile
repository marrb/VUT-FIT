PROJ=ipk-sniffer
CFLAGS=-std=gnu99 -Wall -Wextra -Werror -pedantic
CC=gcc
LIBS=-lm -lpcap
RM=rm -f
FILES=ipk-sniffer.c

$(PROJ) : $(FILES)
	$(CC) $(CFLAGS) -o $(PROJ) $(FILES) $(LIBS)
	
clean:
	$(RM) *.o $(PROJ)