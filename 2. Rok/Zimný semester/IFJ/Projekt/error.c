/*
Project: Compiler for language IFJ21

File: error.c

AUTHORS:
Martin Bublavý <xbubla02@stud.fit.vutbr.cz>
Adrián Hučko <xhucko02@stud.fit.vutbr.cz


*/


#include "error.h"
// error printout by number
void error_out(int error_index, char* error_reason){
    fprintf(stderr,"ERROR (%i): [%s]\n", error_index, error_reason);
    exit(error_index);
}
