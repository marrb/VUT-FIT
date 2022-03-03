<?php

class xml_gen{
    private static $xml;
    private static $arg_count = 0;
    private static $inst_counter = 0;


    //Init xml document--------------------------------------------------
    public static function xml_init(){
        self::$xml = new XMLWriter();
        self::$xml->openMemory();
        self::$xml->startDocument('1.0', 'UTF-8');
        self::$xml->setIndent(true);
        self::$xml->setIndentString("    ");

        self::$xml->startElement('program');
        self::$xml->startAttribute('language');
        self::$xml->text('IPPcode22');
        self::$xml->endAttribute();
    }
    //-------------------------------------------------------------------


    //Add Instruction----------------------------------------------------
    public static function Add_Instruction(Token $token){
        if(self::$inst_counter != 0) self::$xml->endElement();

        self::$xml->startElement('instruction');

        self::$inst_counter++;
        self::$arg_count = 0;

        self::$xml->startAttribute('order');
        self::$xml->text(self::$inst_counter);
        self::$xml->endAttribute();
        self::$xml->startAttribute('opcode');
        self::$xml->text(strtoupper($token->dataString));
        self::$xml->endAttribute();
    }
    //-------------------------------------------------------------------


    //Add Argument-------------------------------------------------------
    public static function Add_Argument(Token $token){
        self::$arg_count++;

        self::$xml->startElement('arg'.self::$arg_count);
        self::$xml->startAttribute('type');
        if($token->Type == "keyword_or_label") self::$xml->text("label");
        else self::$xml->text($token->Type);
        self::$xml->endAttribute();
        if($token->Type == "var" || $token->Type == "type" || $token->Type == "keyword_or_label") self::$xml->text($token->dataString);
        else self::$xml->text(substr($token->dataString, strpos($token->dataString, "@")+1));
        self::$xml->endElement();
    }
    //-------------------------------------------------------------------


    //End XML Document---------------------------------------------------
    public static function XML_End_Document(){
        self::$xml->endElement();
        self::$xml->endDocument();
        echo self::$xml->outputMemory();
    }
    //-------------------------------------------------------------------
}

?>