/*
Project: Compiler for language IFJ21

File: scanner.c

AUTHORS:
Martin Bublavý <xbubla02@stud.fit.vutbr.cz>
Igor Pszota <xpszot00@stud.fit.vutbr.cz>

*/

#include <string.h>
#include <ctype.h>

#include "scanner.h"
#include "error.h"

#define Default_Lexeme_Init_Lenght 20
FILE *source_stream;


bool LS_init(Lexeme *LS){
    LS->string = malloc(Default_Lexeme_Init_Lenght);

    if(!LS->string){
        return false;
    }

    LS->Lexeme_lenght = 0;
    LS->Alloc_lenght = Default_Lexeme_Init_Lenght;
    return true;
}


//Sets Source 
void set_source_stream(FILE *source){
    source_stream = source;
}




bool LS_Add_Char(Lexeme *LS, char c){
    if(LS->Lexeme_lenght+1 < LS->Alloc_lenght){
        LS->string[LS->Lexeme_lenght] = c;
        LS->Lexeme_lenght++;
        return true;
    }
    else{
        LS->string = realloc(LS->string ,(LS->Alloc_lenght*2));
        LS->Alloc_lenght = LS->Alloc_lenght*2;
        for(int i = LS->Lexeme_lenght; i < LS->Alloc_lenght; i++){
            LS->string[i] = '\0';
        }
        if(!LS->string){
            return false;
        }

        LS->string[LS->Lexeme_lenght] = c;
        LS->Lexeme_lenght++;
        return true;
    }
}

void LS_Clear(Lexeme *LS){
    for(int i = 0; i < LS->Alloc_lenght; i++){
        LS->string[i] = '\0';
    }
    LS->keyword = K_not_a_keyword;
    LS->Lexeme_lenght = 0;
}

void LS_Free(Lexeme *LS){
    free(LS->string);
}


int get_token(Lexeme *LS){
    LS_Clear(LS);
    Lexeme tmp;
    FSM_STATE state;
    state = FSM_idle;
    char c;

//FSM----------------------------------------------------------------------------------------------------------------

    while(1){
        c = getc(source_stream);
        //printf("Zatial: %s\n", LS->string);
        
        switch(state){
            case(FSM_idle):
                if(isalpha(c) || c == '_'){
                    state = FSM_Identifier_Keyword;
                     if(!LS_Add_Char(LS, c)){
                        LS_Free(LS);
                        error_out(INTERNAL_ERROR, "Malloc Error!");
                    }
                    break;
                }  
                else if(isdigit(c)){
                    state = FSM_Integer;
                    LS->T_Type = TT_Integer;
                     if(!LS_Add_Char(LS, c)){
                        LS_Free(LS);
                        error_out(INTERNAL_ERROR, "Malloc Error!");
                    }
                }
                else if(c == '-'){
                    state = FSM_Minus;
                }
                else if(c == ']'){
                    state = FSM_Block_Comment_End_End;
                }
                else if(c == '+'){
                    LS->T_Type = TT_Add;
                    return 0;
                }
                else if(c == '/'){
                    state = FSM_Division;
                    LS->T_Type = TT_Divide;
                }
                else if(c == '*'){
                    LS->T_Type = TT_Multiply;
                    return 0;
                }
                else if(c == '('){
                    LS->T_Type = TT_Left_Bracket;
                    return 0;
                }
                else if(c == ')'){
                    LS->T_Type = TT_Right_Bracket;
                    return 0;
                }
                else if(c == '\n'){
                    LS->T_Type = TT_End_Of_Line;
                    return 0;
                }
                else if(c == '<'){
                    state = FSM_Less_Than;
                    if(!LS_Add_Char(LS, c)){
                        LS_Free(LS);
                        error_out(INTERNAL_ERROR, "Malloc Error!");
                    }
                }
                else if(c == '>'){
                    state = FSM_More_Than;
                    if(!LS_Add_Char(LS, c)){
                        LS_Free(LS);
                        error_out(INTERNAL_ERROR, "Malloc Error!");
                    }
                }
                else if(c == '~'){
                    state = FSM_Not_Equal;
                    if(!LS_Add_Char(LS, c)){
                        LS_Free(LS);
                        error_out(INTERNAL_ERROR, "Malloc Error!");
                    }
                }
                else if(c == '='){
                    state = FSM_Assign;
                    if(!LS_Add_Char(LS, c)){
                        LS_Free(LS);
                        error_out(INTERNAL_ERROR, "Malloc Error!");
                    }
                }
                else if(c == '#'){
                    LS->T_Type = TT_Hashtag;
                    if(!LS_Add_Char(LS, c)){
                        LS_Free(LS);
                        error_out(INTERNAL_ERROR, "Malloc Error!");
                    }
                    return 0;
                }
                else if(c == EOF){
                    LS->T_Type = TT_End_Of_File;
                    return 0;
                }
                else if(isspace(c)){
                    state = FSM_idle;
                }
                else if(c == ','){
                    LS->T_Type = TT_Comma;
                    if(!LS_Add_Char(LS, c)){
                        LS_Free(LS);
                        error_out(INTERNAL_ERROR, "Malloc Error!");
                    }
                    return 0;
                }
                else if(c == '"'){
                    LS_init(&tmp);
                    LS_Clear(&tmp);
                    state = FSM_String;
                }
                else if(c == '.'){
                    state = FSM_Dot;
                    if(!LS_Add_Char(LS, c)){
                        LS_Free(LS);
                        error_out(INTERNAL_ERROR, "Malloc Error!");
                    }
                }
                else if(c == ':'){
                    LS->T_Type = TT_Colon;
                    if(!LS_Add_Char(LS, c)){
                        LS_Free(LS);
                        error_out(INTERNAL_ERROR, "Malloc Error!");
                    }
                    return 0;
                }
                else{
                    error_out(LEXEMA_SYNTAX_ERROR, "Token not recognized!");
                }
                break;

            case(FSM_Identifier_Keyword):
                if(isalnum(c) || c == '_'){ 
                    if(!LS_Add_Char(LS, c)){
                        LS_Free(LS);
                        error_out(INTERNAL_ERROR, "Malloc Error!");
                    }
                }
                else{
                    ungetc(c, source_stream);
                    LS->T_Type = TT_Identifier;
                    Is_It_Keyword(LS);
                    return 0;
                }
                break;


            case(FSM_Integer):
                if(isdigit(c)){
                    if(!LS_Add_Char(LS, c)){
                        LS_Free(LS);
                        error_out(INTERNAL_ERROR, "Malloc Error!");
                    }
                }
                else if(c == '.'){
                    state = FSM_Double_Point;
                    LS->T_Type = TT_Double;
                    if(!LS_Add_Char(LS, c)){
                        LS_Free(LS);
                        error_out(INTERNAL_ERROR, "Malloc Error!");
                    }
                    break;
                }
                else if(c == 'e' || c == 'E'){
                    state = FSM_Exponent;
                    LS->T_Type = TT_Exponent;
                    LS->T_Type = TT_Double;
                    if(!LS_Add_Char(LS, c)){
                     LS_Free(LS);
                    error_out(INTERNAL_ERROR, "Malloc Error!");
                    }
                    break;
                }
                else{
                    ungetc(c, source_stream);
                    return 0;
                }
                break;
            

            case(FSM_Double_Point):
                if(isdigit(c)){
                    state = FSM_Double;
                    if(!LS_Add_Char(LS, c)){
                        LS_Free(LS);
                        error_out(INTERNAL_ERROR, "Malloc Error!");
                    }
                }
                else{
                    error_out(LEXEMA_SYNTAX_ERROR, "Token not recognized!");
                }
                break;
            
            
            case(FSM_Double):
                if(isdigit(c)){
                    if(!LS_Add_Char(LS, c)){
                        LS_Free(LS);
                        error_out(INTERNAL_ERROR, "Malloc Error!");
                    }
                }
                else if(c == 'e' || c == 'E'){
                    state = FSM_Exponent;
                    LS->T_Type = TT_Exponent;
                    LS->T_Type = TT_Double;
                    if(isdigit(c)){
                        if(!LS_Add_Char(LS, c)){
                            LS_Free(LS);
                            error_out(INTERNAL_ERROR, "Malloc Error!");
                        }
                    }
                }
                else{
                    ungetc(c, source_stream);
                    return 0;
                }
                break;

            
            case(FSM_Exponent):
                if(c == '-' || c == '+'){
                    state = FSM_Exponent_Sign;
                    if(!LS_Add_Char(LS, c)){
                        LS_Free(LS);
                        error_out(INTERNAL_ERROR, "Malloc Error!");
                    }
                    break;
                }
                else if(isdigit(c)){
                    state = FSM_Exponent_Finish;
                    if(!LS_Add_Char(LS, c)){
                        LS_Free(LS);
                        error_out(INTERNAL_ERROR, "Malloc Error!");
                    }
                    break;
                }
                else{
                    error_out(LEXEMA_SYNTAX_ERROR, "Token not recognized!");
                    return LEXEMA_SYNTAX_ERROR;
                }


            case(FSM_Exponent_Sign):
                if(isdigit(c)){
                    state = FSM_Exponent_Finish;
                    if(!LS_Add_Char(LS, c)){
                        LS_Free(LS);
                        error_out(INTERNAL_ERROR, "Malloc Error!");
                    }
                }
                else{
                    error_out(LEXEMA_SYNTAX_ERROR, "Token not recognized!");
                    return LEXEMA_SYNTAX_ERROR;
                }
                break;


            case(FSM_Exponent_Finish):
                if(isdigit(c)){
                    if(!LS_Add_Char(LS, c)){
                        LS_Free(LS);
                        error_out(INTERNAL_ERROR, "Malloc Error!");
                    }
                    break;
                }
                else{
                    ungetc(c, source_stream);
                    return 0;
                }

                
            case(FSM_Minus):
                if(c == '-'){
                    state = FSM_Comment;
                    LS->T_Type = TT_Comment;
                }
                else{
                    ungetc(c, source_stream);
                    LS->T_Type = TT_Subtract;
                    return 0;
                }
                break;


            case(FSM_Comment):
                if(c == '['){
                    state = FSM_Block_Comment;
                }
                else{
                    if(!LS_Add_Char(LS, c)){
                        LS_Free(LS);
                        error_out(INTERNAL_ERROR, "Malloc Error!");
                    }
                    state = FSM_Comment_String;
                    break;
                }
                break;
            

            case(FSM_Comment_String):
                if(c == '\n'){
                    ungetc(c, source_stream);
                    return 0;
                }
                if(!LS_Add_Char(LS, c)){
                    LS_Free(LS);
                    error_out(INTERNAL_ERROR, "Malloc Error!");
                }
                break;


            case(FSM_Block_Comment):
                if(c == '['){
                    LS->T_Type = TT_Block_Comment;
                    state = FSM_Block_Comment_String;
                    break;
                }
                else{
                    ungetc(c, source_stream);
                    ungetc('[', source_stream);
                    state = FSM_Comment_String;
                    break;
                }

            
            case(FSM_Block_Comment_String):
                if(c == ']'){
                    state = FSM_Block_Comment_End;
                    break;
                }
                if(!LS_Add_Char(LS, c)){
                    LS_Free(LS);
                    error_out(INTERNAL_ERROR, "Malloc Error!");
                }
                break;

            
            case(FSM_Block_Comment_End):
                if(c == ']'){
                    ungetc(c, source_stream);
                    ungetc(c, source_stream);
                    return 0;
                }
                else{
                    if(!LS_Add_Char(LS, ']')){
                        LS_Free(LS);
                        error_out(INTERNAL_ERROR, "Malloc Error!");
                    }
                    if(!LS_Add_Char(LS, c)){
                        LS_Free(LS);
                        error_out(INTERNAL_ERROR, "Malloc Error!");
                    }
                    state = FSM_Block_Comment_String;
                    break;
                }


            case(FSM_Block_Comment_End_End):
                if(c == ']'){
                    LS->T_Type = TT_Block_Comment_End;
                    return 0;
                }
                else{
                    LS_Free(LS);
                    error_out(LEXEMA_SYNTAX_ERROR, "Token not recognized!");
                    return LEXEMA_SYNTAX_ERROR;
                }



            case(FSM_Division):
                if(c == '/'){
                    LS->T_Type = TT_INT_Divide;
                    if(!LS_Add_Char(LS, c)){
                        LS_Free(LS);
                        error_out(INTERNAL_ERROR, "Malloc Error!");
                    }
                    return 0;
                }
                else{
                    ungetc(c,source_stream);
                    return 0;
                }


            case(FSM_Less_Than):
                if(c == '='){
                    LS->T_Type = TT_LT_Equals;
                    if(!LS_Add_Char(LS, c)){
                        LS_Free(LS);
                        error_out(INTERNAL_ERROR, "Malloc Error!");
                    }
                    return 0;
                }
                else{
                    LS->T_Type = TT_Less_Than;
                    ungetc(c,source_stream);
                    return 0;
                }


            case(FSM_More_Than):
                if(c == '='){
                    LS->T_Type = TT_BT_Equals;
                    if(!LS_Add_Char(LS, c)){
                        LS_Free(LS);
                        error_out(INTERNAL_ERROR, "Malloc Error!");
                    }
                    return 0;
                }
                else{
                    LS->T_Type = TT_Bigger_Than;
                    ungetc(c,source_stream);
                    return 0;
                }


            case(FSM_Not_Equal):
                if(c == '='){
                    LS->T_Type = TT_Not_Equal;
                    if(!LS_Add_Char(LS, c)){
                        LS_Free(LS);
                        error_out(INTERNAL_ERROR, "Malloc Error!");
                    }
                    return 0;
                }
                else{
                    LS_Free(LS);
                    error_out(LEXEMA_SYNTAX_ERROR, "Token not recognized!");
                    return LEXEMA_SYNTAX_ERROR;
                }


            case(FSM_Assign):
                if(c == '='){
                    LS->T_Type = TT_Equals;
                    if(!LS_Add_Char(LS, c)){
                        LS_Free(LS);
                        error_out(INTERNAL_ERROR, "Malloc Error!");
                    }
                    return 0;
                }
                else{
                    LS->T_Type = TT_Assign;
                    ungetc(c, source_stream);
                    return 0;
                }


            case(FSM_String):
                if(c == '\\'){
                    state = FSM_Escape;
                }
                else if(c == '"'){
                    LS_Free(&tmp);
                    LS->T_Type = TT_String;
                    return 0;
                }
                else if(c == EOF){
                    LS_Free(LS);
                    error_out(LEXEMA_SYNTAX_ERROR, "Token not recognized!");
                }
                else{
                    if(!LS_Add_Char(LS, c)){
                        LS_Free(LS);
                        error_out(INTERNAL_ERROR, "Malloc Error!");
                    }
                }
                break;
                
            
            case(FSM_Escape):
                if(c == '\\'){
                    if(!LS_Add_Char(LS, c)){
                        LS_Free(LS);
                        error_out(INTERNAL_ERROR, "Malloc Error!");
                    }
                }
                else if(c == '"'){
                    if(!LS_Add_Char(LS, c)){
                        LS_Free(LS);
                        error_out(INTERNAL_ERROR, "Malloc Error!");
                    }
                }
                else if(c == 'n'){
                    c = '\n';
                    if(!LS_Add_Char(LS, c)){
                        LS_Free(LS);
                        error_out(INTERNAL_ERROR, "Malloc Error!");
                    }
                }
                else if(c == 't'){
                    c = '\t';
                    if(!LS_Add_Char(LS, c)){
                        LS_Free(LS);
                        error_out(INTERNAL_ERROR, "Malloc Error!");
                    }
                }
                else if(c == 'x'){
                    if(!LS_Add_Char(LS, c)){
                        LS_Free(LS);
                        error_out(INTERNAL_ERROR, "Malloc Error!");
                    }
                    state = FSM_Hex;
                    break;
                }
                else if(isdigit(c)){
                    if(!LS_Add_Char(&tmp, c)){
                        LS_Free(&tmp);
                        LS_Free(LS);
                        error_out(INTERNAL_ERROR, "Malloc Error!");
                    }
                    if(!LS_Add_Char(LS, c)){
                        LS_Free(&tmp);
                        LS_Free(LS);
                        error_out(INTERNAL_ERROR, "Malloc Error!");
                    }
                    if(tmp.Lexeme_lenght == 4){
                        LS_Free(LS);
                        LS_Free(&tmp);
                        error_out(LEXEMA_SYNTAX_ERROR, "Escape symbol not recognized!");
                    }
                    if(tmp.Lexeme_lenght == 3 && (atoi(tmp.string) < 1 || atoi(tmp.string) > 255)){
                        LS_Free(LS);
                        LS_Free(&tmp);
                        error_out(LEXEMA_SYNTAX_ERROR, "Escape symbol not recognized!");
                    }
                    else if(tmp.Lexeme_lenght == 3){
                        LS->T_Type = TT_String;
                        state = FSM_String;
                        break;
                    }
                    break;
                }
                else{
                    LS_Free(LS);
                    LS_Free(&tmp);
                    error_out(LEXEMA_SYNTAX_ERROR, "Escape symbol not recognized!");
                }
                state = FSM_String;
                break;


            case(FSM_Hex):
                if(!LS_Add_Char(&tmp, c)){
                    LS_Free(&tmp);
                    LS_Free(LS);
                    error_out(INTERNAL_ERROR, "Malloc Error!");
                }
                if(!LS_Add_Char(LS, c)){
                    LS_Free(&tmp);
                    LS_Free(LS);
                    error_out(INTERNAL_ERROR, "Malloc Error!");
                }
                if(tmp.Lexeme_lenght == 2){
                    int dec = Hex_to_Dec(tmp);
                    if(dec == -1) error_out(LEXEMA_SYNTAX_ERROR, "Escape symbol not recognized!");
                    if(dec >= 1 && dec <= 255){
                        state = FSM_String;
                        break;
                    }
                    else{
                        LS_Free(&tmp);
                        LS_Free(LS);
                        error_out(LEXEMA_SYNTAX_ERROR, "Escape symbol not recognized!");
                    }
                }
                break;


            case(FSM_Dot):
                if(c == '.'){
                    LS->T_Type = TT_Concat;
                    if(!LS_Add_Char(LS, c)){
                        LS_Free(LS);
                        error_out(INTERNAL_ERROR, "Malloc Error!");
                    }
                    return 0;
                }
                else{
                    LS_Free(LS);
                    error_out(INTERNAL_ERROR, "Malloc Error!");
                }
        }   
    }
}


void Is_It_Keyword(Lexeme *LS){
    if(LS->T_Type == TT_Identifier){
        if(!strcmp(LS->string, "do")) LS->keyword = K_do;
        else if(!strcmp(LS->string, "else")) LS->keyword = K_else;
        else if(!strcmp(LS->string, "end")) LS->keyword = K_end;
        else if(!strcmp(LS->string, "function")) LS->keyword = K_function;
        else if(!strcmp(LS->string, "global")) LS->keyword = K_global;
        else if(!strcmp(LS->string, "if")) LS->keyword = K_if;
        else if(!strcmp(LS->string, "integer")) LS->keyword = K_integer;
        else if(!strcmp(LS->string, "local")) LS->keyword = K_local;    
        else if(!strcmp(LS->string, "number")) LS->keyword = K_number;
        else if(!strcmp(LS->string, "require")) LS->keyword = K_require;
        else if(!strcmp(LS->string, "return")) LS->keyword = K_return;
        else if(!strcmp(LS->string, "string")) LS->keyword = K_string;
        else if(!strcmp(LS->string, "then")) LS->keyword = K_then;
        else if(!strcmp(LS->string, "while")) LS->keyword = K_while;

        if(LS->keyword != 0){
            LS->T_Type = TT_Keyword;
            return;
        }

        if(!strcmp(LS->string, "reads") || !strcmp(LS->string, "readi") || !strcmp(LS->string, "readn") ||
            !strcmp(LS->string, "write") || !strcmp(LS->string, "tointeger") || !strcmp(LS->string, "substr")
            || !strcmp(LS->string, "ord") || !strcmp(LS->string, "chr")){

                LS->T_Type = TT_PreDefined_Func;
                return;
        }
    }
    return;
}

int Hex_to_Dec(Lexeme LS){
    int val = 0;
    int base = 1;
    for(int i = LS.Lexeme_lenght-1; i >= 0; i--){
        if(LS.string[i] >= '0' && LS.string[i] <= '9'){
            val += (((int)LS.string[i])-48) * base;
            base = base * 16;
        }
        else if(LS.string[i] >= 'A' && LS.string[i] <= 'F'){
            val += ((int)(LS.string[i]) - 55) * base;

            base = base * 16; 
        }
        else{
            return -1;
        }
    }
    return val;
}