<?php
require_once "./parse_source/error.php";

//Class/Object for token----------------------------
class Token{
    public $Type;
    public $dataString;
    public $b_newline;
    public $b_end_of_file;
}
//--------------------------------------------------


class Lexer{
    private static $c = "char";
    private static $first_char = true;

    //ascii codes-----------------------------------
    private static $space = 32;
    private static $newline = 10;
    private static $newline_win = 13;
    private static $EOF = 0;
    private static $hashtag = 35;
    private static $b_EOF = false;
    private static $b_EOL = false;
    //----------------------------------------------

    //Skips whitespaces and comments-----------------------------------------------------------------
    private static function skip_whitespaces(){
        if(self::$first_char == true){
            self::$c = fgetc(STDIN);
            self::$first_char = false;
        }
        
        while(ord(self::$c) == self::$space || ord(self::$c) == self::$newline || ord(self::$c) == self::$EOF || ord(self::$c) == self::$newline_win || ord(self::$c) == self::$hashtag){
            if(ord(self::$c) == self::$EOF){
                self::$b_EOF = true;
                return;
            }
            else if(ord(self::$c) == self::$hashtag){
                while(ord(self::$c) != self::$newline && ord(self::$c) != self::$newline_win && ord(self::$c) != self::$EOF){
                    self::$c = fgetc(STDIN);
                }
                if(ord(self::$c) == self::$EOF) self::$b_EOF = true;
                else self::$b_EOL = true;
            }
            else if(ord(self::$c) == self::$newline || ord(self::$c) == self::$newline_win){
                self::$b_EOL = true;
            }
            self::$c = fgetc(STDIN);
        }
        return;
    }
    //----------------------------------------------------------------------------------------------


    //Gets the token string from STDIN-------------------------------------
    private static function get_Data(){
        $string = "";

        while(ord(self::$c) != self::$space && ord(self::$c) != self::$newline && ord(self::$c) != self::$EOF && ord(self::$c) != self::$newline_win && ord(self::$c) != self::$hashtag){
            $string .= self::$c;
            self::$c = fgetc(STDIN);
        }
        return $string;
    }
    //----------------------------------------------------------------------


    //Main function for getting the token-----------------------------------
    public static function getToken(){
        $token = new Token;
        $token->b_newline = false;
        self::$b_EOL = false;

        self::skip_whitespaces();
        if(self::$b_EOF == true){
            $token->b_end_of_file = true;
            return $token;
        }
        if(self::$b_EOL == true){
            $token->b_newline = true;
        }

        $token->dataString = self::get_Data();
        
        //Gets the token type from the string using pattern matching and returns the whole token
        if(preg_match("/^([a-zA-z]|[\_\-\$\&\%\*\!\?])(\w|[\_\-\$\&\%\*\!\?])*$/", $token->dataString)){
            $token->Type = "keyword_or_label";
        }
        else if(!strcmp(strtoupper($token->dataString), ".IPPCODE22")){
            $token->Type = "header";
        }
        else if(preg_match("/^string@(([^\s\#\\\\]|\\\\[0-9]{3})*)$/", $token->dataString)){
            $token->Type = "string";
        }
        else if(preg_match("/^int@([+\-]?)([0-9])+$/", $token->dataString)){
            $token->Type = "int";
        }
        else if(preg_match("/^bool@(true|false)$/", $token->dataString)){
            $token->Type = "bool";
        }
        else if(preg_match("/^nil@nil$/", $token->dataString)){
            $token->Type = "nil";
        }
        else if(preg_match("/^(GF|LF|TF)@([a-zA-Z]|[\_\-\$\&\%\*\!\?])(\w|[\_\-\$\&\%\*\?\!])*$/", $token->dataString)){
            $token->Type = "var";
        }
        else{
            fprintf(STDERR, "Token type not recognized!\n");
            exit(ErrorCodes::LEXICAL_OR_SYNTAX_ERROR);
        }

        return $token;
    }
    //---------------------------------------------------------------------
}
?>