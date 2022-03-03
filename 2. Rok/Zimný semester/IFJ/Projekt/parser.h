/*
Project: Compiler for language IFJ21

File: parser.h

AUTHORS:
Martin Bublavý <xbubla02@stud.fit.vutbr.cz>
Igor Pszota <xpszot00@stud.fit.vutbr.cz>

*/

#include "symtable.h"
#include "scanner.h"

typedef struct{
    char *str;
    int string_lenght;
    int string_alloc_lenght;
}string;

int parse();
int Find_Header();
int function_def_list();
int function_def();
int function_params(bst_node_var *var_tree);
int function_returns();
int statement_list_rules(bst_node_var *tree);
int statement(bst_node_var *var_tree);
int assign_right(bst_node_var *var_tree, function_list *assign_var_list, int Var_Counter, function_list *assign_assign_list, int assign_assign_counter);
int assign_left(bst_node_var *var_tree, function_list *assign_var_list, int Var_Counter);
int function_call_parameters(bst_node_var *var_tree);
int write_parameters(bst_node_var *var_tree);
int return_values(bst_node_var *var_tree);
void skip_whitespaces();
int global_func_def();

void init_string(string *str);
void free_string(string *str);
void string_clear(string *str);
void copy_string(string *dest, char *source);
