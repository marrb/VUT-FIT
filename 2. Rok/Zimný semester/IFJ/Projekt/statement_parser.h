/*
Project: Compiler for language IFJ21

File: statement_parser.h

AUTHORS:
Martin Bublavý <xbubla02@stud.fit.vutbr.cz>

*/

#include <stdio.h>
#include "error.h"
#include "parser.h"

#define BOOL 40
#define NIL 41

//datatype & final token return 

typedef struct{
    string token_s;
    int symbol;
}token_string;


typedef struct{
    int top;
    token_string *ts;
    int max_size;
}prec_stack;

typedef struct{
    int type;
    int token;
}statement_end_type;


typedef enum{
    PLUS,
    MINUS,
    MULTIPLY,
    DIVISION,
    INT_DIV,
    LESS_THAN,
    LT_EQUALS,
    BIGGER_THAN,
    BT_EQUALS,
    EQUALS,
    NOT_EQUAL,
    HASHTAG,
    CONCAT,
    LEFT_BRACKET,
    RIGHT_BRACKET,
    IDENTIFIER,
    INTEGER,
    STRING,
    DOUBLE,
    DOLLAR
}Symbol;


typedef enum{
    R_PLUS = 20,    //E => E + E
    R_MINUS,        //E => E - E
    R_MULTIPLY,     //E => E * E
    R_DIVISION,     //E => E / E
    R_INT_DIV,      //E => E // E
    R_LESS_THAN,    //E => E < E
    R_LT_EQUALS,    //E => E <= E
    R_BIGGER_THAN,  //E => E > E
    R_BT_EQUALS,    //E => E >= E
    R_EQUALS,       //E => E = E
    R_NOT_EQUAL,    //E => E ~= E
    R_PARS,         //E => (E)
    R_ID,           //E => i
    R_INT,          //E => int
    R_DOUBLE,       //E => int
    R_STRING,       //E => string
    R_HASHTAG,      //E => int
    R_CONCAT,       //E => string
    R_NX            //Not existent rule
}Rule;

statement_end_type statement_parse(bst_node_var *tree, Lexeme *L);
void rule_reduction(bst_node_var *var_tree, prec_stack *s, token_string *top_symbol);
void check_types(prec_stack *s);

//Stack-----------------------------------------------------
prec_stack * stack_init();
void free_stack(prec_stack *s);
int push_prec_stack(prec_stack *s, int symbol, string str);
int pop__prec_stack(prec_stack *s);
int is_full_stack(prec_stack *s);
int is_empty_stack(prec_stack *s);
token_string *top(prec_stack *s);
token_string *top_2(prec_stack *s);
token_string *top_3(prec_stack *s);
token_string *get_symbol(prec_stack *s);
