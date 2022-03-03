/*
Project: Compiler for language IFJ21

File: error.h

AUTHORS:
Martin Bublavý <xbubla02@stud.fit.vutbr.cz>
Adrián Hučko <xhucko02@stud.fit.vutbr.cz

*/

#ifndef ERROR_H
#define ERROR_H

#include <stdio.h>
#include <stdlib.h>
// definition of error level
#define LEXEMA_SYNTAX_ERROR 1
#define SYNTAX_ERROR 2
#define SEM_ERROR_NDEFINED 3
#define SEM_ERROR_INCOMP_TYPE 4
#define SEM_ERROR_FUNCTION 5
#define SEM_ERROR_INCOMP_ASSIGN_INT_STRING 6
#define SEM_ERROR_OTHER 7
#define NIL_ERROR 8
#define ZERO_DIV_ERROR 9
#define INTERNAL_ERROR 99

void error_out (int error_index, char* error_reason);

#endif 
