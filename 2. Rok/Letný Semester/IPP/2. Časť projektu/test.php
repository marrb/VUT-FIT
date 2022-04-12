<?php

require_once "./test_source/html_gen.php";

//Error Codes----------------------------------------------------------
define('PARAMETER_ERROR', 10);
define('REGEX_ERROR', 11);
define('FILE_ERROR', 41);
//---------------------------------------------------------------------

//Flags----------------------------------------------------------------
class Flags{
    public static $recursive_flag = false;
    public static $directory_flag = false;
    public static $parse_file_flag = false;
    public static $int_file_flag = false;
    public static $parse_only_flag = false;
    public static $int_only_flag = false;
    public static $jexam_flag = false;
    public static $noclean_flag = false;
    public static $listfiles_flag = false;
    public static $regex_flag = false;
}
//--------------------------------------------------------------------


//Files---------------------------------------------------------------
class Files{
    public static $directory_path = NULL;
    public static $parse_file = NULL;
    public static $int_file = NULL;
    public static $jexam_file = NULL;
    public static $jexam_options_file = NULL;
    public static $list_file = NULL;
    public static $regex = NULL;
}
//--------------------------------------------------------------------


//Parameter processing------------------------------------------------
array_shift($argv); //Remove first parameter (name of the script)

foreach($argv as $param){
    if($param == "--help"){
        if(count($argv) > 1){ //If there is another parameter except parameter 'help' print error
            fprintf(STDERR, "Parameter --help cannot be combined with any other parameter!");
            exit(PARAMETER_ERROR);
        }
        else{
            print("\ntest.php is used for automatic testing of parse.php and interpret.py.\n\n"
                    ."usage: php test.php [--help] [--directory=path] [--recursive] [--parse-script=file] [--int-script=file]\n"
                    ."                    [--parse-only] [--int-only] [--jexampath=path] [--noclean] [--testlist] [--match]\n\n"
                    ."--help : prints help\n"
                    ."--directory=path : test.php will look for tests in given directory\n"
                    ."--recursive : test.php will look for tests in selected directory and its subdirectories\n"
                    ."--parse-script=file : file with script for parsing source code in IPPcode22 (default=parse.php in current directory)\n"
                    ."--int-script=file : file with script for interpreting XML representation of code in IPPcode22\n"
                    ."                    (default: interpret.py in current directory)\n"
                    ."--parse-only : only script for parsing will be tested\n"
                    ."--int-only : only script for interpreting will be tested\n"
                    ."--jexampath=path : file path to directory containing file 'jexamxml.jar' with A7Soft JExamXML tool and file with\n" 
                    ."                   configuration 'options', if not set, default file path ('/pub/courses/ipp/jexamxml/') will be used\n"
                    ."--noclean : test.php will not delete auxiliary files (file with XML output after testing parse.php .....)\n"
                    ."--testlist : test.php will get test files and test directories from specified text file (files separated by newline)\n"
                    ."--match : test.php will only test files that match specified regex\n");
            exit(0);
        }
    }
    else if(substr($param, 0, 12) == "--directory="){
        if(Flags::$listfiles_flag){
            fprintf(STDERR, "Parameter '--directory' cannot be combined with parameter '--testlist'!\n");
            exit(PARAMETER_ERROR);
        }
        Files::$directory_path = realpath(substr($param, 12));
        if(!is_dir(Files::$directory_path)){
            fprintf(STDERR, "Directory given in '--directory' parameter does not exist!\n");
            exit(FILE_ERROR);
        }
        Flags::$directory_flag = true;
    }
    else if($param == "--recursive") Flags::$recursive_flag = true;
    else if(substr($param, 0, 15) == "--parse-script="){
        Files::$parse_file = realpath(substr($param, 15));
        if(!file_exists(Files::$parse_file) || is_dir(Files::$parse_file)){
            fprintf(STDERR, "File given in '--parse-script' parameter does not exist!\n");
            exit(FILE_ERROR);
        }
        Flags::$parse_file_flag = true;
    }
    else if(substr($param, 0, 13) == "--int-script="){
        Files::$int_file = realpath(substr($param, 13));
        if(!file_exists(Files::$int_file) || is_dir(Files::$int_file)){
            fprintf(STDERR, "File given in '--int-script' parameter does not exist!\n");
            exit(FILE_ERROR);
        }
        Flags::$int_file_flag = true;
    }
    else if($param == "--parse-only"){
        if(Flags::$int_file_flag == true || Flags::$int_only_flag == true){
            fprintf(STDERR, "Parameter '--parse-only' cannot be combined with parameter '--int-script' or '--int-only'!\n");
            exit(PARAMETER_ERROR);
        }
        Flags::$parse_only_flag = true;
    }
    else if($param == "--int-only"){
        if(Flags::$parse_only_flag == true || Flags::$parse_file_flag == true || Flags::$jexam_flag == true){
            fprintf(STDERR, "Parameter '--int-only' cannot be combined with parameter '--parse-script' or '--parse-only'!\n");
            exit(PARAMETER_ERROR);
        }
        Flags::$int_only_flag = true;
    }
    else if(substr($param, 0, 12) == "--jexampath="){
        Files::$jexam_file = realpath(substr($param, 12)) ."/jexamxml.jar";
        Files::$jexam_options_file = realpath(substr($param, 12)) ."/options";
        if(!file_exists(Files::$jexam_file)){
            fprintf(STDERR, "Jexam file wasnt found in '--jexampath' directory!\n");
            exit(FILE_ERROR);
        }
        if(!file_exists(Files::$jexam_options_file)){
            fprintf(STDERR, "Options file wasnt found in '--jexampath' directory!\n");
            exit(FILE_ERROR);
        }
        Flags::$jexam_flag = true;
    }
    else if($param == "--noclean") Flags::$noclean_flag = true;
    else if(substr($param, 0, 11) == "--testlist="){
        if(Flags::$directory_flag){
            fprintf(STDERR, "Parameter '--testlist' cannot be combined with parameter '--directory'!\n");
            exit(PARAMETER_ERROR);
        }
        Files::$list_file = realpath(substr($param, 11));
        if(!file_exists(Files::$list_file) || is_dir(Files::$list_file)){
            fprintf(STDERR, "File given in '--list_file' parameter does not exist!\n");
            exit(FILE_ERROR);
        }
        Flags::$listfiles_flag = true;
    }
    else if(substr($param, 0, 8) == "--match="){
        Files::$regex = substr($param, 8);
        if(@preg_match(Files::$regex, '') === false){
            fprintf(STDERR, "Regex given in '--match' parameter is invalid!\n");
            exit(REGEX_ERROR);
        }
        Flags::$regex_flag = true;   
    }
    else{
        fprintf(STDERR, "Parameter"." '$param' not recognized!\n");
        exit(PARAMETER_ERROR);
    }
}
//--------------------------------------------------------------------


//Function for getting test files from directories--------------------
function Get_Files($direc, &$result = array()){
    $temp = scandir($direc); //Get contents of given/current directory
    $temp = array_diff($temp, array('.', '..')); //Delete not needed stuff from array
    natcasesort($temp); //Sort Files

    if(Flags::$recursive_flag == true){ //If there was an recursive parameter go through the array and get content of subdirectories....
        foreach($temp as $file){
            $path = realpath($direc . DIRECTORY_SEPARATOR . $file);
            if(is_dir($path)) Get_Files($path, $result); //If it is a directory get its files
            else{
                if(Flags::$regex_flag){ //If regex was passed check if filename matches
                    $pathinfo = pathinfo($path);
                    if(preg_match(Files::$regex, $pathinfo['filename']) && $pathinfo['extension'] == "src") $result[] = $path;
                }
                else
                    if(preg_match("/^(\S|\s)*\.(src)$/", $path)) $result[] = $path; //If it is a file with .src sufix put it in result array
            } 
        }
    }
    else{ //If there wasnt a recursive parameter, do the same but for given/current directory
        foreach($temp as $file){
            $path = realpath($direc . DIRECTORY_SEPARATOR . $file);
            if(Flags::$regex_flag){
                $pathinfo = pathinfo($path);
                if(preg_match(Files::$regex, $pathinfo['filename']) && $pathinfo['extension'] == "src") $result[] = $path;
            }
            else
                if(preg_match("/^(\S|\s)*\.(src)$/", $path)) $result[] = $path;
        }
    }

    return $result; 
}
//--------------------------------------------------------------------


//Function for getting test files from --testlist parameter file------
function Get_Files_from_file($line_array, $recurse_counter, &$result = array()){
    foreach($line_array as $file){
        $path = realpath($file); //Get real path to file (absolute)
        if(!file_exists($path)){
            fprintf(STDERR, "File '".$path."' in list-file doesnt exist!\n");
            exit(FILE_ERROR);
        }
        if(is_dir($path) && $recurse_counter == 0){ //If its a directory get its files, resurce counter makes sure that if a directory is in a directory and no --recursive parameter was passed it wont get its files
            $temp = scandir($path); //Get the directory content
            $temp = array_diff($temp, array('.', '..')); //Remove not needed paths
            foreach($temp as $index=>$file_name){
                $temp[$index] = realpath($path . DIRECTORY_SEPARATOR . $file_name); //Change the file names to absolute paths
            }
            if(Flags::$recursive_flag == true)
                Get_Files_from_file($temp, 0, $result); //If recursive parameter was passed pass recuse coutner 0 so it keeps going trough all directories
            else
                Get_Files_from_file($temp, 1, $result); //Else pass 1 as recurse counter so it wont fo trough further directories
        }
        else{
            if(Flags::$regex_flag){
                $pathinfo = pathinfo($path);
                if(preg_match(Files::$regex, $pathinfo['filename']) && $pathinfo['extension'] == "src") $result[] = $path;
            }
            else
                if(preg_match("/^(\S|\s)*\.(src)$/", $path)) $result[] = $path; //If it is a file with .src sufix put it in result array
        } 
    }

    return $result; 
}
//--------------------------------------------------------------------


//Function for testing parser.php-------------------------------------
function Test_Parser($files){
    $real_ret_code = 0; //init int

    foreach($files as $src){
        $path = pathinfo($src); //Get path info
        HTML_gen::Print_File_Name($src); //Put File name into html
        
        if(!file_exists($path['dirname']."/".$path['filename'].".rc")){ //Check if file with return code exists, if not create it and put 0
            $ret_code_file = fopen($path['dirname']."/".$path['filename'].".rc", "w");
            fwrite($ret_code_file, "0");
            fclose($ret_code_file);
        }
        else $ret_code_file = fopen($path['dirname']."/".$path['filename'].".rc", "r"); //If it exists, open it


        if(!file_exists($path['dirname'] ."/".$path['filename'].".out")) fopen($path['dirname'] ."/".$path['filename'].".out", "w"); //If .out or .in file doesnt exist, create it
        if(!file_exists($path['dirname'] ."/".$path['filename'].".in")) fopen($path['dirname'] ."/".$path['filename'].".in", "w");

        $expected_ret_code = file_get_contents($path['dirname']."/".$path['filename'].".rc", "r");//Read expected return code from file

        exec("php8.1 " . Files::$parse_file ." < $src > ".$path['dirname']."/test_out.out", $trash, $real_ret_code);//Run parser
		
        if(Flags::$parse_only_flag == true){ //If parse-only parameter was passed and return codes are 0, run jexam
            if($real_ret_code == 0 && $expected_ret_code == 0){
                exec("java -jar " .Files::$jexam_file ." ".$path['dirname']."/test_out.out" ." " .$path['dirname'] ."/".$path['filename'].".out " .$path['dirname'] ."/test_diff.xml /D " .Files::$jexam_options_file, $trash, $jexam_RC);

                if(Flags::$noclean_flag == false){//If --noclean parameter wasnt given remove test files
                    unlink($path['dirname'] ."/test_diff.xml");
                    unlink($path['dirname']."/test_out.out");
                } 
                HTML_gen::Print_Parser_Row($expected_ret_code, $real_ret_code, $jexam_RC, Flags::$parse_only_flag); //Print parser values into html table
                HTML_gen::End_Row();
            }
            else{//If return codes differ or are not 0, dont match outputs but print values into html table
                HTML_gen::Print_Parser_Row($expected_ret_code, $real_ret_code, 2, Flags::$parse_only_flag);
                HTML_gen::End_Row();
            } 
        }
        else{//If we are testing parser and interpret at the same time
            if(!in_array($expected_ret_code, [21, 22, 23]) || $expected_ret_code == 0){ //If return code is from interpret, print parser row with 0
                HTML_gen::Print_Parser_Row(0, $real_ret_code, 2, Flags::$parse_only_flag);
                Test_Interpret(NULL, $path, $expected_ret_code);//Test interpret
            }
            else{//If return code is from parser print values and end row
                HTML_gen::Print_Parser_Row($expected_ret_code, $real_ret_code, 2, Flags::$parse_only_flag); 
                HTML_gen::Print_Interpret_Row();
                HTML_gen::End_Row();
            }
        }
    }
}
//--------------------------------------------------------------------


//Function for testing interpret.py-----------------------------------
function Test_Interpret($files = NULL, $path_p = NULL, $expected_ret_code = NULL){
    if(is_null($files)){ //If files werent passed (means we are testing both parser and interpret) run interpret with parser output
        exec("python3.8 " .Files::$int_file . " --source=" . $path_p['dirname']."/test_out.out" . " --input=" . $path_p['dirname']. "/".$path_p['filename'].".in > ".$path_p['dirname']."/test_inter.out", $trash, $real_RC);

        if($expected_ret_code == 0 && $real_RC == 0){//If return codes are both 0, check if they are matching
            exec("diff " . $path_p['dirname']."/test_inter.out ". $path_p['dirname'] ."/".$path_p['filename'].".out", $trash, $diff_RC);
            HTML_gen::Print_Interpret_Row($expected_ret_code, $real_RC, $diff_RC);
        }
        else{
            HTML_gen::Print_Interpret_Row($expected_ret_code, $real_RC, 2);
        }
        HTML_gen::End_Row();


        if(Flags::$noclean_flag == false){//Clean test files if --noclean parameter wasnt given
            unlink($path_p['dirname']."/test_out.out");
            unlink($path_p['dirname']."/test_inter.out");
        } 
    }
    else{//If int-only was passed
        foreach($files as $src){//Test interpret for each file
            $path = pathinfo($src);
            HTML_gen::Print_File_Name($src);
            
            //Check if .rc file exists, if not create it
            if(!file_exists($path['dirname']."/".$path['filename'].".rc")){
                $ret_code_file = fopen($path['dirname']."/".$path['filename'].".rc", "w");
                fwrite($ret_code_file, "0");
                fclose($ret_code_file);
            } 

            $expected_ret_code = file_get_contents($path['dirname']."/".$path['filename'].".rc", "r"); //Get return code from file

            if(!file_exists($path['dirname'] ."/".$path['filename'].".out")) fopen($path['dirname'] ."/".$path['filename'].".out", "w"); //Check if .out and .in files exist, if not create it
            if(!file_exists($path['dirname'] ."/".$path['filename'].".in")) fopen($path['dirname'] ."/".$path['filename'].".in", "w");

            exec("python3.8 " .Files::$int_file . " --source=" . $src . " --input=" . $path['dirname']. "/".$path['filename'].".in > ".$path['dirname']."/test_inter.out", $trash, $real_RC);//Run interpret
            if($expected_ret_code == 0 && $real_RC == 0){ //If return codes are both 0, check if outputs are matching
                exec("diff " . $path['dirname']."/test_inter.out ". $path['dirname'] ."/".$path['filename'].".out", $trash, $diff_RC);
                HTML_gen::Print_Interpret_Row($expected_ret_code, $real_RC, $diff_RC);
                HTML_gen::End_Row();
            }
            else{
                HTML_gen::Print_Interpret_Row($expected_ret_code, $real_RC, 2);
                HTML_gen::End_Row();
            }

            if(Flags::$noclean_flag == false) unlink($path['dirname']."/test_inter.out");//Clean test files if --noclean parameter wasnt given
        }
    }
}
//--------------------------------------------------------------------


//Main function-------------------------------------------------------
function main(){
    if(Flags::$directory_flag) $dir = Files::$directory_path; //If there was a directory parameter, take it
    else if(Flags::$listfiles_flag){//If there was a listfile parameter, get its content
        $line_array = array();
        $list_file = fopen(Files::$list_file, "r");
        if($list_file){
            while(($line = fgets($list_file)) !== false) $path_array[] = trim($line);
            natcasesort($path_array);
        }
        else exit(FILE_ERROR);
        fclose($list_file);
    } 
    else $dir = realpath("."); //Else take the path of current directory

    if(Flags::$jexam_flag == false){ //If there wasnt a jexam parameter, use the default path
        Files::$jexam_file = realpath("/pub/courses/ipp/jexamxml/jexamxml.jar");
        Files::$jexam_options_file = realpath("/pub/courses/ipp/jexamxml/options");
    } 

    if(Flags::$int_file_flag == false) Files::$int_file = realpath("." . DIRECTORY_SEPARATOR . "interpret.py"); //If path to interpret.py wasnt given, use current directory
    if(!file_exists(Files::$int_file) && Flags::$parse_only_flag == false){ //If the file doesnt exist, call for an error
        fprintf(STDERR, "Interpret.py wasnt found!");
        exit(FILE_ERROR);
    }

    if(Flags::$parse_file_flag == false) Files::$parse_file = realpath("." . DIRECTORY_SEPARATOR . "parse.php");//If path to parser.py wasnt given, use current directory
    if(!file_exists(Files::$parse_file) && Flags::$int_only_flag == false){ //If the file doesnt exist, call for an error
        fprintf(STDERR, "Parser.php wasnt found!");
        exit(FILE_ERROR);
    }

    if(!Flags::$listfiles_flag)$files = Get_Files($dir); //Get files from directory if no list with files was provided
    else $files = Get_Files_from_file($path_array, 0); //Get files from file list
    
    HTML_gen::start_html(Flags::$int_only_flag, Flags::$parse_only_flag); //Start html file
    
    if(Flags::$parse_only_flag == true || (Flags::$parse_only_flag == false && Flags::$int_only_flag == false)) Test_Parser($files);
    if(Flags::$int_only_flag == true) Test_Interpret($files);
    HTML_gen::end_html(); //End html document
    exit(0);
}
//--------------------------------------------------------------------

main();
?>