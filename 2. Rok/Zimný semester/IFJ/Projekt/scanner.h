/*
Project: Compiler for language IFJ21

File: scanner.h

AUTHORS:
Martin Bublavý <xbubla02@stud.fit.vutbr.cz>
Igor Pszota <xpszot00@stud.fit.vutbr.cz>

*/

#include <stdbool.h>
#include <stdio.h>

//Keywords
typedef enum{
    K_not_a_keyword,
    K_do,
    K_else,
    K_end,
    K_function,
    K_global,
    K_if,
    K_integer,
    K_local,
    K_nil,
    K_number,
    K_require,
    K_return,
    K_string,
    K_then,
    K_while
}  keyword;

typedef enum{
    TT_Identifier,
    TT_Keyword,
    TT_End_Of_Line,
    TT_Hashtag,
    TT_End_Of_File,
    TT_Escape,
    TT_Colon,
    TT_PreDefined_Func,
    
    TT_Equals,
    TT_Not_Equal,
    TT_Bigger_Than,
    TT_Less_Than,
    TT_BT_Equals,
    TT_LT_Equals,
    
    TT_Assign,
    TT_Add,
    TT_Subtract,
    TT_Multiply,
    TT_Divide,
    TT_INT_Divide,
    TT_Concat,

    TT_Integer,
    TT_String,
    TT_Double,
    TT_Exponent,

    TT_Left_Bracket,
    TT_Right_Bracket,
    TT_Comma,
    TT_Comment,
    TT_Block_Comment,
    TT_Block_Comment_End
} Token_Type;

typedef enum{
    FSM_idle,
    FSM_Identifier_Keyword, //Starts with a letter
    FSM_Integer, //Starts with a digit
    FSM_Double_Point,
    FSM_Double,
    FSM_Exponent,
    FSM_Exponent_Sign,
    FSM_Exponent_Finish,
    FSM_Minus,
    FSM_Comment,
    FSM_Block_Comment,
    FSM_Block_Comment_End,
    FSM_Division,
    FSM_Less_Than,
    FSM_More_Than,
    FSM_Not_Equal,
    FSM_Assign,
    FSM_String,
    FSM_Escape,
    FSM_Dot,
    FSM_Comment_String,
    FSM_Block_Comment_String,
    FSM_Block_Comment_End_End,
    FSM_Hex
} FSM_STATE;

//Struct for holding Lexemes
typedef struct {
    char *string;
    int Lexeme_lenght;
    int Alloc_lenght;
    Token_Type T_Type;
    keyword keyword;
} Lexeme;

//Initializes Lexeme
bool LS_init(Lexeme *LS);

void LS_Free(Lexeme *LS);

bool LS_Add_Char(Lexeme *LS, char c);

void set_source_stream(FILE *source);

void LS_Clear(Lexeme *LS);

int get_token(Lexeme *LS);

void Is_It_Keyword(Lexeme *LS);

void reallocSequence(char ** sequence, int currentMemory, int * maxMemory);

int Hex_to_Dec(Lexeme LS);