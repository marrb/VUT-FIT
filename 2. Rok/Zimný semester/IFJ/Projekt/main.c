/*
Project: Compiler for language IFJ21

File: main.c

AUTHORS:
Martin Bublavý <xbubla02@stud.fit.vutbr.cz>

*/

#include <stdio.h>
#include "error.h"
#include "parser.h"

int main(){
FILE* source; //Source stream
source = stdin;
int result;
set_source_stream(source);

result = parse();

if(result != 0){
    error_out(result, "No token could be read!");
}

exit(0);
}

//gcc -o main main.c error.c error.h parser.c parser.h scanner.c scanner.h statement_parser.c statement_parser.h symtable.c symtable.h -Wall -Werror
//Get-Content kod.txt | ./main.exe