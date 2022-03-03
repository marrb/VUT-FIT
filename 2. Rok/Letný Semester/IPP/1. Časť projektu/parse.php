<?php

//require_once "xml.php";
require_once "./parse_source/instruction.php";
require_once "./parse_source/error.php";
require_once "./parse_source/xml_gen.php";


//Parameter processing-------------------------------------------------
array_shift($argv); //Remove first parameter (name of the script)

foreach($argv as $param){
    if($param == "--help"){
        if(count($argv) > 1){ //If there is another parameter except parameter 'help' print error
            fprintf(STDERR, "Parameter --help cannot be combined with any other parameter!");
            exit(ErrorCodes::PARAMETER_ERROR);
        }
        else{
            print("\nParse.php loads source code in IPPcode22 from standart input,\nchecks it for lexical and syntactic errors and prints XML\nrepresentation of the code to standart output.\n\nusage: php parse.php [--help]\n\n--help prints help\n");
            exit(0);
        }
    }
}

//--------------------------------------------------------------------


//Header check--------------------------------------------------------
function check_header(){
    global $token;

    $token = Lexer::getToken();
    if($token->Type != "header"){ //If header is missing, print an error
        fprintf(STDERR, "Header syntax error or header missing!");
        exit(ErrorCodes::LEXICAL_OR_SYNTAX_ERROR);
    }
    $token = Lexer::getToken();
    if($token->b_newline != true){ //Checks for EOL after header
        fprintf(STDERR, "Missing EOL after header!");
        exit(ErrorCodes::LEXICAL_OR_SYNTAX_ERROR);
    }
}
//--------------------------------------------------------------------


//Instruction get-----------------------------------------------------
function Find_Instruction(&$index){
    global $token;
    $found = false;

    foreach(Instruction_array::$inst_arr as $key => $instr){
        //Looks for instruction in instruction array in "instruction.php"
        if(strtoupper($token->dataString) == $instr->inst){
            //If found, then store the index number
            $found = true;
            $index = $key;
            break;
        }
    }
    if($found != true){
        //If not found print an error
        fprintf(STDERR, "Instruction not recognized!");
		exit(ErrorCodes::INVALID_OPERATION_CODE);
    }

    xml_gen::Add_Instruction($token);
}
//--------------------------------------------------------------------


//Get arguments-------------------------------------------------------
function Find_Arguments($index){
    global $token;
    $token = Lexer::getToken();
    $arg = Instruction_array::$inst_arr[$index]->arg_1; //Gets the type of first argument of current instruction
    $arg_counter = 1;

    while($token->b_newline != true && $token->b_end_of_file != true){
        if($arg == "var"){
            //If the supposed type is var and the argument being processed is not of type var, print an error
            if($token->Type !== "var"){
                fprintf(STDERR, "Argument '%s' is of wrong type!\n", $token->dataString);
                exit(ErrorCodes::LEXICAL_OR_SYNTAX_ERROR);
            }
        }
        else if($arg == "symbol"){
            //If the supposed type is symbol and the argument being processed is not of type var,int,string,bool or nil print an error
            if($token->Type != "var" && $token->Type != "int" && $token->Type != "string" && $token->Type != "bool" && $token->Type != "nil"){
                fprintf(STDERR, "Argument '%s' is of wrong type!\n", $token->dataString);
                exit(ErrorCodes::LEXICAL_OR_SYNTAX_ERROR);
            }
        }
        else if($arg == "label"){
            //If the supposed type is label and the argument being processed is not of type keyword_or_label print an error
            if($token->Type != "keyword_or_label"){
                fprintf(STDERR, "Argument '%s' is of wrong type!\n", $token->dataString);
                exit(ErrorCodes::LEXICAL_OR_SYNTAX_ERROR);
            }
            /*else{
                foreach(Instruction_array::$inst_arr as $instr){
                    if(strtoupper($token->dataString) == $instr->inst){
                        fprintf(STDERR, "Label '%s' has the same name as an instrcution!\n", $token->dataString);
                        exit(ErrorCodes::LEXICAL_OR_SYNTAX_ERROR);
                    }
                }
            }*/
        }
        else if($arg == "type"){
            //If the supposed type is type and the argument being processed is not containing string int,string or bool print an error
            if($token->dataString !== "int" && $token->dataString !== "string" && $token->dataString !== "bool"){
                fprintf(STDERR, "Argument '%s' is of wrong type!\n", $token->dataString);
                exit(ErrorCodes::LEXICAL_OR_SYNTAX_ERROR);
            }
            $token->Type = "type";
        }
        else if($arg == NULL){
            //If there isnt supposed to be another argument and we got another one on stdin, then print an error
            fprintf(STDERR, "Too many arguments in instruction!\n", $token->dataString);
            exit(ErrorCodes::LEXICAL_OR_SYNTAX_ERROR);
        }
        
        xml_gen::Add_Argument($token); //If everything is ok, then put argument in xml document
        $arg_counter++;
        $token = Lexer::getToken();

        if($arg_counter == 2) $arg = Instruction_array::$inst_arr[$index]->arg_2; //Gets the supposed type of second argument
        else if($arg_counter == 3) $arg = Instruction_array::$inst_arr[$index]->arg_3; //Gets the supposed type of third argument
        else if($arg_counter == 4) $arg = NULL; //There isnt any instruction with 4 arguments
    }

    if($arg_counter < 3 && $arg != NULL){
        //If supposed type of argument is not NULL, and we encountered EOL, instruction is missing an argument
        fprintf(STDERR, "Instruction is missing argument(s)!\n");
        exit(ErrorCodes::LEXICAL_OR_SYNTAX_ERROR);
    }
}
//--------------------------------------------------------------------


//Main Function for checking syntax-----------------------------------
function check_syntax($token){
    global $token;
    $index = 0;

    //Fills instruction array in "instruction.php" with instructions
    Instruction_array::fill_inst_array();
    
    while($token->b_end_of_file != true){
        Find_Instruction($index);
        Find_Arguments($index);
    }
    xml_gen::XML_End_Document();
    //Ends the XML document and prints it to "output.xml"
    return 0;
}
//--------------------------------------------------------------------


//Main Function-------------------------------------------------------
function main(){
    ini_set('display_errors', 'stderr');
    global $token;

    check_header();

    xml_gen::xml_init(); //Starts the xml document
    check_syntax($token);

    return 0;
}
//--------------------------------------------------------------------

$token = new Token;
main();
?>