<?php
require_once "Lexer.php";

//Instruction class/object----------------------------------------------
class Instruction{
    public $inst;
    public $arg_1 = NULL;
    public $arg_2 = NULL;
    public $arg_3 = NULL;

    function __construct($inst, $arg1, $arg2, $arg3){
		$this->inst = $inst;
		$this->arg_1 = $arg1;
		$this->arg_2 = $arg2;
		$this->arg_3 = $arg3;
	}
}
//---------------------------------------------------------------------


//Instruction array----------------------------------------------------
class Instruction_array{
    public static $inst_arr = array();

    public static function fill_inst_array(){
        array_push(self::$inst_arr, new Instruction("MOVE", "var", "symbol", NULL));
        array_push(self::$inst_arr, new Instruction("CREATEFRAME", NULL, NULL, NULL));
        array_push(self::$inst_arr, new Instruction("PUSHFRAME", NULL, NULL, NULL));
        array_push(self::$inst_arr, new Instruction("POPFRAME", NULL, NULL, NULL));
        array_push(self::$inst_arr, new Instruction("DEFVAR", "var", NULL, NULL));
        array_push(self::$inst_arr, new Instruction("CALL", "label", NULL, NULL));
        array_push(self::$inst_arr, new Instruction("RETURN", NULL, NULL, NULL));
        array_push(self::$inst_arr, new Instruction("PUSHS", "symbol", NULL, NULL));
        array_push(self::$inst_arr, new Instruction("POPS", "var", NULL, NULL));
        array_push(self::$inst_arr, new Instruction("ADD", "var", "symbol", "symbol"));
        array_push(self::$inst_arr, new Instruction("SUB", "var", "symbol", "symbol"));
        array_push(self::$inst_arr, new Instruction("MUL", "var", "symbol", "symbol"));
        array_push(self::$inst_arr, new Instruction("IDIV", "var", "symbol", "symbol"));
        array_push(self::$inst_arr, new Instruction("LT", "var", "symbol", "symbol"));
        array_push(self::$inst_arr, new Instruction("GT", "var", "symbol", "symbol"));
        array_push(self::$inst_arr, new Instruction("EQ", "var", "symbol", "symbol"));
        array_push(self::$inst_arr, new Instruction("AND", "var", "symbol", "symbol"));
        array_push(self::$inst_arr, new Instruction("OR", "var", "symbol", "symbol"));
        array_push(self::$inst_arr, new Instruction("NOT", "var", "symbol", "symbol"));
        array_push(self::$inst_arr, new Instruction("INT2CHAR", "var", "symbol", NULL));
        array_push(self::$inst_arr, new Instruction("STRI2INT", "var", "symbol", "symbol"));
        array_push(self::$inst_arr, new Instruction("READ", "var", "type", NULL));
        array_push(self::$inst_arr, new Instruction("WRITE", "symbol", NULL, NULL));
        array_push(self::$inst_arr, new Instruction("CONCAT", "var", "symbol", "symbol"));
        array_push(self::$inst_arr, new Instruction("STRLEN", "var", "symbol", NULL));
        array_push(self::$inst_arr, new Instruction("GETCHAR", "var", "symbol", "symbol"));
        array_push(self::$inst_arr, new Instruction("SETCHAR", "var", "symbol", "symbol"));
        array_push(self::$inst_arr, new Instruction("TYPE", "var", "symbol", NULL));
        array_push(self::$inst_arr, new Instruction("LABEL", "label", NULL, NULL));
        array_push(self::$inst_arr, new Instruction("JUMP", "label", NULL, NULL));
        array_push(self::$inst_arr, new Instruction("JUMPIFEQ", "label", "symbol", "symbol"));
        array_push(self::$inst_arr, new Instruction("JUMPIFNEQ", "label", "symbol", "symbol"));
        array_push(self::$inst_arr, new Instruction("EXIT", "symbol", NULL, NULL));
        array_push(self::$inst_arr, new Instruction("DPRINT", "symbol", NULL, NULL));
        array_push(self::$inst_arr, new Instruction("BREAK", NULL, NULL, NULL));
    }
}
//---------------------------------------------------------------------

?>