/*
Project: Compiler for language IFJ21

File: statement_parser.c

AUTHORS:
Martin Bublavý <xbubla02@stud.fit.vutbr.cz>

*/

#include "statement_parser.h"

#define DEFAULT_STACK_SIZE 1000

int prec_table[20][20] = {
/*                      +    -    *    /    //   <    <=   >    >=   ==   ~=   #   ..    (    )      ID    INT   STRING  DOUBLE   $   */
/* + */               {'>', '>', '<', '<', '<', '>', '>', '>', '>', '>', '>', '<', 'X', '<', '>',   '<',   '<',   '<',    '<',   '>'},
/* - */               {'>', '>', '<', '<', '<', '>', '>', '>', '>', '>', '>', '<', 'X', '<', '>',   '<',   '<',   '<',    '<',   '>'},
/* * */               {'>', '>', '>', '>', '>', '>', '>', '>', '>', '>', '>', '<', 'X', '<', '>',   '<',   '<',   '<',    '<',   '>'},
/* / */               {'>', '>', '>', '>', '>', '>', '>', '>', '>', '>', '>', '<', 'X', '<', '>',   '<',   '<',   '<',    '<',   '>'},
/* // */              {'>', '>', '>', '>', '>', '>', '>', '>', '>', '>', '>', '<', 'X', '<', '>',   '<',   '<',   '<',    '<',   '>'},
/* < */               {'<', '<', '<', '<', '<', 'X', 'X', 'X', 'X', 'X', 'X', '<', 'X', '<', '>',   '<',   '<',   '<',    '<',   '>'},
/* <= */              {'<', '<', '<', '<', '<', 'X', 'X', 'X', 'X', 'X', 'X', '<', 'X', '<', '>',   '<',   '<',   '<',    '<',   '>'},
/* > */               {'<', '<', '<', '<', '<', 'X', 'X', 'X', 'X', 'X', 'X', '<', 'X', '<', '>',   '<',   '<',   '<',    '<',   '>'},
/* >= */              {'<', '<', '<', '<', '<', 'X', 'X', 'X', 'X', 'X', 'X', '<', 'X', '<', '>',   '<',   '<',   '<',    '<',   '>'},
/* == */              {'<', '<', '<', '<', '<', 'X', 'X', 'X', 'X', 'X', 'X', '<', 'X', '<', '>',   '<',   '<',   '<',    '<',   '>'},
/* ~= */              {'<', '<', '<', '<', '<', 'X', 'X', 'X', 'X', 'X', 'X', '<', 'X', '<', '>',   '<',   '<',   '<',    '<',   '>'},
/* # */               {'X', 'X', 'X', 'X', 'X', 'X', 'X', 'X', 'X', 'X', 'X', 'X', 'X', 'X', 'X',   '>',   'X',   '>',    'X',   '>'},
/* .. */              {'X', 'X', 'X', 'X', 'X', 'X', 'X', 'X', 'X', 'X', 'X', 'X', 'X', 'X', '>',   '<',   'X',   '<',    'X',   '>'},
/* ( */               {'<', '<', '<', '<', '<', '<', '<', '<', '<', '<', '<', '<', '<', '<', '=',   '<',   '<',   '<',    '<',   'X'},
/* ) */               {'>', '>', '>', '>', '>', '>', '>', '>', '>', '>', '>', 'X', '>', 'X', '>',   'X',   'X',   'X',    'X',   '>'},
/* ID */              {'>', '>', '>', '>', '>', '>', '>', '>', '>', '>', '>', 'X', '>', 'X', '>',   'X',   'X',   'X',    'X',   '>'},
/* INT */             {'>', '>', '>', '>', '>', '>', '>', '>', '>', '>', '>', 'X', 'X', 'X', '>',   'X',   'X',   'X',    'X',   '>'},
/* STRING */          {'>', '>', '>', '>', '>', '>', '>', '>', '>', '>', '>', 'X', '>', 'X', '>',   'X',   'X',   'X',    'X',   '>'},
/* DOUBLE */          {'>', '>', '>', '>', '>', '>', '>', '>', '>', '>', '>', 'X', 'X', 'X', '>',   'X',   'X',   'X',    'X',   '>'},
/* $ */               {'<', '<', '<', '<', '<', '<', '<', '<', '<', '<', '<', '<', '<', '<', 'X',   '<',   '<',   '<',    '<',   '&'}
};

int TT_to_PT(int T_Type){
    switch(T_Type){
        case(TT_Add):
            return PLUS;

        case(TT_Subtract):
            return MINUS;

        case(TT_Multiply):
            return MULTIPLY;

        case(TT_Divide):
            return DIVISION;

        case(TT_INT_Divide):
            return INT_DIV;

        case(TT_Hashtag):
            return HASHTAG;

        case(TT_Concat):
            return CONCAT;

        case(TT_Left_Bracket):
            return LEFT_BRACKET;

        case(TT_Right_Bracket):
            return RIGHT_BRACKET;

        case(TT_Equals):
            return EQUALS;

        case(TT_Not_Equal):
            return NOT_EQUAL;

        case(TT_Less_Than):
            return LESS_THAN;

        case(TT_LT_Equals):
            return LT_EQUALS;

        case(TT_Bigger_Than):
            return BIGGER_THAN;

        case(TT_BT_Equals):
            return BT_EQUALS;

        case(TT_Identifier):
            return IDENTIFIER;

        case(TT_String):
            return STRING;

        case(TT_Integer):
            return INTEGER;

        case(TT_Double):
            return DOUBLE;
        
        case(TT_End_Of_File):
        case(TT_End_Of_Line):
        case(TT_Keyword):
        case(TT_PreDefined_Func):
        case(TT_Comma):
            return DOLLAR;
        
        default:
            error_out(SYNTAX_ERROR, "Incorrect statement syntax!");
            return 1;
    }
    error_out(INTERNAL_ERROR, "Internal ERROR!");
    return 1;
}

prec_stack * stack_init(){
    prec_stack * s = (prec_stack *)malloc(sizeof(prec_stack));
    if(!s) error_out(INTERNAL_ERROR, "Malloc Error!");
    s->ts = (token_string *)malloc(sizeof(token_string) * DEFAULT_STACK_SIZE);
    if(!(s->ts)) error_out(INTERNAL_ERROR, "Malloc Error!");
    s->max_size = DEFAULT_STACK_SIZE;
    s->top = -1;
    return s;
}

void free_stack(prec_stack *s){
    free(s);
}

int push_prec_stack(prec_stack *s, int symbol, string str){
    if((is_full_stack(s)) == 1) error_out(INTERNAL_ERROR, "ERROR: Precedence stack full! (Statement too long)");

    s->top++;
    s->ts[s->top].symbol = symbol;
    init_string(&(s->ts[s->top].token_s));
    copy_string(&(s->ts[s->top].token_s), str.str);
    return 0;
}   

int pop_prec_stack(prec_stack *s){
    if(is_empty_stack(s) == 1) return 0;

    free_string(&(s->ts[s->top].token_s));
    s->top--;
    return 0;
}

int is_full_stack(prec_stack *s){
    if(((s->top)+1) == DEFAULT_STACK_SIZE) return 1;
    else return 0;
}

int is_empty_stack(prec_stack *s){
    if(s->top == -1) return 1;
    else return 0;
}

token_string *top(prec_stack *s){
    if(is_empty_stack(s) == 1) return NULL;
    else return &(s->ts[s->top]);
}

token_string *top_2(prec_stack *s){
    if(is_empty_stack(s) == 1) return NULL;
    else return &(s->ts[s->top-1]);
}

token_string *top_3(prec_stack *s){
    if(is_empty_stack(s) == 1) return NULL;
    else return &(s->ts[s->top-2]);
}

token_string *get_symbol(prec_stack *s){
    token_string *str = top(s);
    if(str->symbol >= 20){
        str = top_2(s);
    }
    return str;
}




void rule_reduction(bst_node_var *var_tree, prec_stack *s, token_string *top_symbol){
    if(top_symbol->symbol == IDENTIFIER){
        if(!strcmp(top_symbol->token_s.str, "nil")){
            pop_prec_stack(s);
            pop_prec_stack(s);
            pop_prec_stack(s);
            string str;
            init_string(&str);
            string_clear(&str);
            push_prec_stack(s, BOOL, str);
        }
        else{
            if(bst_Declared_var(*var_tree, top_symbol->token_s.str) == 0) error_out(SEM_ERROR_NDEFINED, "Variable used in statement is not defined!");

            int var_type = bst_Get_Type_var(*var_tree, top_symbol->token_s.str);

            token_string *s2 = top_2(s);
            if(s2->symbol == R_HASHTAG && var_type == K_string){
                pop_prec_stack(s);
                pop_prec_stack(s);
                string str;
                init_string(&str);
                string_clear(&str);
                push_prec_stack(s, R_INT, str);
            }
            else{
                if(var_type == K_integer) top_symbol->symbol = R_INT;
                else if(var_type == K_number) top_symbol->symbol = R_DOUBLE;
                else if(var_type == K_string) top_symbol->symbol = R_STRING;
                else error_out(SEM_ERROR_INCOMP_ASSIGN_INT_STRING, "Invalid type of variable in statement!");
            }
        }
    }
    else if(top_symbol->symbol == STRING){
        token_string *s2 = top_2(s);
        if(s2->symbol == R_HASHTAG){
            pop_prec_stack(s);
            pop_prec_stack(s);
            string str;
            init_string(&str);
            string_clear(&str);
            push_prec_stack(s, R_INT, str);
        }
        else{
            top_symbol->symbol = R_STRING;
        }
    }
    else if(top_symbol->symbol == INTEGER){
        top_symbol->symbol = R_INT;
    }
    else if(top_symbol->symbol == DOUBLE){
        top_symbol->symbol = R_DOUBLE;
    }
    else if(top_symbol->symbol == PLUS){
        check_types(s);
    }
    else if(top_symbol->symbol == MINUS){
        check_types(s);
    }
    else if(top_symbol->symbol == MULTIPLY){
        check_types(s);
    }
    else if(top_symbol->symbol == DIVISION){
        token_string *s1 = top_3(s);
        token_string *s2 = top(s);

        if(s1->symbol == R_DOUBLE && s2->symbol == R_DOUBLE){
            double num = atof(s2->token_s.str);
            if(num == 0.0) error_out(ZERO_DIV_ERROR, "Division by 0 not allowed!");
            check_types(s);
        }
        else error_out(SEM_ERROR_INCOMP_ASSIGN_INT_STRING, "Invalid type of variable in statement!");
    }
    else if(top_symbol->symbol == INT_DIV){
        token_string *s1 = top_3(s);
        token_string *s2 = top(s);

        if(s1->symbol == R_INT && s2->symbol == R_INT){
            if(!strcmp(s2->token_s.str, "0")) error_out(ZERO_DIV_ERROR, "Division by 0 not allowed!");
            check_types(s);
        }
        else{
            error_out(SEM_ERROR_INCOMP_ASSIGN_INT_STRING, "Invalid type of variable in statement!");
        }
    }
    else if(top_symbol->symbol == RIGHT_BRACKET){
        token_string *s1 = top_2(s);
        int sym = s1->symbol;
        pop_prec_stack(s);
        pop_prec_stack(s);
        pop_prec_stack(s);
        string str;
        init_string(&str);
        copy_string(&str, s1->token_s.str);
        push_prec_stack(s, sym, str);
    }
    else if(top_symbol->symbol == LESS_THAN || top_symbol->symbol == LT_EQUALS || top_symbol->symbol == BIGGER_THAN ||
            top_symbol->symbol == BT_EQUALS || top_symbol->symbol == EQUALS || top_symbol->symbol == NOT_EQUAL){

        string str;
        init_string(&str);
        string_clear(&str);

        token_string *s1 = top(s);
        token_string *s2 = top_3(s);

        if((s1->symbol == R_INT && s2->symbol == R_INT) || (s1->symbol == R_STRING && s2->symbol == R_STRING)
            || (s1->symbol == R_DOUBLE && s2->symbol == R_DOUBLE)){

            pop_prec_stack(s);
            pop_prec_stack(s);
            pop_prec_stack(s);
            push_prec_stack(s, BOOL, str);
        }
        else error_out(SEM_ERROR_INCOMP_ASSIGN_INT_STRING, "Invalid type of variable in statement!");
    }
    else if(top_symbol->symbol == HASHTAG){
        top_symbol->symbol = R_HASHTAG;
    }
    else if(top_symbol->symbol == CONCAT){
        top_symbol->symbol = R_CONCAT;
        token_string *s3 = top_3(s);
        token_string *s1 = top(s);

        if(s3->symbol == R_STRING && s1->symbol == R_STRING){
                    pop_prec_stack(s);
                    pop_prec_stack(s);
                    pop_prec_stack(s);
                    string str;
                    init_string(&str);
                    string_clear(&str);
                    push_prec_stack(s, R_STRING, str);
        }
        else{
            error_out(SEM_ERROR_INCOMP_ASSIGN_INT_STRING, "Invalid type of variable in statement!");
        }
    }
    else error_out(SYNTAX_ERROR, "Statement error!");
    return;
}

void check_types(prec_stack *s){
    token_string *s1 = top_3(s);
    token_string *s2 = top(s);
    string str;
    init_string(&str);
    string_clear(&str);

    if(s1->symbol == s2->symbol){
        if(s1->symbol == R_INT){
            pop_prec_stack(s);
            pop_prec_stack(s);
            pop_prec_stack(s);
            push_prec_stack(s, R_INT, str);
        }
        else if(s1->symbol == R_STRING){
            pop_prec_stack(s);
            pop_prec_stack(s);
            pop_prec_stack(s);
            push_prec_stack(s, R_STRING, str);
        }
        else if(s1->symbol == R_DOUBLE){
            pop_prec_stack(s);
            pop_prec_stack(s);
            pop_prec_stack(s);
            push_prec_stack(s, R_DOUBLE, str);
        }
    }
    else if((s1->symbol == R_INT && s2->symbol == R_DOUBLE) || (s1->symbol == R_DOUBLE && s2->symbol == R_INT)){
        pop_prec_stack(s);
        pop_prec_stack(s);
        pop_prec_stack(s);
        push_prec_stack(s, R_DOUBLE, str);
    }
    else error_out(SEM_ERROR_INCOMP_ASSIGN_INT_STRING, "Performing operations with different data types!");

    return;
}


statement_end_type statement_parse(bst_node_var *tree, Lexeme *L){
    bool last_token_operand = 0;
    bool exception = 0;
    Token_Type T;
    bool end = 0;
    token_string *top_symbol;
    prec_stack *p_stack = stack_init();

    string ts;
    init_string(&ts);
    string_clear(&ts);
    push_prec_stack(p_stack, DOLLAR, ts);
    
    if(TT_to_PT(L->T_Type) < 11 || TT_to_PT(L->T_Type) == RIGHT_BRACKET || TT_to_PT(L->T_Type) == CONCAT) error_out(SYNTAX_ERROR, "Incorrect statement!");
    if(L->T_Type == TT_Integer || L->T_Type == TT_String || L->T_Type == TT_Double || L->T_Type == TT_Identifier) last_token_operand = 1;

    while(end != 1){
        top_symbol = get_symbol(p_stack);

        switch(prec_table[top_symbol->symbol][TT_to_PT(L->T_Type)]){
            case('&'):
                end = 1;
                break;

            case('='):
            case('<'):
                copy_string(&ts, L->string);
                push_prec_stack(p_stack, TT_to_PT(L->T_Type), ts);

                get_token(L);
                skip_whitespaces();
                if(L->T_Type == TT_Identifier || L->T_Type == TT_Integer || L->T_Type == TT_String || L->T_Type == TT_Double){
                    if(last_token_operand == 1){
                        exception = 1;
                        T = L->T_Type;
                        L->T_Type = TT_PreDefined_Func;
                        break;
                    }
                    else last_token_operand = 1;
                }
                else if((L->T_Type >= 8 && L->T_Type <= 13) || (L->T_Type >= 15 && L->T_Type <= 20)){
                    if(last_token_operand == 0) error_out(SYNTAX_ERROR, "Incorrect statement - Operator cannot go after operator!");
                    else last_token_operand = 0;
                }
                break;
            case('X'):
                error_out(SEM_ERROR_INCOMP_ASSIGN_INT_STRING, "Incorrect statement!");
                break;

            case('>'):
                rule_reduction(tree, p_stack, top_symbol);
                break;

            default:
                error_out(SYNTAX_ERROR, "Incorrect statement!");
        }
    }

    statement_end_type stat;
    stat.token = L->T_Type;
    
    token_string *s1 = top(p_stack);
    if(s1->symbol == R_INT) stat.type = K_integer;
    else if(s1->symbol == R_STRING) stat.type = K_string;
    else if(s1->symbol == R_DOUBLE) stat.type = K_number;
    else if(s1->symbol == BOOL) stat.type = BOOL;
    else error_out(SEM_ERROR_OTHER, "Invalid statement!");
    if(exception == 1) L->T_Type = T;
    return stat;
}
