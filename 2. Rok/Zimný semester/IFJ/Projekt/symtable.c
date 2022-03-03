/*
Project: Compiler for language IFJ21

File: symtable.c

AUTHORS:
Martin Bublavý <xbubla02@stud.fit.vutbr.cz>
Adrián Hučko <xhucko02@stud.fit.vutbr.cz

*/

#include "symtable.h"
#include <stdio.h>
#include <stdlib.h>
#include "error.h"

#define DEFAULT_KEY_LENGHT 10

void bst_init_var(bst_node_var *tree){
  *tree = NULL;
}

bst_node_var bst_search_var(bst_node_var tree, char *S_key)
{
  if(tree==NULL) return NULL;

  if(strcmp(tree->key, S_key) < 0){
    return bst_search_var(tree->left, S_key);
  }
  else if(strcmp(tree->key, S_key) > 0){
    return bst_search_var(tree->right, S_key);
  }
  
  return tree;
}

void bst_insert_var(bst_node_var *tree,  char *I_key, int type, int scope){
  if(*tree==NULL){
		if((*tree = (bst_node_var)malloc(sizeof(struct bst_node_var)))==NULL) return;

    (*tree)->var_stack = NULL;
    (*tree)->right = NULL;
    (*tree)->left = NULL;		
    (*tree)->key = (char *) malloc(DEFAULT_KEY_LENGHT);
    push_stack(tree, type, scope);
    bst_insert_key(tree, I_key);

    if(!(*tree)->key) error_out(INTERNAL_ERROR, "Malloc Error!");
    return;
  }

  if(strcmp((*tree)->key, I_key) < 0){
    bst_insert_var(&((*tree)->left), I_key, type, scope);
    return;
  }
  else if(strcmp((*tree)->key, I_key) > 0){
    bst_insert_var(&((*tree)->right), I_key, type, scope);
    return;
  }

  if(scope > (*tree)->var_stack->scope) push_stack(tree, type, scope);
  else error_out(SEM_ERROR_NDEFINED, "Redefinition of variable in the same scope!");
}

void bst_Replace_By_Rightmost_var(bst_node_var Target, bst_node_var *tree){
  if((*tree)==NULL) return;

  if((*tree)->right != NULL) bst_Replace_By_Rightmost_var(Target, &((*tree)->right));
  else{
    Target->key_alloc_lenght = 0;
    Target->key_lenght = 0;

    delete_stack(&Target);
    bst_insert_key(&Target, (*tree)->key);
    Target->var_stack = (*tree)->var_stack;

    free((*tree)->key);
    (*tree) = (*tree)->left;
  }
}

void bst_delete_var(bst_node_var *tree, char *key){
  if((*tree) == NULL) return;
	if(strcmp((*tree)->key, key) < 0) bst_delete_var(&((*tree)->left), key);
  else if(strcmp((*tree)->key, key) > 0) bst_delete_var(&((*tree)->right), key);
  else if((*tree)->left != NULL && (*tree)->right != NULL) bst_Replace_By_Rightmost_var((*tree), &((*tree)->left));
  else{
    bst_node_var backup = (*tree);
    if((*tree)->left != NULL){
      (*tree) = (*tree)->left;
    }
    else{
      (*tree) = (*tree)->right;
    }

    delete_stack(&backup);
    free(backup->key);
    free(backup);
  }
}


void bst_dispose_var(bst_node_var *tree){
  if(*tree == NULL) return;

	bst_dispose_var(&((*tree)->left)); 
	bst_dispose_var(&((*tree)->right)); 
	delete_stack(tree);
  free((*tree)->key);
  free(*tree);
  bst_init_var(tree);
}


int bst_Get_Type_var(bst_node_var tree, char *key){
  tree = bst_search_var(tree, key);
	if(!tree)	error_out(INTERNAL_ERROR, "");

	return tree->var_stack->type;
}


void bst_scope_delete_var(bst_node_var *tree, int new_s){
  if((*tree) == NULL) return;

  bst_scope_delete_var(&((*tree)->left), new_s);
  bst_scope_delete_var(&((*tree)->right), new_s);

  while((*tree)->var_stack != NULL && (*tree)->var_stack->scope > new_s) pop_stack(tree);

  if((*tree)->var_stack == NULL) bst_delete_var(tree, (*tree)->key);
}


bool bst_Declared_var(bst_node_var tree, char *key){
  if((bst_search_var(tree, key)) == NULL) return 0;
	return 1;
}


void pop_stack(bst_node_var *tree){
  if ((*tree)->var_stack != NULL){
		Var_Stack temp = (*tree)->var_stack;
		(*tree)->var_stack = (*tree)->var_stack->Prev_Element;
		free(temp);
	}
}


void push_stack(bst_node_var *tree, int type, int scope){
  Var_Stack new = (Var_Stack) malloc(sizeof(struct Var_Stack));
	new->Prev_Element = (*tree)->var_stack;

	(*tree)->var_stack = new;
  new->type = type;
	new->scope = scope;
}



bool delete_stack(bst_node_var *tree){
  while ((*tree)->var_stack != NULL) pop_stack(tree);
  return 0;
}


void bst_init_function(bst_node_function *tree){
  *tree = NULL;
}


bst_node_function *bst_search_function(bst_node_function *tree, char *S_key){
  if ((*tree) == NULL) return tree;
  else if(strcmp((*tree)->key, S_key) < 0) return bst_search_function(&((*tree)->left), S_key);
  else if(strcmp((*tree)->key, S_key) > 0) return bst_search_function(&((*tree)->right), S_key);

  return tree;
}


void bst_insert_function(bst_node_function *tree, char *key){
  tree = bst_search_function(tree, key);

  if((*tree) != NULL) if(!(strcmp(((*tree)->key), key))) error_out(SEM_ERROR_NDEFINED, "Redefiniton of a function!");
  if((*tree) == NULL) (*tree) = (bst_node_function)malloc(sizeof(struct bst_node_function));
  if(!(*tree)) error_out(INTERNAL_ERROR, "Malloc Error!");

  Function_init_params(tree, key);
}


void bst_dispose_function(bst_node_function *tree){
  if (*tree == NULL) return;

	bst_dispose_function(&((*tree)->left)); 
	bst_dispose_function(&((*tree)->right)); 
  free((*tree)->key);
  List_Delete((*tree)->params);
  List_Delete((*tree)->return_codes);
  free(*tree);
  bst_init_function(tree);
}


void function_change(bst_node_function *tree, char *key, bool declared, bool called, int parameter_C){
  tree = bst_search_function(tree, key);

  if(!(*tree)) error_out(INTERNAL_ERROR, "");

	if(declared == 1 && ((*tree)->Declared == 1) && ((*tree)->Global_Dec == 0)) error_out(SEM_ERROR_NDEFINED, "Redefinition of function!");

	if((*tree)->params->num_of_elements != parameter_C) error_out(SEM_ERROR_FUNCTION, "Function has incorrect amount of parameters!");

	if(declared == 1 && (*tree)->Declared == 0) (*tree)->Declared = 1;

	if (called == 1 && (*tree)->Called == 0) (*tree)->Called = 1;
}


void Insert_Function_Call(bst_node_function *tree, char *key, bst_node_var varTree, int parameter_C){
  if(bst_search_var(varTree, key)) error_out(SEM_ERROR_NDEFINED, "Function has the same ID as variable!");
  function_change(tree, key, 0, 1, parameter_C);
}


void Insert_Return(bst_node_function *tree, char *key, int return_T, int return_seq){
  tree = bst_search_function(tree, key);

	if((*tree)->Called == 0 && (*tree)->Declared == 0) List_Add((*tree)->return_codes, return_T, return_seq);
  else List_Check_Element((*tree)->return_codes, return_T, return_seq);	
}


void is_function_declared(bst_node_function tree){
    if(tree == NULL) return;

    is_function_declared(tree->left);
    is_function_declared(tree->right);

    if(tree->Called == 1 && tree->Declared == 0) error_out(SEM_ERROR_NDEFINED, "Function not declared!");
}


void Function_Return_Check(bst_node_function *tree, char *key, int return_C){
  tree = bst_search_function(tree, key);
	if((*tree)->return_codes->num_of_elements != return_C) error_out(SEM_ERROR_FUNCTION, "Incorrect amount of returns in function!");
}

void Insert_Function_Declaration(bst_node_function *tree, char *key, int parameter_C){
  function_change(tree, key, 1, 0, parameter_C);
}


void Function_Compare_List(function_list *function_list, bst_node_function *tree, char *key, int return_C){
  tree = bst_search_function(tree, key);
  function_list_element n_returns = function_list->First;

  if((*tree)->Called == 0 && (*tree)->Declared == 0){
    (*tree)->return_codes->First = n_returns;
    (*tree)->return_codes->num_of_elements = function_list->num_of_elements;
  }
  else{
    function_list_element function_returns = (*tree)->return_codes->First;
    if(return_C != (*tree)->return_codes->num_of_elements) error_out(SEM_ERROR_FUNCTION, "Incorrect amount of returns in function!");
    
    while(n_returns != NULL && function_returns != NULL){
      if(n_returns->type != function_returns->type) error_out(SEM_ERROR_FUNCTION, "Incorrect type of return of a function!");
      n_returns = n_returns->Next;
      function_returns = function_returns->Next;
    }
  }
}


void List_Init(function_list *list){
  list = malloc(sizeof(struct function_list));
  if(!list) error_out(INTERNAL_ERROR, "Malloc Error!");
  list->First = NULL;
  list->num_of_elements = 0;
}


void List_Add(function_list *list, int value, int seq){
  if(list->First == NULL){
    list->First = (function_list_element)malloc(sizeof(struct function_list_element));
    if(list->First == NULL) error_out(INTERNAL_ERROR, "Malloc Error!");
    list->First->type = value;
    list->First->sequence = seq;
    list->First->Next = NULL;
    list->num_of_elements++;
    return;
  }

  function_list_element tmp = list->First;
  while(tmp->Next != NULL) tmp = tmp->Next;
  function_list_element new = (function_list_element)malloc(sizeof(struct function_list_element));
  if(new == NULL) error_out(INTERNAL_ERROR, "Malloc Error!");
  new->Next = NULL;
  new->type = value;
  new->sequence = seq;
  
  tmp->Next = new;
  list->num_of_elements++;
}


function_list_element List_Search(function_list *list, int seq){
  if(!list->First) return NULL;
    
  function_list_element tmp = list->First;

  for (int i = 1; tmp != NULL; i++){
    if (i == seq) return tmp;
    tmp = tmp->Next;
  }

  return NULL;
}


void List_Delete(function_list *list){
  function_list_element tmp = list->First;
  function_list_element backup = tmp;
  while (tmp != NULL){
      tmp = tmp->Next;
      free(backup);
      backup = tmp;
  }
}


void List_Check_Element(function_list *list, int type, int seq){
  function_list_element tmp;
  tmp = List_Search(list, seq);
        
  if(!tmp) return;   
  if(tmp->type != type) error_out(SEM_ERROR_FUNCTION, "Incorrect type of function parameter!");
}


void List_Compare(function_list *list_A, function_list *list_B){
  if(list_A->num_of_elements != list_B->num_of_elements) error_out(SEM_ERROR_FUNCTION, "Wrong number of parameters in function!");

  function_list_element tmp1 = list_A->First;
  function_list_element tmp2 = list_B->First;

  while(tmp1 != NULL && tmp2 != NULL){
    if(tmp1->type != tmp2->type) error_out(SEM_ERROR_INCOMP_TYPE, "Incorrect type of assign parameter!");
    tmp1 = tmp1->Next;
    tmp2 = tmp2->Next;
  }
}


int bst_insert_key(bst_node_var *tree, char *name){
  if((int)((strlen(name))+1) > (*tree)->key_alloc_lenght){
    int new_lenght = strlen(name)+1;
    (*tree)->key = (char *)realloc((*tree)->key, new_lenght); 
    if(!(*tree)->key) return 1;

    (*tree)->key_alloc_lenght = new_lenght;
    (*tree)->key_lenght = new_lenght-1;
    strcpy((*tree)->key, name);
    return 0;
  }
  else strcpy((*tree)->key, name);
  return 0;
}

int bst_insert_key_func(bst_node_function *tree, char *name){
  if((int)(strlen(name)+1) > (*tree)->key_alloc_lenght){
      int new_lenght = strlen(name)+1;
      (*tree)->key = (char *)realloc((*tree)->key, new_lenght); 
      if(!(*tree)->key) return 1;

      (*tree)->key_alloc_lenght = new_lenght;
      (*tree)->key_lenght = new_lenght-1;
      strcpy((*tree)->key, name);
      return 0;
  }
  
  else strcpy((*tree)->key, name);
  return 0;
}

void Function_init_params(bst_node_function *tree, char *key){
  (*tree)->params = malloc(sizeof(struct function_list));
  if(!((*tree)->params)) error_out(INTERNAL_ERROR, "Malloc Error!");
  (*tree)->params->First = NULL;
  (*tree)->params->num_of_elements = 0;

  (*tree)->return_codes = malloc(sizeof(struct function_list));
  if(!((*tree)->return_codes)) error_out(INTERNAL_ERROR, "Malloc Error!");
  (*tree)->return_codes->First = NULL;
  (*tree)->return_codes->num_of_elements = 0;

  (*tree)->key = (char *) malloc(DEFAULT_KEY_LENGHT);
  if(!((*tree)->key)) error_out(INTERNAL_ERROR, "Malloc Error!");
  bst_insert_key_func(tree, key);

  (*tree)->left = NULL;
  (*tree)->right = NULL;
  (*tree)->Declared = 0;
  (*tree)->Called = 0;
  (*tree)->Global_Dec = 0;
  return;
}

void Insert_Function_Parameter(bst_node_function *tree, char *key, int parameter_T, int parameter_seq){
  tree = bst_search_function(tree, key);

  if((*tree)->Called == 0 && (*tree)->Declared == 0) List_Add((*tree)->params, parameter_T, parameter_seq);
  else List_Check_Element((*tree)->params, parameter_T, parameter_seq);
}