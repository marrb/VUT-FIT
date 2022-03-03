/*
Project: Compiler for language IFJ21

File: parser.c

AUTHORS:
Martin Bublavý <xbubla02@stud.fit.vutbr.cz>
Igor Pszota <xpszot00@stud.fit.vutbr.cz>

*/

#include <string.h>
#include <ctype.h>
#include "statement_parser.h"

#define DEFAULT_STRING_LEN 10

Lexeme L;
bst_node_function function_tree;

string function_name;
string function_dec_name;
int function_name_alloc_len;
int function_name_len;
int Is_Return = 0;
int scope = 1;
int function_parameter_counter;
int function_return_counter;
bool global = 0;

int parse(){
if(!LS_init(&L)) return INTERNAL_ERROR;

//Get first token
int result;
result = get_token(&L);

//It the function failed return error, or if the file is empty return error
if(result != 0){
    LS_Free(&L);
    return result;
}
else if (L.T_Type == TT_End_Of_File){
    LS_Free(&L);
    return SYNTAX_ERROR;
}

//Else, check if the token is a keyword
Is_It_Keyword(&L);
//Check if the source code has a header, if not return an error
result = Find_Header();
if(result != 0) error_out(SYNTAX_ERROR, "Header missing!");

bst_init_function(&function_tree);
init_string(&function_name);
init_string(&function_dec_name);
function_def_list();

LS_Free(&L);
free_string(&function_name);
free_string(&function_dec_name);

return 0;
}

void skip_whitespaces(){
    while(L.T_Type == TT_End_Of_Line || L.T_Type == TT_Comment || L.T_Type == TT_Block_Comment){
        if(L.T_Type == TT_End_Of_Line){
                get_token(&L);
        }
        else if(L.T_Type == TT_Comment){
            while(L.T_Type != TT_End_Of_Line){
                get_token(&L);
            }
        }
        else{
            while(L.T_Type != TT_Block_Comment_End){
                get_token(&L);
            }
            get_token(&L);
        }
    }
}

int Find_Header(){
    int ERR_CODE;
    if(L.keyword == K_require){
        ERR_CODE = get_token(&L);
        skip_whitespaces();
        if(ERR_CODE != 0) return ERR_CODE;
        if(!strcmp(L.string, "ifj21") && L.T_Type == TT_String) return 0;
        else return SYNTAX_ERROR;
    }
    else{
        skip_whitespaces();
        return Find_Header();
    }
    return 0;
}

int function_def_list(){
   int result = 0;
   bst_node_function *tmp;
   function_list *param_list;
   int param_counter = 0;
    
    if(global == 0){
        get_token(&L); 
        skip_whitespaces();
    }
    else if(global == 1){
        global = 0;
    }

   switch(L.T_Type){
        case(TT_End_Of_File):
            is_function_declared(function_tree);
            return result;
            break;

        case(TT_Keyword):
            if(L.keyword == K_global){
                global_func_def();
                return function_def_list();
            }
            if(L.keyword == K_function){
                result = function_def();
                if(result != 0){
                    return result;
                }

                if(Is_Return == 1){
                    error_out(SEM_ERROR_FUNCTION, "Function is missing return!");
                }
                return function_def_list();
            }
            break;

        case(TT_Identifier):
            tmp = bst_search_function(&function_tree, L.string);
            param_list = malloc(sizeof(struct function_list));
            param_list->First = NULL;
            param_list->num_of_elements = 0;
            if(!(*tmp)){
                error_out(SEM_ERROR_NDEFINED, "Function not defined!");
            }
            get_token(&L);
            skip_whitespaces();
            if(L.T_Type != TT_Left_Bracket) error_out(SYNTAX_ERROR, "Function missing '('!");
            while(L.T_Type != TT_Right_Bracket){
                get_token(&L);
                skip_whitespaces();
                if(L.T_Type != TT_Integer && L.T_Type != TT_String && L.T_Type != TT_Double && L.T_Type != TT_Right_Bracket) error_out(SEM_ERROR_FUNCTION, "Invalid parameter in function call!");
                if(L.T_Type == TT_Right_Bracket) break;
                if(L.T_Type == TT_Integer) List_Add(param_list, K_integer, param_counter);
                else if(L.T_Type == TT_String) List_Add(param_list, K_string, param_counter);
                else if(L.T_Type == TT_Double) List_Add(param_list, K_number, param_counter);
                param_counter++;
                get_token(&L);
                skip_whitespaces();
                if(L.T_Type != TT_Comma && L.T_Type != TT_Right_Bracket) error_out(SEM_ERROR_FUNCTION, "Invalid parameter in function call!");
                if(L.T_Type == TT_Right_Bracket){
                    break;
                }
            }
            List_Compare(param_list, (*tmp)->params);
            free(param_list);

            return function_def_list();

            
        default:
            break;
   }

   error_out(SYNTAX_ERROR, "Not a function!");
   return SYNTAX_ERROR;
}

int function_def(){
    int result = 0;

    get_token(&L);
    skip_whitespaces();

    if(L.T_Type == TT_Identifier){
        string_clear(&function_name);
        copy_string(&function_name, L.string);
        bst_node_function *tmp = bst_search_function(&function_tree, function_name.str);

        if(!(*tmp)){
            bst_insert_function(&function_tree, function_name.str);
        }

        string_clear(&function_dec_name);
        copy_string(&function_dec_name, L.string);


        get_token(&L);
        skip_whitespaces();
        if(L.T_Type != TT_Left_Bracket) error_out(SYNTAX_ERROR, "Function missing '('");

        bst_node_var var_tree;
        bst_init_var(&var_tree);

        function_parameter_counter = 0;
        function_return_counter = 0;

        result = function_params(&var_tree);
        if(result != 0) return result;

        if(L.T_Type != TT_Right_Bracket) error_out(SYNTAX_ERROR, "Function missing ')'");
        get_token(&L);
        skip_whitespaces();

        if(L.T_Type == TT_Colon){
            function_return_counter = 0;

            result = function_returns();
            if(result != 0) return result;
           
            Function_Return_Check(&function_tree, function_name.str, function_return_counter);
            if(function_return_counter != 0) Is_Return = 1;
        }

        Insert_Function_Declaration(&function_tree, function_name.str, function_parameter_counter);
        result = statement_list_rules(&var_tree);
        if(result != 0) return result;

        if(L.keyword != K_end) error_out(SYNTAX_ERROR, "Function missing end!");
        bst_dispose_var(&var_tree);
    }
    else if(L.T_Type == TT_PreDefined_Func){
        error_out(SEM_ERROR_NDEFINED, "Redefinition of function!");
    }
    else{
        error_out(SYNTAX_ERROR, "Missing ID in function!");
    }
    return result;
}

int statement_list_rules(bst_node_var *tree){
    int result = 0;
    if(L.T_Type == TT_End_Of_Line){
        get_token(&L);
        skip_whitespaces();
    }
    if(L.T_Type == TT_Identifier || L.keyword == K_if || L.keyword == K_return || L.T_Type == TT_PreDefined_Func ||  L.keyword == K_integer || L.keyword == K_number || L.keyword == K_string || L.keyword == K_local || L.keyword == K_global || L.keyword == K_while){
            result = statement(tree);
            if(result != 0) return result;

            return statement_list_rules(tree);
    }
    else if(L.keyword == K_end){
        return result;
    }
    else if(L.keyword == K_else) return 0;
    else{
        error_out(SYNTAX_ERROR, "Incorrect statement!");
        return result;
    }
}

int statement(bst_node_var *var_tree){
    int result = 0;

    if(L.keyword == K_if){
        scope++;
        get_token(&L);
        skip_whitespaces();

        if(L.T_Type != TT_Integer && L.T_Type != TT_String && L.T_Type != TT_Double && L.T_Type != TT_Identifier && L.T_Type != TT_Left_Bracket && L.T_Type != TT_Hashtag){
            error_out(SYNTAX_ERROR, "Incorrect token following if statement!");
        }

        statement_end_type stat = statement_parse(var_tree, &L);
        if(stat.type != BOOL) error_out(SEM_ERROR_OTHER, "Statement in 'if' needs to be a boolean!");

        if(L.keyword != K_then) error_out(SYNTAX_ERROR, "If is missing then!");
        get_token(&L);
        skip_whitespaces();

        result = statement_list_rules(var_tree);
        if(result != 0) return result;

        scope--;
        bst_scope_delete_var(var_tree, scope);

        if(L.keyword != K_else) error_out(SYNTAX_ERROR, "If is missing else!");

        scope++;
        get_token(&L);
        skip_whitespaces();
        
        result = statement_list_rules(var_tree);
        if(result != 0) return result;

        if(L.keyword != K_end) error_out(SYNTAX_ERROR, "If is missing end!");

        scope--;
        bst_scope_delete_var(var_tree, scope);
        

        get_token(&L);
        skip_whitespaces();
    }
    else if(L.keyword == K_while){
        scope++;
        get_token(&L);
        skip_whitespaces();

        if(L.T_Type != TT_Integer && L.T_Type != TT_String && L.T_Type != TT_Double && L.T_Type != TT_Identifier && L.T_Type != TT_Left_Bracket && L.T_Type != TT_Hashtag){
            error_out(SYNTAX_ERROR, "Incorrect token following while statement!");
        }

        statement_end_type stat = statement_parse(var_tree, &L);
        if(stat.type != BOOL) error_out(SEM_ERROR_OTHER, "Statement in 'while' needs to be a boolean!");

        if(L.keyword != K_do) error_out(SYNTAX_ERROR, "While is missing do!");
        get_token(&L);
        skip_whitespaces();

        result = statement_list_rules(var_tree);
        if(result != 0) return result;

        scope--;
        bst_scope_delete_var(var_tree, scope);
        if(L.keyword != K_end) error_out(SYNTAX_ERROR, "While is missing end!");

        get_token(&L);
        //if(L.T_Type != TT_End_Of_Line) error_out(SYNTAX_ERROR, "Unexpected symbol after end!");
        skip_whitespaces();
    }
    else if(L.keyword == K_local || L.keyword == K_global){
        int b_scope;
        bool global = 0;
        if(L.keyword == K_global){
            global = 1;
            b_scope = scope;
            scope = 0;
        }

        get_token(&L);
        skip_whitespaces();

        if(L.T_Type != TT_Identifier) error_out(SYNTAX_ERROR, "Invalid token following local/global!");
        
        string var_name;
        init_string(&var_name);
        copy_string(&var_name, L.string);

        get_token(&L);
        skip_whitespaces();

        if(L.T_Type != TT_Colon) error_out(SYNTAX_ERROR, "Unexpected symbol following ID!");
        get_token(&L);
        skip_whitespaces();
        
        if(L.keyword != K_integer && L.keyword != K_string && L.keyword != K_number) error_out(SYNTAX_ERROR, "Unexpected token following ':' in variable definition!");
        bst_insert_var(var_tree, var_name.str, L.keyword, scope);

        get_token(&L);
        skip_whitespaces();
        if(L.T_Type == TT_Assign){
           get_token(&L);
           skip_whitespaces();

           if(L.T_Type == TT_Identifier || L.T_Type == TT_Integer || L.T_Type == TT_String || L.T_Type == TT_Double || L.T_Type == TT_Hashtag || L.T_Type == TT_Left_Bracket){
               if(L.T_Type == TT_Identifier){
                    bst_node_function *tmp_F = bst_search_function(&function_tree, L.string);
                    if(!(*tmp_F)){
                        statement_end_type stat;
                        stat = statement_parse(var_tree, &L);

                        if(stat.type != bst_Get_Type_var(*var_tree, var_name.str)) error_out(SEM_ERROR_INCOMP_TYPE, "Invalid type for assign!");
                        return result;
                    }
                    else{
                        string tmp;
                        init_string(&tmp);
                        copy_string(&tmp, L.string);

                        get_token(&L);
                        skip_whitespaces();
                        if(L.T_Type != TT_Left_Bracket){
                            error_out(SYNTAX_ERROR, "Function missing '('!");
                        }
                        else if(L.T_Type == TT_Left_Bracket){
                            function_parameter_counter = 0;
                            copy_string(&function_name, tmp.str);

                            result = function_call_parameters(var_tree);
                            if(result != 0) return result;

                            Function_Return_Check(&function_tree, function_name.str, 1);
                            if((*tmp_F)->return_codes->First->type != bst_Get_Type_var(*var_tree, var_name.str)) error_out(SEM_ERROR_INCOMP_TYPE, "Incompatible types in assign!");
                            get_token(&L);
                            skip_whitespaces();

                            return result;
                        }
                    }
               }
               else{
                    statement_end_type stat;
                    stat = statement_parse(var_tree, &L);

                    if(stat.type != bst_Get_Type_var(*var_tree, var_name.str)) error_out(SEM_ERROR_INCOMP_TYPE, "Invalid type for assign!");
                    return result;
               }
            }
            else if(!strcmp(L.string, "readi") || !strcmp(L.string, "reads") || !strcmp(L.string, "readn")){
                int var_type = bst_Get_Type_var(*var_tree, var_name.str);
                if(!strcmp(L.string, "readi")){
                    if(var_type != K_integer) error_out(SEM_ERROR_INCOMP_TYPE, "Statement incompatible type!");
                }
                else if(!strcmp(L.string, "reads")){ 
                    if(var_type != K_string) error_out(SEM_ERROR_INCOMP_TYPE, "Statement incompatible type!");
                }
                else if (!strcmp(L.string, "readn")){
                    if(var_type != K_number) error_out(SEM_ERROR_INCOMP_TYPE, "Statement incompatible type!");
                }

                get_token(&L);
                skip_whitespaces();
                if(L.T_Type != TT_Left_Bracket) error_out(SYNTAX_ERROR, "Function read missing '('!");
                get_token(&L);
                skip_whitespaces();
                if(L.T_Type == TT_Identifier || L.T_Type == TT_Integer || L.T_Type == TT_Double || L.T_Type == TT_String) error_out(SEM_ERROR_FUNCTION, "Function read doesnt need parameters!");
                else if(L.T_Type != TT_Right_Bracket) error_out(SYNTAX_ERROR, "Function read missing ')'!");
                get_token(&L);
                skip_whitespaces();
                //if(L.T_Type != TT_End_Of_Line) error_out(SYNTAX_ERROR, "Function read missing EOL!");
                return result;
            }
            else if(!strcmp(L.string, "write")) error_out(SEM_ERROR_NDEFINED, "Write cannot be uset in assign!");
            else if(!strcmp(L.string, "tointeger")){
                int ID_TYPE = bst_Get_Type_var(*var_tree, var_name.str);

                get_token(&L);
                skip_whitespaces();
                if(L.T_Type != TT_Left_Bracket) error_out(SYNTAX_ERROR, "Function missing '(");

                get_token(&L);
                skip_whitespaces();
                if(L.T_Type != TT_Identifier && L.T_Type != TT_Double && L.T_Type != TT_Integer) error_out(SEM_ERROR_FUNCTION, "Tointeger incompatible type!");

                if(L.T_Type == TT_Identifier){
                    bool Is_Declared = bst_Declared_var(*var_tree, L.string);
                    if(Is_Declared == 0) error_out(SEM_ERROR_NDEFINED, "Variable not declared!");

                    int var_type = bst_Get_Type_var(*var_tree, L.string);
                    if(var_type != K_number && var_type != K_integer) error_out(SEM_ERROR_FUNCTION, "tointeger - varaible must be float!");
                }

                get_token(&L);
                skip_whitespaces();
                if(L.T_Type != TT_Right_Bracket) error_out(SYNTAX_ERROR, "Function missing ')'!");

                get_token(&L);
                skip_whitespaces();
                //if(L.T_Type != TT_End_Of_Line) error_out(SYNTAX_ERROR, "Missing end of line!");

                if(ID_TYPE != K_integer) error_out(SEM_ERROR_INCOMP_TYPE, "tointeger - Incorrect variable type!");
                return result;
            }
            else if(!strcmp(L.string, "chr")){
                int var_type = bst_Get_Type_var(*var_tree, var_name.str);
                if(var_type != K_string) error_out(SEM_ERROR_INCOMP_TYPE, "Statement incompatible type (function chr)!");

                get_token(&L);
                skip_whitespaces();
                if(L.T_Type != TT_Left_Bracket) error_out(SYNTAX_ERROR, "Function chr missing '('!");
                get_token(&L);
                skip_whitespaces();
                if(L.T_Type != TT_Identifier && L.T_Type != TT_Integer) error_out(SEM_ERROR_FUNCTION, "Incorrect type of variable in function chr!");
                if(L.T_Type == TT_Identifier){
                    var_type = bst_Get_Type_var(*var_tree, L.string);
                    if(var_type != K_integer) error_out(SEM_ERROR_FUNCTION, "Incorrect type of variable in function chr!");
                }
                get_token(&L);
                skip_whitespaces();
                if(L.T_Type != TT_Right_Bracket) error_out(SYNTAX_ERROR, "Function chr missing ')'!");
                get_token(&L);
                skip_whitespaces();
                //if(L.T_Type != TT_End_Of_Line) error_out(SYNTAX_ERROR, "Function chr missing EOL!");
                return result;
            }
            else if(!strcmp(L.string, "ord")){
                int var_type = bst_Get_Type_var(*var_tree, var_name.str);
                if(var_type != K_integer) error_out(SEM_ERROR_INCOMP_TYPE, "Statement incompatible type (function ord)!");
                get_token(&L);
                skip_whitespaces();
                if(L.T_Type != TT_Left_Bracket) error_out(SYNTAX_ERROR, "Function ord missing '('!");
                get_token(&L);
                skip_whitespaces();
                if(L.T_Type != TT_Identifier && L.T_Type != TT_String) error_out(SEM_ERROR_FUNCTION, "Incorrect type of variable in function ord!");
                if(L.T_Type == TT_Identifier){
                    var_type = bst_Get_Type_var(*var_tree, L.string);
                    if(var_type != K_string) error_out(SEM_ERROR_FUNCTION, "Incorrect type of variable in function ord!");
                }
                get_token(&L);
                skip_whitespaces();
                if(L.T_Type != TT_Comma) error_out(SEM_ERROR_FUNCTION, "Missing comma in function call (ord)!");
                get_token(&L);
                skip_whitespaces();
                if(L.T_Type != TT_Identifier && L.T_Type != TT_Integer) error_out(SEM_ERROR_FUNCTION, "Incorrect type of variable in function ord!");
                if(L.T_Type == TT_Identifier){
                    var_type = bst_Get_Type_var(*var_tree, L.string);
                    if(var_type != K_integer) error_out(SEM_ERROR_FUNCTION, "Incorrect type of variable in function ord!");
                }
                get_token(&L);
                skip_whitespaces();
                if(L.T_Type != TT_Right_Bracket) error_out(SYNTAX_ERROR, "Function ord missing ')'!");
            }
            else if(!strcmp(L.string, "substr")){
                int var_type = bst_Get_Type_var(*var_tree, var_name.str);
                if(var_type != K_string) error_out(SEM_ERROR_INCOMP_TYPE, "Statement incompatible type (function substr)!");
                get_token(&L);
                skip_whitespaces();
                if(L.T_Type != TT_Left_Bracket) error_out(SYNTAX_ERROR, "Function substr missing '('!");
                get_token(&L);
                skip_whitespaces();
                if(L.T_Type != TT_Identifier && L.T_Type != TT_String) error_out(SEM_ERROR_FUNCTION, "Incorrect type of variable in function substr!");
                if(L.T_Type == TT_Identifier){
                    var_type = bst_Get_Type_var(*var_tree, L.string);
                    if(var_type != K_string) error_out(SEM_ERROR_FUNCTION, "Incorrect type of variable in function substr!");
                }
                get_token(&L);
                skip_whitespaces();
                if(L.T_Type != TT_Comma) error_out(SEM_ERROR_FUNCTION, "Missing comma in function call (substr)!");
                get_token(&L);
                skip_whitespaces();
                if(L.T_Type != TT_Identifier && L.T_Type != TT_Double && L.T_Type != TT_Integer) error_out(SEM_ERROR_FUNCTION, "Incorrect type of variable in function substr!");
                if(L.T_Type == TT_Identifier){
                    var_type = bst_Get_Type_var(*var_tree, L.string);
                    if(var_type != K_number && var_type != K_integer) error_out(SEM_ERROR_FUNCTION, "Incorrect type of variable in function substr!");
                }
                get_token(&L);
                skip_whitespaces();
                if(L.T_Type != TT_Comma) error_out(SYNTAX_ERROR, "Missing comma in function call (substr)!");
                get_token(&L);
                skip_whitespaces();
                if(L.T_Type != TT_Identifier && L.T_Type != TT_Double && L.T_Type != TT_Integer) error_out(SEM_ERROR_FUNCTION, "Incorrect type of variable in function substr!");
                if(L.T_Type == TT_Identifier){
                    var_type = bst_Get_Type_var(*var_tree, L.string);
                    if(var_type != K_number && var_type != K_integer) error_out(SEM_ERROR_FUNCTION, "Incorrect type of variable in function substr!");
                }
                get_token(&L);
                skip_whitespaces();
                if(L.T_Type != TT_Right_Bracket) error_out(SYNTAX_ERROR, "Function ord missing ')'!");
                get_token(&L);
                skip_whitespaces();
                //if(L.T_Type != TT_End_Of_Line) error_out(SYNTAX_ERROR, "Function ord missing EOL!");
                return result;
            }
        }
        if(global == 1) scope = b_scope;
    }
    else if(L.T_Type == TT_Identifier){
        bool ID_Declared = bst_Declared_var(*var_tree, L.string);
        if(ID_Declared == 0) error_out(SEM_ERROR_NDEFINED, "Variable not declared!");
        string var_name;
        init_string(&var_name);
        copy_string(&var_name, L.string);

        get_token(&L);
        skip_whitespaces();
        if(L.T_Type != TT_Comma && L.T_Type != TT_Assign && L.T_Type != TT_Left_Bracket) error_out(SYNTAX_ERROR, "Unexpected symbol following ID!");

        if(L.T_Type == TT_Assign){
            get_token(&L);
            skip_whitespaces();

            if(!strcmp(L.string, "write")) error_out(SEM_ERROR_OTHER, "Write cannot be used in assign!");

            if(!strcmp(L.string, "tointeger")){
                int ID_TYPE = bst_Get_Type_var(*var_tree, var_name.str);

                get_token(&L);
                skip_whitespaces();
                if(L.T_Type != TT_Left_Bracket) error_out(SYNTAX_ERROR, "Function missing '(");

                get_token(&L);
                skip_whitespaces();
                if(L.T_Type != TT_Identifier && L.T_Type != TT_Double && L.T_Type != TT_Integer) error_out(SEM_ERROR_FUNCTION, "Tointeger incompatible type!");

                if(L.T_Type == TT_Identifier){
                    bool Is_Declared = bst_Declared_var(*var_tree, L.string);
                    if(Is_Declared == 0) error_out(SEM_ERROR_NDEFINED, "Variable not declared!");

                    int var_type = bst_Get_Type_var(*var_tree, L.string);
                    if(var_type != K_number && var_type != K_integer) error_out(SEM_ERROR_FUNCTION, "tointeger - variable must be float!");
                }

                get_token(&L);
                skip_whitespaces();
                if(L.T_Type != TT_Right_Bracket) error_out(SYNTAX_ERROR, "Function missing ')'!");

                get_token(&L);
                skip_whitespaces();
                //if(L.T_Type != TT_End_Of_Line) error_out(SYNTAX_ERROR, "Missing end of line!");

                if(ID_TYPE != K_integer) error_out(SEM_ERROR_INCOMP_TYPE, "tointeger - Incorrect variable type!");
                return result;
            }
            if(!strcmp(L.string, "readi") || !strcmp(L.string, "reads") || !strcmp(L.string, "readn")){
                    int var_type = bst_Get_Type_var(*var_tree, var_name.str);
                    if(!strcmp(L.string, "readi")){
                         if(var_type != K_integer) error_out(SEM_ERROR_INCOMP_TYPE, "Statement incompatible type!");
                         }
                    else if(!strcmp(L.string, "reads")){ 
                        if(var_type != K_string) error_out(SEM_ERROR_INCOMP_TYPE, "Statement incompatible type!");
                    }
                    else if (!strcmp(L.string, "readn")){
                         if(var_type != K_number) error_out(SEM_ERROR_INCOMP_TYPE, "Statement incompatible type!");
                    }

                    get_token(&L);
                    skip_whitespaces();
                    if(L.T_Type != TT_Left_Bracket) error_out(SYNTAX_ERROR, "Function read missing '('!");
                    get_token(&L);
                    skip_whitespaces();
                    if(L.T_Type == TT_Identifier || L.T_Type == TT_Integer || L.T_Type == TT_Double || L.T_Type == TT_String) error_out(SEM_ERROR_FUNCTION, "Function read doesnt need parameters!");
                    else if(L.T_Type != TT_Right_Bracket) error_out(SYNTAX_ERROR, "Function read missing ')'!");
                    get_token(&L);
                    skip_whitespaces();
                    //if(L.T_Type != TT_End_Of_Line) error_out(SYNTAX_ERROR, "Function read missing EOL!");
                    return result;
            }
            else if(!strcmp(L.string, "chr")){
                int var_type = bst_Get_Type_var(*var_tree, var_name.str);
                if(var_type != K_string) error_out(SEM_ERROR_INCOMP_TYPE, "Statement incompatible type (function chr)!");

                get_token(&L);
                skip_whitespaces();
                if(L.T_Type != TT_Left_Bracket) error_out(SYNTAX_ERROR, "Function chr missing '('!");
                get_token(&L);
                skip_whitespaces();
                if(L.T_Type != TT_Identifier && L.T_Type != TT_Integer) error_out(SEM_ERROR_FUNCTION, "Incorrect type of variable in function chr!");
                if(L.T_Type == TT_Identifier){
                    var_type = bst_Get_Type_var(*var_tree, L.string);
                    if(var_type != K_integer) error_out(SEM_ERROR_FUNCTION, "Incorrect type of variable in function chr!");
                }
                get_token(&L);
                skip_whitespaces();
                if(L.T_Type != TT_Right_Bracket) error_out(SYNTAX_ERROR, "Function chr missing ')'!");
                get_token(&L);
                skip_whitespaces();
                //if(L.T_Type != TT_End_Of_Line) error_out(SYNTAX_ERROR, "Function chr missing EOL!");
                return result;
            }
            else if(!strcmp(L.string, "ord")){
                int var_type = bst_Get_Type_var(*var_tree, var_name.str);
                if(var_type != K_integer) error_out(SEM_ERROR_INCOMP_TYPE, "Statement incompatible type (function ord)!");
                get_token(&L);
                skip_whitespaces();
                if(L.T_Type != TT_Left_Bracket) error_out(SYNTAX_ERROR, "Function ord missing '('!");
                get_token(&L);
                skip_whitespaces();
                if(L.T_Type != TT_Identifier && L.T_Type != TT_String) error_out(SEM_ERROR_FUNCTION, "Incorrect type of variable in function ord!");
                if(L.T_Type == TT_Identifier){
                    var_type = bst_Get_Type_var(*var_tree, L.string);
                    if(var_type != K_string) error_out(SEM_ERROR_FUNCTION, "Incorrect type of variable in function ord!");
                }
                get_token(&L);
                skip_whitespaces();
                if(L.T_Type != TT_Comma) error_out(SEM_ERROR_FUNCTION, "Missing comma in function call (ord)!");
                get_token(&L);
                skip_whitespaces();
                if(L.T_Type != TT_Identifier && L.T_Type != TT_Integer) error_out(SEM_ERROR_FUNCTION, "Incorrect type of variable in function ord!");
                if(L.T_Type == TT_Identifier){
                    var_type = bst_Get_Type_var(*var_tree, L.string);
                    if(var_type != K_integer) error_out(SEM_ERROR_FUNCTION, "Incorrect type of variable in function ord!");
                }
                get_token(&L);
                skip_whitespaces();
                if(L.T_Type != TT_Right_Bracket) error_out(SYNTAX_ERROR, "Function ord missing ')'!");
                get_token(&L);
                skip_whitespaces();
                //if(L.T_Type != TT_End_Of_Line) error_out(SYNTAX_ERROR, "Function ord missing EOL!");
                return result;
            }
            else if(!strcmp(L.string, "substr")){
                int var_type = bst_Get_Type_var(*var_tree, var_name.str);
                if(var_type != K_string) error_out(SEM_ERROR_INCOMP_TYPE, "Statement incompatible type (function substr)!");
                get_token(&L);
                skip_whitespaces();
                if(L.T_Type != TT_Left_Bracket) error_out(SYNTAX_ERROR, "Function substr missing '('!");
                get_token(&L);
                skip_whitespaces();
                if(L.T_Type != TT_Identifier && L.T_Type != TT_String) error_out(SEM_ERROR_FUNCTION, "Incorrect type of variable in function substr!");
                if(L.T_Type == TT_Identifier){
                    var_type = bst_Get_Type_var(*var_tree, L.string);
                    if(var_type != K_string) error_out(SEM_ERROR_FUNCTION, "Incorrect type of variable in function substr!");
                }
                get_token(&L);
                skip_whitespaces();
                if(L.T_Type != TT_Comma) error_out(SEM_ERROR_FUNCTION, "Missing comma in function call (substr)!");
                get_token(&L);
                skip_whitespaces();
                if(L.T_Type != TT_Identifier && L.T_Type != TT_Double && L.T_Type != TT_Integer) error_out(SEM_ERROR_FUNCTION, "Incorrect type of variable in function substr!");
                if(L.T_Type == TT_Identifier){
                    var_type = bst_Get_Type_var(*var_tree, L.string);
                    if(var_type != K_number && var_type != K_integer) error_out(SEM_ERROR_FUNCTION, "Incorrect type of variable in function substr!");
                }
                get_token(&L);
                skip_whitespaces();
                if(L.T_Type != TT_Comma) error_out(SEM_ERROR_FUNCTION, "Missing comma in function call (substr)!");
                get_token(&L);
                skip_whitespaces();
                if(L.T_Type != TT_Identifier && L.T_Type != TT_Double && L.T_Type != TT_Integer) error_out(SEM_ERROR_FUNCTION, "Incorrect type of variable in function substr!");
                if(L.T_Type == TT_Identifier){
                    var_type = bst_Get_Type_var(*var_tree, L.string);
                    if(var_type != K_number && var_type != K_integer) error_out(SEM_ERROR_FUNCTION, "Incorrect type of variable in function substr!");
                }
                get_token(&L);
                skip_whitespaces();
                if(L.T_Type != TT_Right_Bracket) error_out(SYNTAX_ERROR, "Function substr missing ')'!");
                get_token(&L);
                skip_whitespaces();
                //if(L.T_Type != TT_End_Of_Line) error_out(SYNTAX_ERROR, "Function substr missing EOL!");
                return result;
            }
            else if(L.T_Type != TT_Integer && L.T_Type != TT_String && L.T_Type != TT_Double && L.T_Type != TT_Identifier && L.T_Type != TT_Left_Bracket && L.T_Type != TT_PreDefined_Func && L.T_Type != TT_Hashtag){
                error_out(SEM_ERROR_INCOMP_TYPE, "Incompatible type after '='!");
            }

            if(L.T_Type == TT_Identifier || L.T_Type == TT_Integer || L.T_Type == TT_String || L.T_Type == TT_Double || L.T_Type == TT_Hashtag || L.T_Type == TT_Left_Bracket){
                if(L.T_Type == TT_Identifier){
                    bst_node_function *tmp_F = bst_search_function(&function_tree, L.string);
                    if(!(*tmp_F)){
                        statement_end_type stat;
                        stat = statement_parse(var_tree, &L);

                        if(stat.type != bst_Get_Type_var(*var_tree, var_name.str)) error_out(SEM_ERROR_INCOMP_TYPE, "Invalid type for assign!");
                        return result;
                    }
                    else{
                        string tmp;
                        init_string(&tmp);
                        copy_string(&tmp, L.string);

                        get_token(&L);
                        skip_whitespaces();
                        if(L.T_Type != TT_Left_Bracket){
                            error_out(SYNTAX_ERROR, "Function missing '('!");
                        }
                        else if(L.T_Type == TT_Left_Bracket){
                            function_parameter_counter = 0;
                            copy_string(&function_name, tmp.str);

                            result = function_call_parameters(var_tree);
                            if(result != 0) return result;

                            Function_Return_Check(&function_tree, function_name.str, 1);
                            if((*tmp_F)->return_codes->First->type != bst_Get_Type_var(*var_tree, var_name.str)) error_out(SEM_ERROR_INCOMP_TYPE, "Incompatible types in assign!");
                            get_token(&L);
                            skip_whitespaces();

                            //if(L.T_Type != TT_End_Of_Line) error_out(SYNTAX_ERROR, "EOL missing in return statement!");
                            return result;
                        }
                    }
               }
               else{
                    statement_end_type stat;
                    stat = statement_parse(var_tree, &L);

                    if(stat.type != bst_Get_Type_var(*var_tree, var_name.str)) error_out(SEM_ERROR_INCOMP_TYPE, "Invalid type for assign!");
                    return result;
               }
            }
        }
        else if(L.T_Type == TT_Comma){
            function_list *assign_vars_list;
            assign_vars_list = malloc(sizeof(struct function_list));
            assign_vars_list->First = NULL;
            assign_vars_list->num_of_elements = 0;

            int Var_Counter = 1;
            int Var_Type = bst_Get_Type_var(*var_tree, var_name.str);
            List_Add(assign_vars_list, Var_Type, Var_Counter);

            return assign_left(var_tree, assign_vars_list, Var_Counter);
        }
    }
    else if(L.keyword == K_return){
        function_return_counter = 0;
        Is_Return = 0;
        return return_values(var_tree);
    }
    else if(!strcmp(L.string, "reads") || !strcmp(L.string, "readn") || !strcmp(L.string, "readf") || !strcmp(L.string, "tointeger") || !strcmp(L.string, "ord") ||
            !strcmp(L.string, "substr") || !strcmp(L.string, "chr")){
        error_out(SEM_ERROR_OTHER, "Statement error!");
    }
    else if(!strcmp(L.string, "write")){
        get_token(&L);
        skip_whitespaces();
        if(L.T_Type != TT_Left_Bracket) error_out(SYNTAX_ERROR, "Write function missing '('!");

        result = write_parameters(var_tree);
        if(result != 0) return result;
    }

    return result;
}

int assign_left(bst_node_var *var_tree, function_list *assign_var_list, int Var_Counter){
    int result = 0;

    get_token(&L);
    skip_whitespaces();
    if(L.T_Type != TT_Identifier) error_out(SYNTAX_ERROR, "Statement must be ID!");

    Var_Counter++;

    bool ID_Declared = bst_Declared_var(*var_tree, L.string);
    if(ID_Declared == 0) error_out(SEM_ERROR_NDEFINED, "Variable not declared!");
    int V_Type = bst_Get_Type_var(*var_tree, L.string);
    List_Add(assign_var_list, V_Type, Var_Counter);

    get_token(&L);
    skip_whitespaces();
    if(L.T_Type != TT_Comma && L.T_Type != TT_Assign) error_out(SYNTAX_ERROR, "Assing statement error!");

    if(L.T_Type == TT_Comma) return assign_left(var_tree, assign_var_list, Var_Counter);

    function_list *ass_right_list;
    ass_right_list = malloc(sizeof(struct function_list));
    ass_right_list->First = NULL;
    ass_right_list->num_of_elements = 0;

    int ass_right_counter = 0;
    return assign_right(var_tree, assign_var_list, Var_Counter, ass_right_list, ass_right_counter);

    List_Delete(assign_var_list);
    List_Delete(ass_right_list);
    return result;
}


int assign_right(bst_node_var *var_tree, function_list *assign_var_list, int Var_Counter, function_list *assign_assign_list, int assign_assign_counter){
    int result = 0;

    get_token(&L);
    skip_whitespaces();

    if(!strcmp(L.string, "write")) error_out(SEM_ERROR_INCOMP_TYPE, "Write cannot be used in assign!");

    if(!strcmp(L.string, "readi")){
        get_token(&L);
        skip_whitespaces();
        if(L.T_Type != TT_Left_Bracket) error_out(SYNTAX_ERROR, "Statement missing '('!");

        get_token(&L);
        skip_whitespaces();
        if(L.T_Type != TT_Right_Bracket) error_out(SYNTAX_ERROR, "Statement missing ')'!");

        get_token(&L);
        skip_whitespaces();
        //if(L.T_Type != TT_End_Of_Line) error_out(SYNTAX_ERROR, "Statement missing EOL!");

        assign_assign_counter++;
        List_Add(assign_assign_list, K_integer, assign_assign_counter);

        get_token(&L);
        skip_whitespaces();
        if(L.T_Type == TT_Comma) return assign_right(var_tree, assign_var_list, Var_Counter, assign_assign_list, assign_assign_counter);
        List_Compare(assign_var_list, assign_assign_list);
        return result;
    }
    else if(!strcmp(L.string, "readf")){
        get_token(&L);
        skip_whitespaces();
        if(L.T_Type != TT_Left_Bracket) error_out(SYNTAX_ERROR, "Statement missing '('!");

        get_token(&L);
        skip_whitespaces();
        if(L.T_Type != TT_Right_Bracket) error_out(SYNTAX_ERROR, "Statement missing ')'!");

        get_token(&L);
        skip_whitespaces();
        //if(L.T_Type != TT_End_Of_Line) error_out(SYNTAX_ERROR, "Statement missing EOL!");

        assign_assign_counter++;
        List_Add(assign_assign_list, K_number, assign_assign_counter);

        get_token(&L);
        skip_whitespaces();
        if(L.T_Type == TT_Comma) return assign_right(var_tree, assign_var_list, Var_Counter, assign_assign_list, assign_assign_counter);
        List_Compare(assign_var_list, assign_assign_list);
        return result;
    }
    else if(!strcmp(L.string, "reads")){
        get_token(&L);
        skip_whitespaces();
        if(L.T_Type != TT_Left_Bracket) error_out(SYNTAX_ERROR, "Statement missing '('!");

        get_token(&L);
        skip_whitespaces();
        if(L.T_Type != TT_Right_Bracket) error_out(SYNTAX_ERROR, "Statement missing ')'!");

        get_token(&L);
        skip_whitespaces();
        //if(L.T_Type != TT_End_Of_Line) error_out(SYNTAX_ERROR, "Statement missing EOL!");

        assign_assign_counter++;
        List_Add(assign_assign_list, K_string, assign_assign_counter);

        get_token(&L);
        skip_whitespaces();
        if(L.T_Type == TT_Comma) return assign_right(var_tree, assign_var_list, Var_Counter, assign_assign_list, assign_assign_counter);
        List_Compare(assign_var_list, assign_assign_list);
        return result;
    }
    else if(!strcmp(L.string, "substr")){
        string s_substr;

        get_token(&L);
        skip_whitespaces();
        if(L.T_Type != TT_Left_Bracket) error_out(SYNTAX_ERROR, "Statement missing '('!");

        get_token(&L);
        skip_whitespaces();
        if(L.T_Type != TT_Identifier || L.T_Type != TT_String) error_out(SEM_ERROR_FUNCTION, "substr function - invalid token!");

        if(L.T_Type == TT_Identifier){
            bool ID_Declared = bst_Declared_var(*var_tree, L.string);
            if(ID_Declared == 0) error_out(SEM_ERROR_NDEFINED, "Substr is not declared!");

            init_string(&s_substr);
            copy_string(&s_substr, L.string);

            int Var_Type = bst_Get_Type_var(*var_tree, s_substr.str);
            if(Var_Type != K_string) error_out(SEM_ERROR_FUNCTION, "substr - Variable must be integer!");
            free_string(&s_substr);
        }

        get_token(&L);
        skip_whitespaces();
        if(L.T_Type != TT_Comma) error_out(SEM_ERROR_FUNCTION, "substr function - Comma missing!");

        get_token(&L);
        skip_whitespaces();
        if(L.T_Type != TT_Identifier || L.T_Type != TT_Integer || L.T_Type != TT_Double) error_out(SEM_ERROR_FUNCTION, "substr function - invalid token!");

        if(L.T_Type == TT_Identifier){
            bool ID_Declared = bst_Declared_var(*var_tree, L.string);
            if(ID_Declared == 0) error_out(SEM_ERROR_NDEFINED, "Substr is not declared!");

            init_string(&s_substr);
            copy_string(&s_substr, L.string);

            int Var_Type = bst_Get_Type_var(*var_tree, s_substr.str);
            if(Var_Type != K_integer && Var_Type != K_number) error_out(SEM_ERROR_FUNCTION, "substr - Variable must be integer!");
            free_string(&s_substr);
        }

        get_token(&L);
        skip_whitespaces();
        if(L.T_Type != TT_Comma) error_out(SEM_ERROR_FUNCTION, "substr function - Comma missing!");

        get_token(&L);
        skip_whitespaces();
        if(L.T_Type != TT_Identifier || L.T_Type != TT_Integer || L.T_Type != TT_Double) error_out(SEM_ERROR_FUNCTION, "substr function - invalid token!");

        if(L.T_Type == TT_Identifier){
            bool ID_Declared = bst_Declared_var(*var_tree, L.string);
            if(ID_Declared == 0) error_out(SEM_ERROR_NDEFINED, "Substr is not declared!");

            init_string(&s_substr);
            copy_string(&s_substr, L.string);

            int Var_Type = bst_Get_Type_var(*var_tree, s_substr.str);
            if(Var_Type != K_integer && Var_Type != K_number) error_out(SEM_ERROR_FUNCTION, "substr - Variable must be integer!");
            free_string(&s_substr);
        }

        get_token(&L);
        skip_whitespaces();
        if(L.T_Type != TT_Right_Bracket) error_out(SYNTAX_ERROR, "substr function - missing ')'!");

        assign_assign_counter++;
        List_Add(assign_assign_list, K_string, assign_assign_counter);

        get_token(&L);
        skip_whitespaces();
        if(L.T_Type == TT_Comma) return assign_right(var_tree, assign_var_list, Var_Counter, assign_assign_list, assign_assign_counter);
        List_Compare(assign_var_list, assign_assign_list);
        return result;
    }
    else if(!strcmp(L.string, "chr")){
        string chr_string;

        get_token(&L);
        skip_whitespaces();
        if(L.T_Type != TT_Left_Bracket) error_out(SYNTAX_ERROR, "chr function - missing '('!");

        get_token(&L);
        skip_whitespaces();
        if(L.T_Type != TT_Identifier || L.T_Type != TT_Integer) error_out(SEM_ERROR_FUNCTION, "chr function - invalid token!");

        if(L.T_Type == TT_Identifier){
            bool ID_Declared = bst_Declared_var(*var_tree, L.string);
            if(ID_Declared == 0) error_out(SEM_ERROR_NDEFINED, "ID is not declared!");

            init_string(&chr_string);
            copy_string(&chr_string, L.string);

            int Var_Type = bst_Get_Type_var(*var_tree, chr_string.str);
            if(Var_Type != K_integer) error_out(SEM_ERROR_FUNCTION, "chr - Variable must be integer!");
            free_string(&chr_string);
        }

        get_token(&L);
        skip_whitespaces();
        if(L.T_Type != TT_Right_Bracket) error_out(SYNTAX_ERROR, "chr function - missing ')'!");


        assign_assign_counter++;
        List_Add(assign_assign_list, K_string, assign_assign_counter);

        get_token(&L);
        skip_whitespaces();
        if(L.T_Type == TT_Comma) return assign_right(var_tree, assign_var_list, Var_Counter, assign_assign_list, assign_assign_counter);
        List_Compare(assign_var_list, assign_assign_list);
        return result;
    }
    else if(!strcmp(L.string, "ord")){
        string ord_string;

        get_token(&L);
        skip_whitespaces();
        if(L.T_Type != TT_Left_Bracket) error_out(SYNTAX_ERROR, "ord function - missing '('!");

        get_token(&L);
        skip_whitespaces();
        if(L.T_Type != TT_Identifier || L.T_Type != TT_String) error_out(SEM_ERROR_FUNCTION, "ord function - invalid token!");

        if(L.T_Type == TT_Identifier){
            bool ID_Declared = bst_Declared_var(*var_tree, L.string);
            if(ID_Declared == 0) error_out(SEM_ERROR_NDEFINED, "ID is not declared!");

            init_string(&ord_string);
            copy_string(&ord_string, L.string);

            int Var_Type = bst_Get_Type_var(*var_tree, ord_string.str);
            if(Var_Type != K_string) error_out(SEM_ERROR_FUNCTION, "ord - Variable must be string!");
            free_string(&ord_string);
        }

        get_token(&L);
        skip_whitespaces();
        if(L.T_Type != TT_Comma) error_out(SEM_ERROR_FUNCTION, "Missing comma between function parameters!");

        get_token(&L);
        skip_whitespaces();
        if(L.T_Type != TT_Identifier || L.T_Type != TT_Integer) error_out(SEM_ERROR_FUNCTION, "ord function - invalid token!");

        if(L.T_Type == TT_Identifier){
            bool ID_Declared = bst_Declared_var(*var_tree, L.string);
            if(ID_Declared == 0) error_out(SEM_ERROR_NDEFINED, "ID is not declared!");

            init_string(&ord_string);
            copy_string(&ord_string, L.string);

            int Var_Type = bst_Get_Type_var(*var_tree, ord_string.str);
            if(Var_Type != K_integer) error_out(SEM_ERROR_FUNCTION, "ord - Variable must be integer!");
            free_string(&ord_string);
        }

        get_token(&L);
        skip_whitespaces();
        if(L.T_Type != TT_Right_Bracket) error_out(SYNTAX_ERROR, "ord function - missing ')'!");

        assign_assign_counter++;
        List_Add(assign_assign_list, K_integer, assign_assign_counter);

        get_token(&L);
        skip_whitespaces();
        if(L.T_Type == TT_Comma) return assign_right(var_tree, assign_var_list, Var_Counter, assign_assign_list, assign_assign_counter);
        List_Compare(assign_var_list, assign_assign_list);
        return result;
    }

    if(L.T_Type != TT_Integer && L.T_Type != TT_String && L.T_Type != TT_Double && L.T_Type != TT_Identifier &&  L.T_Type != TT_Left_Bracket && L.T_Type != TT_Hashtag) error_out(SYNTAX_ERROR, "Invalid token after return!");

    statement_end_type stat;
    if(L.T_Type == TT_Identifier || L.T_Type != TT_String || L.T_Type != TT_Double || L.T_Type != TT_Identifier ||  L.T_Type != TT_Left_Bracket || L.T_Type != TT_Hashtag){
        if(L.T_Type == TT_Identifier){
            bst_node_function *tmp_F = bst_search_function(&function_tree, L.string);
            if(!(*tmp_F)){
                statement_end_type stat;
                stat = statement_parse(var_tree, &L);

                assign_assign_counter++;
                List_Add(assign_assign_list, stat.type, assign_assign_counter);

                if(L.T_Type == TT_Comma) return assign_right(var_tree, assign_var_list, assign_assign_counter, assign_assign_list, assign_assign_counter);
                List_Compare(assign_var_list, assign_assign_list);
                return result; 
            }
            else{
                string tmp;
                init_string(&tmp);
                copy_string(&tmp, L.string);

                get_token(&L);
                skip_whitespaces();
                if(L.T_Type != TT_Left_Bracket){
                    error_out(SYNTAX_ERROR, "Function missing '('");
                }
                else if(L.T_Type == TT_Left_Bracket){
                    function_parameter_counter = 0;
                    copy_string(&function_name, tmp.str);

                    result = function_call_parameters(var_tree);
                    if(result != 0) return result;
                    
                    function_list_element el = (*tmp_F)->return_codes->First;
                    for(int i = 0; i < (*tmp_F)->return_codes->num_of_elements; i++){
                        assign_assign_counter++;
                        List_Add(assign_assign_list, el->type, assign_assign_counter);
                        el = el->Next;
                    }
                    get_token(&L);
                    skip_whitespaces();

                    if(L.T_Type == TT_Comma) return assign_right(var_tree, assign_var_list, assign_assign_counter, assign_assign_list, assign_assign_counter);
                    List_Compare(assign_var_list, assign_assign_list);
                    return result; 
                }
            }
        }

        stat = statement_parse(var_tree, &L);
        if(stat.type == BOOL) error_out(SEM_ERROR_INCOMP_TYPE, "Assigning type bool!");
        assign_assign_counter++;
        List_Add(assign_assign_list, stat.type, assign_assign_counter);

        if(L.T_Type == TT_Comma) return assign_right(var_tree, assign_var_list, assign_assign_counter, assign_assign_list, assign_assign_counter);

        List_Compare(assign_var_list, assign_assign_list);
        return result;
    }
    return result;
}

int function_call_parameters(bst_node_var *var_tree){
    int result = 0;

    get_token(&L);
    skip_whitespaces();
    if(L.T_Type != TT_Integer && L.T_Type != TT_String && L.T_Type != TT_Double && L.T_Type != TT_Identifier &&  L.T_Type != TT_Right_Bracket && L.T_Type != TT_Integer) error_out(SEM_ERROR_FUNCTION, "Invalid token in function call!");

    if(L.T_Type == TT_Identifier){
        bool ID_Declared = bst_Declared_var(*var_tree, L.string);
        if(ID_Declared == 0) error_out(SEM_ERROR_NDEFINED, "Variable not declared!");
    }

    if(L.T_Type == TT_Right_Bracket){
        Insert_Function_Call(&function_tree, function_name.str, *var_tree, function_parameter_counter);
        return result;
    }

    int type;
    if(L.T_Type == TT_Integer) type = K_integer;
    else if(L.T_Type == TT_String) type = K_string;
    else if(L.T_Type == TT_Double) type = K_number;

    function_parameter_counter++;
    if(L.T_Type == TT_Identifier) Insert_Function_Parameter(&function_tree, function_name.str, bst_Get_Type_var(*var_tree, L.string), function_parameter_counter);
    else Insert_Function_Parameter(&function_tree, function_name.str, type, function_parameter_counter);
    
    get_token(&L);
    skip_whitespaces();
    if(L.T_Type != TT_Comma && L.T_Type != TT_Right_Bracket) error_out(SYNTAX_ERROR, "Invalid token in function call!");

    if(L.T_Type == TT_Comma){
        return function_call_parameters(var_tree);
    }

    Insert_Function_Call(&function_tree, function_name.str, *var_tree, function_parameter_counter);
    return result;
}


int function_params(bst_node_var *var_tree){
    int result = 0;
    int m_params = 0;

    if(L.T_Type == TT_Comma) m_params = 1;

    get_token(&L);
    skip_whitespaces();
    if(L.T_Type != TT_Identifier && L.T_Type != TT_Right_Bracket) error_out(SYNTAX_ERROR, "Invalid parameters!");

    if(L.T_Type == TT_Right_Bracket && m_params == 0) return result;
    else if(L.T_Type == TT_Right_Bracket && m_params == 1) error_out(SYNTAX_ERROR, "Invalid parameters!");

    bool ID_Declared = bst_Declared_var(*var_tree, L.string);
    string ID;
    init_string(&ID);
    copy_string(&ID, L.string);
        
    if(ID_Declared == 1) error_out(SEM_ERROR_NDEFINED, "Variable already declared!");

    get_token(&L);
    skip_whitespaces();
    if(L.T_Type != TT_Colon) error_out(SYNTAX_ERROR, "Function missing : !");
    get_token(&L);
    skip_whitespaces();
    if(L.keyword != K_number && L.keyword != K_integer && L.keyword != K_string) error_out(SYNTAX_ERROR, "Invalid or missing parameter type!");

    function_parameter_counter++;
    Insert_Function_Parameter(&function_tree, function_name.str, L.keyword, function_parameter_counter);
    bst_insert_var(var_tree, ID.str, L.keyword, scope);

    get_token(&L);
    skip_whitespaces();
    if(L.T_Type != TT_Comma && L.T_Type != TT_Right_Bracket) error_out(SYNTAX_ERROR, "Invalid parameters!");
    if(L.T_Type == TT_Comma) return function_params(var_tree);

    return result;
 }

 int function_returns(){
    int result = 0;
    //int m_params = 0;

    //if(L.T_Type == TT_Comma) m_params = 1;

    get_token(&L);
    skip_whitespaces();
    if(L.keyword != K_number && L.keyword != K_integer && L.keyword != K_string) error_out(SYNTAX_ERROR, "Invalid or return parameters!");

    //if(L.T_Type == TT_End_Of_Line && m_params == 0) return result;
    //else if(L.T_Type == TT_End_Of_Line && m_params == 1) error_out(SYNTAX_ERROR, "Incorrect return parameters!");

    int T_Data_type = 0;
    if(L.keyword == K_number) T_Data_type = K_number;
    else if(L.keyword == K_integer) T_Data_type = K_integer;
    else if(L.keyword == K_string) T_Data_type = K_string;
    else error_out(SEM_ERROR_OTHER, "Incompatible type in function parameters!");

    function_return_counter++;
    Insert_Return(&function_tree, function_name.str, T_Data_type, function_return_counter);

    get_token(&L);
    skip_whitespaces();
    if(L.T_Type == TT_Comma) return function_returns();

    return result;
 }

 int write_parameters(bst_node_var *var_tree){
     int result = 0;
     int m_params = 0;

     if(L.T_Type == TT_Comma) m_params = 1;
    
     get_token(&L);
     skip_whitespaces();
     if(L.T_Type != TT_Double && L.T_Type != TT_Integer && L.T_Type != TT_String && L.T_Type != TT_Identifier && L.T_Type != TT_Right_Bracket) error_out(SYNTAX_ERROR, "Write function - invalid parameters!");

     if(L.T_Type == TT_Right_Bracket && m_params == 0) return result;
     else if(L.T_Type == TT_Right_Bracket && m_params == 1) error_out(SYNTAX_ERROR, "Write function - Invalid parameters!");

     bool ID_Declared = bst_Declared_var(*var_tree, L.string);
     if(ID_Declared == 0 && L.T_Type == TT_Identifier) error_out(SEM_ERROR_NDEFINED, "Variable is not declared!");

     get_token(&L);
     skip_whitespaces();
     if(L.T_Type != TT_Comma && L.T_Type != TT_Right_Bracket) error_out(SYNTAX_ERROR, "Write function - Invalid parameters!");
     if(L.T_Type == TT_Comma) return write_parameters(var_tree);
    
    get_token(&L);
    skip_whitespaces();
    return result;
 }

 int return_values(bst_node_var *var_tree){
     int result = 0;
     int m_params = 0;
     if(L.T_Type == TT_Comma) m_params = 1;

     get_token(&L);
     skip_whitespaces();
     if(L.T_Type != TT_Double && L.T_Type != TT_Integer && L.T_Type != TT_String && L.T_Type != TT_Identifier && L.T_Type != TT_Keyword) error_out(SYNTAX_ERROR, "Invalid parameters in return!");
     if(L.T_Type == TT_Keyword && m_params == 0){
         Function_Return_Check(&function_tree, function_dec_name.str, function_return_counter);
         return result;
     }
     else if((L.T_Type == TT_Keyword) && m_params == 1) error_out(SYNTAX_ERROR, "Invalid parameters in return!");
     bool ID_Declared = bst_Declared_var(*var_tree, L.string);
     if(ID_Declared == 0 && L.T_Type == TT_Identifier) error_out(SEM_ERROR_NDEFINED, "Variable not declared!");
  
     statement_end_type stat = statement_parse(var_tree, &L);
     function_return_counter++;;
     Insert_Return(&function_tree, function_dec_name.str, stat.type, function_return_counter);

     if(L.T_Type == TT_Comma) return return_values(var_tree);

     Function_Return_Check(&function_tree, function_dec_name.str, function_return_counter);
     return result;
 }

 void init_string(string *str){
    str->str = (char *)malloc(DEFAULT_STRING_LEN);
    if(!str->str) error_out(INTERNAL_ERROR, "Malloc Error!");
    str->str[0] = '\0';
    str->string_lenght = 0;
    str->string_alloc_lenght = DEFAULT_STRING_LEN;
}

void free_string(string *str){
    free(str->str);
}

void string_clear(string *str){
    str->string_lenght = 0;
    str->str[0] = '\0';
}

void copy_string(string *dest, char *source){
    int new_len = strlen(source)+1;

    if(new_len > dest->string_alloc_lenght){
        dest->str = (char *)realloc(dest->str, new_len);
        if(!dest->str) error_out(INTERNAL_ERROR, "Malloc Error!");
        dest->string_alloc_lenght = new_len;
    }
    strcpy(dest->str, source);
    dest->string_lenght = new_len-1;
}

int global_func_def(){
    get_token(&L);
    skip_whitespaces();

    if(L.T_Type != TT_Identifier) error_out(SYNTAX_ERROR, "Function definition syntax error!");
    string func_name;
    init_string(&func_name);
    copy_string(&func_name, L.string);
    bst_insert_function(&function_tree, L.string);
    int param_counter = 0;
    bst_node_function *el = bst_search_function(&function_tree, func_name.str);
    (*el)->Global_Dec = 1;

    get_token(&L);
    skip_whitespaces();
    
    if(L.T_Type != TT_Colon) error_out(SYNTAX_ERROR, "Function definition syntax error!");
    get_token(&L);
    skip_whitespaces();

    if(L.keyword != K_function) error_out(SYNTAX_ERROR, "Function definition syntax error!");
    get_token(&L);
    skip_whitespaces();

    if(L.T_Type != TT_Left_Bracket) error_out(SYNTAX_ERROR, "Function definition syntax error!");
    get_token(&L);
    skip_whitespaces();

    while(L.T_Type != TT_Right_Bracket){
        if(L.T_Type == TT_Comma){
            get_token(&L);
            skip_whitespaces();
        }

        if(L.keyword != K_integer && L.keyword != K_string && L.keyword != K_number) error_out(SYNTAX_ERROR, "Function definition syntax error!");
        param_counter++;
        Insert_Function_Parameter(&function_tree, func_name.str, L.keyword, param_counter);

        get_token(&L);
        skip_whitespaces();

        if(L.T_Type != TT_Right_Bracket && L.T_Type != TT_Comma) error_out(SYNTAX_ERROR, "Function definition syntax error!");
    }

    get_token(&L);
    skip_whitespaces();

    if(L.T_Type != TT_Colon) return 0;

    get_token(&L);
    skip_whitespaces();
    param_counter = 0;

    while(L.keyword != K_function && L.T_Type != TT_Identifier){
        if(L.T_Type == TT_Comma){
            get_token(&L);
            skip_whitespaces();
        }

        if(L.keyword != K_integer && L.keyword != K_string && L.keyword != K_number) error_out(SYNTAX_ERROR, "Function definition syntax error!");
        param_counter++;
        Insert_Return(&function_tree, func_name.str, L.keyword, param_counter);

        get_token(&L);
        skip_whitespaces();

        if(L.keyword != K_function && L.T_Type != TT_Comma && L.T_Type != TT_Identifier) error_out(SYNTAX_ERROR, "Function definition syntax error!");
    }
    (*el)->Declared = 1;
    global = 1;
    return 0;
}