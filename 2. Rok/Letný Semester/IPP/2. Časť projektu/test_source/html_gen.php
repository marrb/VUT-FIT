<?php

class HTML_gen{

public static $tick = "<td style=\"color:#00FF00;\" >&#10004;</td>"; //green tick symbol
public static $slash = "<td style=\"color:white;\" ><b>/</b></td>";//white slash symbol (test wasnt conducted)
public static $cross = "<td style=\"color:#FF0000;\" >&#10005;</td>"; //red cross symbol

public static $passed = 0; //number of passed tests
public static $failed = 0; //number of failed tests
public static $all_passed = "NO";


//Start HTML-----------------------------------------------------------------
public static function start_html($int_only , $parse_only){
    //Setup the start of the html file
    echo "<!DOCTYPE html>
<html lang=\"en\">
<head>
        <meta charset=\"UTF-8\">
        <title>Test results for IPPcode22</title>
        <style>
            table, th, td {
                border: 2px solid black;
                border-collapse: collapse;
                text-align:center;
                margin: auto;
                margin-bottom: 1.7cm;
            }
            table thead{
				background-color: #393939;
				color: #00ad5f;
			}
            table thead td{
                padding: 10px 10px 10px 10px;
                font-weight: bold;
            }
            table tbody td{
                padding: 10px 10px 10px 10px;
				background-color: #222222;
				color: #808080;
            }
            .footer {
                position: fixed;
                left: 0;
                bottom: 0;
                width: 100%;
                background-color: #88B04B;
                opacity: 0.8;
                text-align: center;
                font-weight: bold;
                display: grid;
                grid-template-rows: auto;
                grid-template-columns: auto auto auto;
                font-size: 20px;
            }
            body{
                margin: 0;
            }
        </style>
</head>
<body>
    <table>
        <thead>
            <tr>
                <td rowspan=\"3\">Tested File</td>";
    
    //Change the html table structure acording to input parameters
    if($parse_only == true){
        echo "  <td colspan=\"3\">Parser</td>
                <td rowspan=\"3\">Overall</td>
            </tr>
            <tr>
                <td colspan=\"2\">Return Code</td>
                <td rowspan=\"2\">Matching XML</td>
            </tr>
            <tr>
                <td>Expected</td>
                <td>Real</td>
            </tr>";
    } 
    else if($int_only == true){
        echo "  <td colspan=\"3\">Interpret</td>
                <td rowspan=\"3\">Overall</td>
            </tr>
            <tr>
                <td colspan=\"2\">Return Code</td>
                <td rowspan=\"2\">Matching OUT</td>
            </tr>
            <tr>
                <td><b>Expected</b></td>
                <td><b>Real</b></td>
            </tr>";
    } 
    else{
        echo "  <td colspan=\"3\">Parser</td>
                <td colspan=\"3\">Interpret</td>
                <td rowspan=\"3\">Overall</td>
            </tr>
            <tr>
                <td colspan=\"2\">Return Code</td>
                <td rowspan=\"2\">Matching XML</td>
                <td colspan=\"2\">Return Code</td>
                <td rowspan=\"2\">Matching OUTS</td>
            </tr>
            <tr>
                <td>Expected</td>
                <td>Real</td>
                <td>Expected</td>
                <td>Real</td>
            </tr>";
    }
    
    echo "
        </thead>
        <tbody>";
}
//---------------------------------------------------------------------------



//Print currently tested file into table-------------------------------------
public static function Print_File_Name($filename){
    echo "  
            <tr>
                <td>".$filename."</td>";
}
//---------------------------------------------------------------------------



//Print values into parser part of the table--------------------------------
public static function Print_Parser_Row($expected_RC_parser, $real_RC_parser, $matching_xml, $parse_only){
    if($matching_xml == 0) $symbol = self::$tick; //tick
    else if($matching_xml == 2) $symbol = self::$slash; // (/)
    else $symbol = self::$cross; // cross

    if(($expected_RC_parser == 0 && $real_RC_parser == 0 && $matching_xml == 0)||
        ($expected_RC_parser == $real_RC_parser && $expected_RC_parser != 0) ||
        ($expected_RC_parser == 0 && $real_RC_parser == 0 && $matching_xml == 2)) $overall = self::$tick;
    else $overall = self::$cross;

    echo "      
                <td>".$expected_RC_parser."</td>
                <td>".$real_RC_parser."</td>
                ".$symbol;

    if($parse_only == True){
        echo " 
                ".$overall;
        if($overall == self::$tick) self::$passed++;
        else self::$failed++;
    } 
}
//---------------------------------------------------------------------------



//Print values into interpret part of the table------------------------------
public static function Print_Interpret_Row($expected_RC_inter = NULL, $real_RC_inter = NULL, $matching_OUT = NULL){
    if(is_null($expected_RC_inter)){
        echo "  
                <td>-</td>
                <td>-</td>
                <td>-</td>
                ".self::$cross;
        self::$failed++;
        return;
    }
    if($matching_OUT == 0) $symbol = self::$tick; //tick
    else if($matching_OUT == 2) $symbol = self::$slash; // (/)
    else $symbol = self::$cross; // cross

    if(($expected_RC_inter == 0 && $real_RC_inter == 0 && $matching_OUT == 0)||
        ($expected_RC_inter == $real_RC_inter && $expected_RC_inter != 0)){
            $overall = self::$tick;
            self::$passed++;
        } 
    else{
        $overall = self::$cross;
        self::$failed++;
    } 
    $expected_RC_inter = trim(preg_replace('/\s+/', ' ', $expected_RC_inter)); //Strip newlines (for some reason it had newline char on the end)

    echo "     
                <td>".$expected_RC_inter."</td>
                <td>".$real_RC_inter."</td>
                ".$symbol."
                ".$overall;
}
//---------------------------------------------------------------------------



//End table row--------------------------------------------------------------
public static function End_Row(){
    echo "  
            </tr>";
}
//---------------------------------------------------------------------------



//End table and html file and show number of passed and failed tests---------
public static function end_html(){
    if(self::$failed == 0) self::$all_passed = "YES";
    echo "
        </tbody>
    </table>
    <div class=\"footer\">
        <div class=\"item\">
	        <p>Passed: ".self::$passed."</p>
        </div>
        <div class=\"item\">
	        <p>Failed: ".self::$failed."<p>
        </div>
        <div class=\"item\">
	        <p>All passed: ".self::$all_passed."<p>
        </div>
	</div>
</body>
</html>";
}
//---------------------------------------------------------------------------
}
?>