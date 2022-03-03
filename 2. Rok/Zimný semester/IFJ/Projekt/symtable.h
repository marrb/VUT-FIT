/*
Project: Compiler for language IFJ21

File: symtable.h

AUTHORS:
Martin Bublavý <xbubla02@stud.fit.vutbr.cz>
Adrián Hučko <xhucko02@stud.fit.vutbr.cz

*/

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <stdbool.h>


typedef struct Var_Stack{                          
    int type;
    int scope;
    struct Var_Stack *Prev_Element;
}*Var_Stack;

typedef struct bst_node_var{
    char *key;
    int key_alloc_lenght;
    int key_lenght;    

    struct Var_Stack *var_stack;                    

	  struct bst_node_var *left;         
	  struct bst_node_var *right;        
} *bst_node_var;


typedef struct bst_node_function {
  char *key;
  int key_alloc_lenght;
  int key_lenght;

  bool Declared;
  bool Called;
  bool Global_Dec;

  struct function_list *return_codes;
  struct function_list *params;  
          
  struct bst_node_function *left;  
  struct bst_node_function *right; 
} *bst_node_function;

typedef struct function_list_element{
  int type;
  int sequence;

  struct function_list_element *Next;
}*function_list_element;

typedef struct function_list{
  function_list_element First;
  int num_of_elements;
}function_list;


void bst_init_var(bst_node_var *tree);
void bst_insert_var(bst_node_var *tree,  char *key, int type, int scope);
bst_node_var bst_search_var(bst_node_var tree, char *S_key);
void bst_dispose_var(bst_node_var *tree);
bool bst_Declared_var(bst_node_var tree, char *key);
int bst_Get_Type_var(bst_node_var tree, char *key);
void bst_Replace_By_Rightmost_var(bst_node_var Target, bst_node_var *tree);
void bst_delete_var(bst_node_var *tree, char *key);
void bst_scope_delete_var(bst_node_var *tree, int new_s);
int bst_insert_key(bst_node_var *tree, char *name);

void pop_stack(bst_node_var *tree);
void push_stack(bst_node_var *tree, int type, int scope);
bool delete_stack(bst_node_var *tree);

void bst_init_function(bst_node_function *tree);
void bst_insert_function(bst_node_function *tree, char *key);
bst_node_function *bst_search_function(bst_node_function *tree, char *S_key);
void bst_dispose_function(bst_node_function *tree);
void function_change(bst_node_function *tree, char *key, bool declared, bool called, int parameter_C);
void Insert_Function_Call(bst_node_function *tree, char *key, bst_node_var varTree, int parameter_C);
void Insert_Function_Declaration(bst_node_function *tree, char *key, int parameter_C);
void Insert_Function_Parameter(bst_node_function *tree, char *key, int parameter_T, int parameter_seq);
void Insert_Return(bst_node_function *tree, char *key, int return_T, int return_seq);
void is_function_declared(bst_node_function tree);
void Function_Return_Check(bst_node_function *tree, char *key, int return_C);
void Function_Compare_List(function_list *function_list, bst_node_function *tree, char *key, int return_C);
void Function_init_params(bst_node_function *tree, char *key);
int bst_insert_key_func(bst_node_function *tree, char *name);

void List_Init(function_list *list);
void List_Add(function_list *list, int value, int seq);
function_list_element List_Search(function_list *list, int seq);
void List_Delete(function_list *list);
void List_Check_Element(function_list *list, int type, int seq);
void List_Compare(function_list *list_A, function_list *list_B);