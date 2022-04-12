import sys
import os
import io
from textwrap import dedent as dedent
import xml.etree.ElementTree as ET

from interpret_source.error import Error as ER
from interpret_source.Instuction import Instruction as Instruction
from interpret_source.Instuction import Argument as Argument
from interpret_source.variable import var as var
from interpret_source.variable import frame as frame
from interpret_source.variable import symbol as symbol
from interpret_source.variable import label as label
from interpret_source.stack import stack as ST

input_stream = io.TextIOWrapper(sys.stdin.buffer, encoding='utf-8') #set STDIN encoding to utf-8

def sort_func(element): #Sorting function for sorting instructions and arguments
    return element.order


#Classes-------------------------------------------
class Flags:
    input_flag = False
    source_flag = False

class File_paths:
    input_path = ""
    source_path = ""

class Files:
    source_string = ""
    input_string = None
#--------------------------------------------------


#Parameter processing------------------------------
args = sys.argv

help_msg = """
interpret.py loads XML representation of program in IPPcode22, interprets it and generates output.

usage: python interpret.py [--help] [--source=file] [--input=file]

--help prints help
--source=file file with XML representation of source code
--input=file file with inputs for interpretation
"""

i = 1
args_len = len(args)
while(i < args_len):
    if(args[i] == "--help"):
        if(args_len > 2):
            ER.error_out(ER.Parameter_error, "Parameter --help cannot be combined with any other parameter!\n")

        
        print(help_msg)
        sys.exit(0)
    
    elif(args[i][:9] == "--source="):
        Flags.source_flag = True
        File_paths.source_path = os.path.abspath(args[i][9::])

    elif(args[i][:8] == "--input="):
        Flags.input_flag = True
        File_paths.input_path = os.path.abspath(args[i][8::])
    
    else:
        ER.error_out(ER.Parameter_error, "Error: Parameter not recognized!\n")

    i += 1

if(Flags.source_flag == False and Flags.input_flag == False):
    ER.error_out(ER.Parameter_error, "Error: Please specify --source or/and --input parameter!\n")
    
#--------------------------------------------------



#Load files from stdin parameters------------------
if(Flags.source_flag == True): #Source argument was passed
    if(not os.path.isfile(File_paths.source_path)):
        ER.error_out(ER.input_open_file_error, "Error: Parameter --source contains invalid file!\n")
    
    try:
        tree = ET.parse(File_paths.source_path) #Try creating XML tree from file
    except ET.ParseError:
        ER.error_out(ER.xml_format_error, "Error: XML file is not well formed!\n")
    

else: #Source argument wasnt passed
    Files.source_string = input_stream.read() #Get source from stdin
    try:
        tree = ET.fromstring(Files.source_string) #Try creating XML tree
    except ET.ParseError: 
        ER.error_out(ER.xml_format_error, "Error: XML is not well formed!\n")



if(Flags.input_flag == True): #Input argument was passed
    if(not os.path.isfile(File_paths.input_path)): #Check if the file exists
        ER.error_out(ER.input_open_file_error, "Error: Parameter --input contains invalid file!\n")
    input_file = open(File_paths.input_path) #Open the file
    Files.input_string = input_file.read().splitlines() #Get the contents of the file into an array
    input_file.close()
#--------------------------------------------------


#Load Instructions into list and sort them---------
def Get_Instructions(root):
    Instruction_list = [] #List for instructions
    arg_list = [] #List for arguments
    order_list = [] #List for order numbers, checks if there isnt instructions with the same order number
    arg_list_check = [] #List for checking if there arent more arguments with the same order (arg1, arg1 ....)

    for instr in root:
        if(instr.tag != "instruction"):
            ER.error_out(ER.xml_structure_error, "Error: Invalid XMl structure!\n")

        if(instr.get("opcode") is None or instr.get("order") is None or not str.isnumeric(instr.get("order")) or int(instr.get("order")) < 1):
                ER.error_out(ER.xml_structure_error, "Error: Invalid XMl structure!\n")

        for attrib in instr.attrib:
            if(attrib not in ("opcode", "order")):
                ER.error_out(ER.xml_structure_error, "Error: Invalid XMl structure!\n")

        for arg in instr:
            if(arg.tag not in ("arg1", "arg2", "arg3")):
                ER.error_out(ER.xml_structure_error, "Error: Invalid XMl structure!\n")

            for arg_at in arg.attrib:
                if(arg_at != "type"):
                    ER.error_out(ER.xml_structure_error, "Error: Invalid XMl structure!\n")

            arg_list.append(Argument(arg.get("type"), int(arg.tag[3:]), arg.text)) #Put argument into the argument list
            arg_list_check.append(arg.tag) #Put argument tag into checking list


        arg_list.sort(key=sort_func) #Sort arguments acording to their order number

        if((len(arg_list) >= 1 and "arg1" not in arg_list_check) or (len(arg_list) >= 2 and "arg2" not in arg_list_check)): #Check if there arent more arguments with the swame order
            ER.error_out(ER.xml_structure_error, "Error: Invalid XMl structure!\n")

        Instruction_list.append(Instruction(instr.get("opcode"), instr.get("order"), arg_list)) #Put instruction in instruction list
        order_list.append(instr.get("order"))
        arg_list = [] #Clear arguments list
        arg_list_check = []

    
    if len(order_list) != len(set(order_list)): #Check if there isnt more instructions with the same order number
        ER.error_out(ER.xml_structure_error, "Error: Invalid XMl structure!\n")


    Instruction_list.sort(key=sort_func) #Sort instructions acording to their order number

    return Instruction_list
#--------------------------------------------------


#Check if var is defined---------------------------
def Check_Var(frame, name, frame_stack = None, GF=None, TF=None):
    #If only first 2 parameters are passed, var is being created, check for var with the same name
    if(frame_stack is None and GF is None and TF is None):
        for var in frame.var_list:
            if(var.name == name):
                ER.error_out(ER.semantic_error, "Redefinition of a variable!\n")
        return

    #Else var is being used, check if it exists in its frame (if var is found return it)
    if(frame == "GF"):
        for var in GF.var_list:
            if(var.name == name): #Look for var in GF
                return var

    elif(frame == "LF"):
        if(not(temp := frame_stack.top())): #If LF doesnt exist error
            ER.error_out(ER.frame_existence_error, "No Local Frame defined!\n")
        
        for var in temp.var_list:
            if(var.name == name): #Look for var in LF
                return var

    elif(frame == "TF"):
        if(TF is None): #If TF doesnt exist
            ER.error_out(ER.frame_existence_error, "No Temporary Frame defined!\n")

        for var in TF.var_list:
            if(var.name == name): #Look for var in TF
                return var

    ER.error_out(ER.var_not_defined, "Variable not defined!\n") #Var wasnt found
#--------------------------------------------------


#Check if types are matching------------------------
def Check_Types(type_source, type_source2, type_to_check):
#Check if the types match specified type (type to check) and check if its not empty
    if(type_source is None or type_source2 is None): ER.error_out(ER.missing_value, "Missing value!\n")

    if(type_source not in type_to_check or type_source2 != type_source):
        ER.error_out(ER.operand_type_error, "Types not matching!\n")
#--------------------------------------------------


#Get instruction arguments as var (if its var) or symbol (if its symbol)
def Get_Vars(instruction, start, frame_stack, GF, TF):
    end = len(instruction.arguments)
    result = []
    i = start

    while(i <= end):
        if(instruction.get_arg_type(i) == "var"):
            #If its var, check if it exists and append it to result list
            result.append(Check_Var(instruction.get_var_frame(i), instruction.get_var_name(i), frame_stack, GF, TF))
        else:
            #If its a symbol (int@25 .... ) append it to result list       
            result.append(symbol(instruction.get_arg_type(i), instruction.get_arg_content(i)))

        if(result[-1].type == "float"): #If the type was float, change the value to decadic form
            result[-1].value = Get_Float(result[-1].value)

        #Check if the value is set
        if(result[-1].value is None and instruction.opcode != "TYPE"):
            ER.error_out(ER.missing_value, instruction.opcode + ": Missing value!\n")

        i += 1
    return result
#------------------------------------------------------------------------


#Check if instructian has the correct number of args---------------------
def Check_Args(instruction, number_of_args):
    if(len(instruction.arguments) != number_of_args):
        ER.error_out(ER.xml_structure_error, "Error: Invalid XMl structure!\n")
#------------------------------------------------------------------------


#Check the value of float and return hex representation------------------
def Get_Float(value, return_type = None):
    try: result = float(value) #Try turning value into float
    except ValueError: #If it fails try turning it into float value from hexadecimal form
        try: result = float.fromhex(value)
        except ValueError: ER.error_out(ER.operand_wrong_value, "FLOAT: Wrong value!\n") #If all fails call error

    if(return_type == "nohex"): return result #If nohex was passed return decadic float value
    return str(float.hex(result)) #Else return hexadecimal form
#------------------------------------------------------------------------



#Process instructions----------------------------------------------------
def Process_Instructions(instr_list):
    GF = frame() #GF frame
    TF = None    #TF frame
    frame_stack = ST()
    call_stack = ST()
    data_stack = ST() #Stack for PUSH, POP ...
    label_list = [] #List of defined labels
    instr_num = 0 #Number of processed instructions
    line_num = 0 #Line number in input file (if given)
    list_len = None #Number of lines in input file (if given)

    if(Flags.input_flag == True): list_len = len(Files.input_string) - 1

    instr_counter = 0 #index of instruction being processed

    #Scan instructions for labels a add them to label list
    for instr in instr_list:
        if(instr.opcode == "LABEL"):
            Check_Args(instr,1)
            name = instr.get_arg_content(1)

            for lab in label_list: #If label with the same name was already added call for an error
                if(lab.name == name):
                    ER.error_out(ER.semantic_error, "LABEL: Label with the same name already exists!\n")

            label_list.append(label(name, instr_counter))
            
        instr_counter += 1


    instr_list_len = len(instr_list)
    instr_counter = 0
    while instr_counter < instr_list_len: #Go trough all the instructions
        instr = instr_list[instr_counter]

        if(instr.opcode == "LABEL"):
            #If instruction is label skip it (already processed before everything else)
            instr_num += 1 
            instr_counter += 1
            continue


        elif(instr.opcode == "CREATEFRAME"):
            Check_Args(instr,0)
            if(TF is None): #If frame doesnt exist, create it
                TF = frame()
            else: #Else clear its var list
                TF.var_list = []


        elif(instr.opcode == "PUSHFRAME"):
            Check_Args(instr,0)
            if(TF is None): #If frame doest exist, error
                ER.error_out(ER.frame_existence_error, "PUSHFRAME: No Temporary Frame exists!\n")

            frame_stack.stack.append(frame(TF.var_list)) #Else append it to frame stack
            TF = None


        elif(instr.opcode == "POPFRAME"):
            Check_Args(instr,0)
            temp = frame_stack.ptop(instr.opcode, ER.frame_existence_error) #Get top from frame stack

            if(TF is None): TF = frame() #If TF doesnt exist, create it
            TF.var_list = temp.var_list #Fill TF with LF var list


        elif(instr.opcode == "DEFVAR"):
            Check_Args(instr, 1) #Check number of args
            fr = instr.get_var_frame(1) #var frame
            name = instr.get_var_name(1)#var name

            if(fr == "GF"):
                Check_Var(GF, name) #Check if var exists in GF
                GF.var_list.append(var(name)) #Append to GF var list

            elif(fr == "LF"):
                if(not(LF := frame_stack.top())): #Check if frame stack is not empty
                    ER.error_out(ER.frame_existence_error, "DEFVAR: No Local Frame exists!\n")

                Check_Var(LF, name) #Check if var exists in LF
                LF.var_list.append(var(name)) #Add it to LF var list

            elif(fr == "TF"):
                if(TF is None):
                    ER.error_out(ER.frame_existence_error, "DEFVAR: No Temporary Frame exists!\n")

                Check_Var(TF, name) #Check if var exists in TF
                TF.var_list.append(var(name)) #Add it to TF var list


        elif(instr.opcode == "MOVE"):
            Check_Args(instr, 2) #Check number of args
            var_dest = Check_Var(instr.get_var_frame(1), instr.get_var_name(1), frame_stack, GF, TF) #Get destination var
            source_l = Get_Vars(instr, 2, frame_stack, GF, TF) #Get arguments

            var_dest.update_content(source_l[0].value, source_l[0].type) #update vars contents


        elif(instr.opcode == "CALL"):
            Check_Args(instr,1) #Check number of args
            call_stack.stack.append(instr_counter+1) #Push incremented instruction counter to call stack
            name = instr.get_arg_content(1)
            found = False
            
            for lab in label_list:
                if(lab.name == name):
                    counter = lab.instr_index
                    found = True

            if(not found): ER.error_out(ER.semantic_error, "CALL: Label '" + name +"' doesnt exist!\n") #If label with speciefied name doesnt exist call for an error
            instr_counter = counter #If it exists change instruction index to new value
            instr_num += 1 #Increment number of processed instructions
            continue


        elif(instr.opcode == "RETURN"):
            Check_Args(instr,0) #Check number of args
            if(not(temp := call_stack.top())): #If call stack empty error
                ER.error_out(ER.missing_value, "RETURN: Call Stack Empty!\n")
            
            call_stack.pop() #Else pop call stack and change instruction index
            instr_counter = temp
            instr_num += 1
            continue


        elif(instr.opcode == "PUSHS"):
            Check_Args(instr,1) #Check number of args
            source_l = Get_Vars(instr, 1, frame_stack, GF, TF) #Get arguments
            data_stack.stack.append(symbol(source_l[0].type, source_l[0].value)) #Push value to data stack


        elif(instr.opcode == "POPS"):
            Check_Args(instr,1) #Check number of args
            var_dest = Check_Var(instr.get_var_frame(1), instr.get_var_name(1), frame_stack, GF, TF) #Get destination var

            source = data_stack.ptop(instr.opcode, ER.missing_value) #Get source from data stack and pop
            var_dest.update_content(source.value, source.type) #Update destination var


        elif(instr.opcode in ("ADD", "SUB", "MUL", "IDIV")):
            Check_Args(instr,3) #Check number of args
            var_dest = Check_Var(instr.get_var_frame(1), instr.get_var_name(1), frame_stack, GF, TF) #Get destination var
            source_l = Get_Vars(instr, 2, frame_stack, GF, TF) #Get arguments

            Check_Types(source_l[0].type, source_l[1].type, ["int", "float"]) #Check if the type is either int or float

            try: #Try changing args to int, if cant error
                if(source_l[0].type == "int"): #If its int change the values to int
                    source1 = int(source_l[0].value)
                    source2 = int(source_l[1].value)
                else:
                    if(instr.opcode == "IDIV"):
                        ER.error_out(ER.xml_structure_error, "Error: Using IDIV with floats!\n")
                    source1 = float.fromhex(source_l[0].value) #Else change values to float (decadic form)
                    source2 = float.fromhex(source_l[1].value)
            except ValueError: #If fails call for an error
                ER.error_out(ER.xml_structure_error, "Error: Invalid XMl structure!\n")

            if(instr.opcode == "ADD"): result = source1 + source2 
            elif(instr.opcode == "SUB"): result = source1 - source2
            elif(instr.opcode == "MUL"): result = source1 * source2
            else:
                if(source2 == 0): ER.error_out(ER.operand_wrong_value, "IDIV: Division by 0!\n")
                result = source1 / source2

            if(source_l[0].type == "int"): var_dest.update_content(result, "int") #update vars contents
            else: var_dest.update_content(float.hex(result), "float")


        elif(instr.opcode == "DIV"):
            Check_Args(instr,3) #Check number of args
            var_dest = Check_Var(instr.get_var_frame(1), instr.get_var_name(1), frame_stack, GF, TF) #Get destination var
            source_l = Get_Vars(instr, 2, frame_stack, GF, TF) #Get arguments

            Check_Types(source_l[0].type, source_l[1].type, "float")

            source1 = float.fromhex(source_l[0].value)
            source2 = float.fromhex(source_l[1].value)
            if(source2 == 0):
                ER.error_out(ER.operand_wrong_value, "DIV: Division by 0!\n")

            result = source1 / source2
            var_dest.update_content(float.hex(result), "float")

            
        elif(instr.opcode in ("LT", "GT", "EQ", "LTS", "GTS", "EQS")):
            if(instr.opcode in ("LT", "GT", "EQ")): #If its not a stack instruction get destination var and arguments
                Check_Args(instr,3) #Check number of args
                var_dest = Check_Var(instr.get_var_frame(1), instr.get_var_name(1), frame_stack, GF, TF)
                source_l = Get_Vars(instr, 2, frame_stack, GF, TF) #Get arguments
                source1 = source_l[0]
                source2 = source_l[1]
            else: #Else get values from data stack and pop
                Check_Args(instr, 0) #Check number of args
                source2 = data_stack.ptop(instr.opcode, ER.missing_value)
                source1 = data_stack.ptop(instr.opcode, ER.missing_value)


            if(instr.opcode not in ("EQ", "EQS") and (source1.type == "nil" or source2.type == "nil")): #Check if nil is used only with EQ or EQS
                ER.error_out(ER.operand_type_error, "Nil can be used with EQ only!\n")

            if(source2.type == "nil"): #If second source arguments is nil, swap them (easier checking)
                source1, source2 = source2, source1

            elif(source1.type != "nil"):
                if(source1.type not in("int", "bool", "string", "float") or source1.type != source2.type): #Check if the source values are of correct types
                    ER.error_out(ER.operand_type_error, instr.opcode+": Types not matching!\n")

            if(instr.opcode in ("LT", "LTS")): 
                if(source1.type == "int"): result = "true" if int(source1.value) < int(source2.value) else "false" #If int change values to int and proceed
                elif(source1.type == "float"): result = "true" if float.fromhex(source1.value) < float.fromhex(source2.value) else "false" #Change to float if float
                else: result = "true" if source1.value < source2.value else "false" #String

            elif(instr.opcode in ("GT", "GTS")): 
                if(source1.type == "int"): result = "true" if int(source1.value) > int(source2.value) else "false"
                elif(source1.type == "float"): result = "true" if float.fromhex(source1.value) > float.fromhex(source2.value) else "false"
                else: result = "true" if source1.value > source2.value else "false"

            elif(instr.opcode in ("EQ", "EQS")): 
                if(source1.type == "int"): result = "true" if int(source1.value) == int(source2.value) else "false"
                elif(source1.type == "float"): result = "true" if float.fromhex(source1.value) == float.fromhex(source2.value) else "false"
                elif(source1.type == "nil"): result = "true" if source2.type == "nil" else "false"
                else: result = "true" if source1.value == source2.value else "false"

            if(instr.opcode in ("LT", "GT", "EQ")):
                var_dest.update_content(result, "bool") #update vars contents
            else:
                data_stack.stack.append(symbol("bool", result)) #Push to data stack


        elif(instr.opcode in ("AND", "OR", "NOT")):
            if(instr.opcode == "NOT"): Check_Args(instr,2) #Check number of args for NOT
            else: Check_Args(instr,3) #Check number of args for else

            var_dest = Check_Var(instr.get_var_frame(1), instr.get_var_name(1), frame_stack, GF, TF) #Get destination var
            source_l = Get_Vars(instr, 2, frame_stack, GF, TF) #Get arguments


            if(instr.opcode != "NOT"):
                Check_Types(source_l[0].type, source_l[1].type, "bool") #Check if the source types are bool
                if(instr.opcode == "AND"): result = "true" if source_l[0].value == "true" and source_l[1].value == "true" else "false"
                else: result = "true" if source_l[0].value == "true" or source_l[1].value == "true" else "false"
                    
            else: #Not---
                Check_Types("bool", source_l[0].type, "bool")
                result = "true" if source_l[0].value == "false" else "false"

            var_dest.update_content(result, "bool") #update vars contents

        
        elif(instr.opcode == "INT2CHAR"):
            Check_Args(instr,2) #Check number of args
            var_dest = Check_Var(instr.get_var_frame(1), instr.get_var_name(1), frame_stack, GF, TF) #Get destination var
            source_l = Get_Vars(instr, 2, frame_stack, GF, TF) #Get arguments
 
            if(source_l[0].type != "int"):
                ER.error_out(ER.operand_type_error, "INT2CHAR: Types not matching!\n")

            if(int(source_l[0].value) < 0 or int(source_l[0].value) > 1114111): #Check if the unicode value doesnt exceed max and min values
                ER.error_out(ER.string_error, "INT2CHAR: Unicode value out of bounds!\n")
            
            var_dest.update_content(chr(int(source_l[0].value)), "string") #update vars contents


        elif(instr.opcode == "STRI2INT"):
            Check_Args(instr,3) #Check number of args
            var_dest = Check_Var(instr.get_var_frame(1), instr.get_var_name(1), frame_stack, GF, TF) #Get destination var
            source_l = Get_Vars(instr, 2, frame_stack, GF, TF) #Get arguments

            if(source_l[0].type != "string" or source_l[1].type != "int"):
                ER.error_out(ER.operand_type_error, "STRI2INT: Types not matching!\n")

            index = int(source_l[1].value)
            str_len = len(source_l[0].value)-1

            if(index < 0 or index > str_len): #Check if the specified index doesnt exceed string lenght
                ER.error_out(ER.string_error, "STRI2INT: index out of range!\n")

            var_dest.update_content(ord(source_l[0].value[index:index+1:]), "int") #update vars contents

        
        elif(instr.opcode == "READ"):
            Check_Args(instr,2) #Check number of args
            var_dest = Check_Var(instr.get_var_frame(1), instr.get_var_name(1), frame_stack, GF, TF) #Get destination var
            result = None

            type = instr.get_arg_content(2)
            if(Flags.input_flag == True): #If input file was passed get input from it
                if(line_num <= list_len):
                    result = Files.input_string[line_num]
                    line_num += 1
                else: #If all lines in input file were read update var value to nil
                    var_dest.update_content("nil", "nil")
                    instr_num += 1 
                    instr_counter += 1
                    continue


            if(type == "int"):
                if(Flags.input_flag != True): result = input() #If input file wasnt passed get input from stdin
                try:
                    result = int(result) #Try changing value to int
                    var_dest.update_content(result, "int")
                except ValueError or not result: #If fails update with nil
                    var_dest.update_content("nil", "nil")


            elif(type == "string"):
                if(Flags.input_flag != True): result = input()
                try:
                    result = str(result)
                    var_dest.update_content(result, "string")
                except ValueError or not result:
                    var_dest.update_content("nil", "nil")


            elif(type == "bool"):
                if(Flags.input_flag != True): result = input()
                try: 
                    string = str(result.lower())
                    result = "true" if string == "true" else "false"
                    var_dest.update_content(result, "bool")
                except ValueError or not result:
                    var_dest.update_content("nil", "nil")

            
            elif(type == "float"):
                if(Flags.input_flag != True): result = input()
                try:
                    result = Get_Float(result)#Get hexadecimal form of float
                    var_dest.update_content(result, "float")
                except ValueError or not result:
                    var_dest.update_content("nil", "nil")

        
        elif(instr.opcode == "WRITE"):
            Check_Args(instr,1) #Check number of args
            source_l = Get_Vars(instr, 1, frame_stack, GF, TF) #Get arguments
            if(source_l[0].type == "nil"): print("",end="") #If the var is nil, print empty string
            else: print(str(source_l[0].value), end="") #Else print its value

        
        elif(instr.opcode == "CONCAT"):
            Check_Args(instr,3) #Check number of args
            var_dest = Check_Var(instr.get_var_frame(1), instr.get_var_name(1), frame_stack, GF, TF)
            source_l = Get_Vars(instr, 2, frame_stack, GF, TF) #Get arguments


            Check_Types(source_l[0].type, source_l[1].type, "string")

            var_dest.update_content(str(source_l[0].value + source_l[1].value), "string") #Update var content with the concatenated strings


        elif(instr.opcode == "STRLEN"):
            Check_Args(instr,2) #Check number of args
            var_dest = Check_Var(instr.get_var_frame(1), instr.get_var_name(1), frame_stack, GF, TF)
            source_l = Get_Vars(instr, 2, frame_stack, GF, TF) 

            if(source_l[0].type != "string"):
                ER.error_out(ER.operand_type_error, "STRLEN: Types not matching!\n")

            var_dest.update_content(len(str(source_l[0].value)), "int") #Update the var value with lenght of string and change type to int

        
        elif(instr.opcode == "GETCHAR"):
            Check_Args(instr,3) #Check number of args
            var_dest = Check_Var(instr.get_var_frame(1), instr.get_var_name(1), frame_stack, GF, TF)
            source_l = Get_Vars(instr, 2, frame_stack, GF, TF)
            

            if(source_l[0].type != "string" or source_l[1].type != "int"):
                ER.error_out(ER.operand_type_error, "GETCHAR: Types not matching!\n")


            index = int(source_l[1].value)
            str_len = len(source_l[0].value)-1
            if(index < 0 or index > str_len): #Change if index doesnt exceed string lenght
                ER.error_out(ER.string_error, "GETCHAR: index out of range!\n")

            var_dest.update_content(source_l[0].value[index:index+1:], "string") #update vars contents

        
        elif(instr.opcode == "SETCHAR"):
            Check_Args(instr,3) #Check number of args
            var_dest = Check_Var(instr.get_var_frame(1), instr.get_var_name(1), frame_stack, GF, TF)
            source_l = Get_Vars(instr, 2, frame_stack, GF, TF) #Get arguments

            if(var_dest.type is None): ER.error_out(ER.missing_value, "SETCHAR: Missing value!\n")

            if(source_l[0].type != "int" or source_l[1].type != "string" or var_dest.type != "string"):
                ER.error_out(ER.operand_type_error, "SETCHAR: Types not matching!\n")

            index = int(source_l[0].value)
            str_len = len(var_dest.value)-1

            if((index < 0 or index > str_len) or len(source_l[1].value) == 0):
                ER.error_out(ER.string_error, "SETCHAR: index out of range!\n")

            char = source_l[1].value[0:1:] #Get the first char from source string

            var_dest.update_content(var_dest.value[0:index:] + char + var_dest.value[index+1:], "string") #update vars contents with char in its place
            

        elif(instr.opcode == "TYPE"):
            Check_Args(instr,2) #Check number of args
            var_dest = Check_Var(instr.get_var_frame(1), instr.get_var_name(1), frame_stack, GF, TF)
            source_l = Get_Vars(instr, 2, frame_stack, GF, TF) #Get arguments

            if(source_l[0].type is None): var_dest.update_content("", "string") #if arg doesnt have a type, update var value to empty
            else: var_dest.update_content(source_l[0].type, "string") #else give its type


        elif(instr.opcode == "JUMP"):
            Check_Args(instr,1) #Check number of args
            name = instr.get_arg_content(1)
            found = False

            for lab in label_list: #Check existence of label
                if(lab.name == name):
                    instr_counter = lab.instr_index
                    found = True

            if(found == True):
                instr_num += 1
                continue
            else: ER.error_out(ER.semantic_error, "JUMP: Label '" + name +"' doesnt exist!\n") #Label wasnt found


        elif(instr.opcode in ("JUMPIFEQ", "JUMPIFNEQ")):
            Check_Args(instr,3) #Check number of args
            name = instr.get_arg_content(1)
            source_l = Get_Vars(instr, 2, frame_stack, GF, TF) #Get arguments
            found = False


            if(source_l[0].type != source_l[1].type and (source_l[0].type != "nil" and source_l[1].type != "nil")): #If the source types dont match and none of them is nil, call for an error
                ER.error_out(ER.operand_type_error, instr.opcode+": Types not matching!\n")

            for lab in label_list: #Check existence of label
                if(lab.name == name):
                    counter = lab.instr_index
                    found = True

            if(found == False): ER.error_out(ER.semantic_error, instr.opcode+": Label '" + name +"' doesnt exist!\n")

            if(instr.opcode == "JUMPIFEQ"):
                if(source_l[0].value == source_l[1].value): #If the values are equal (JUMIFEQ) change instruction index
                    instr_counter = counter
                    instr_num += 1
                    continue
            else:
                if(source_l[0].value != source_l[1].value): #If the values are not equal (JUMIFNEQ) change instruction index
                    instr_counter = counter
                    instr_num += 1
                    continue

        elif(instr.opcode == "EXIT"):
            Check_Args(instr,1) #Check number of args
            source_l = Get_Vars(instr, 1, frame_stack, GF, TF) #Get arguments

            if(source_l[0].type != "int"): ER.error_out(ER.operand_type_error, "EXIT: Type not matching!\n")

            value = int(source_l[0].value)
            if(value < 0 or value > 49): #Check if the int is within limits
                ER.error_out(ER.operand_wrong_value, "EXIT: Wrong value!\n")

            sys.exit(value) #Exit with the value


        elif(instr.opcode == "DPRINT"):
            Check_Args(instr,1) #Check number of args
            source_l = Get_Vars(instr, 1, frame_stack, GF, TF) #Get arguments
            if(source_l[0].type == "nil"):
                print("", file=sys.stderr) 
            else:
                print(source_l[0].value, file=sys.stderr) 


        elif(instr.opcode == "BREAK"):
            Check_Args(instr, 0) #Check number of args
            msg=dedent("""
            Position in code: {0}
            Number of processed instructions: {1}

            Frame State (GF):
            {2}
            Frame State (LF):
            {3}

            Frame State (TF):
            {4}
            """)

            print(msg.format(str(instr_counter+1), str(instr_num), str(GF), str(frame_stack.top()), str(TF)), file=sys.stderr) #Print interpret status
        

        elif(instr.opcode == "INT2FLOAT"):
            Check_Args(instr, 2) #Check number of args
            var_dest = Check_Var(instr.get_var_frame(1), instr.get_var_name(1), frame_stack, GF, TF)
            source_l = Get_Vars(instr, 2, frame_stack, GF, TF)
            Check_Types("int", source_l[0].type, "int") #Check if the source type is int

            var_dest.update_content(Get_Float(source_l[0].value), "float") #Update var with hexadecimal representation

        
        elif(instr.opcode == "FLOAT2INT"):
            Check_Args(instr, 2) #Check number of args
            var_dest = Check_Var(instr.get_var_frame(1), instr.get_var_name(1), frame_stack, GF, TF)
            source_l = Get_Vars(instr, 2, frame_stack, GF, TF) #Get arguments
            Check_Types("float", source_l[0].type, "float")

            var_dest.update_content(int(float.fromhex(source_l[0].value)), "int") #Change float value to int and update var content


        elif(instr.opcode == "CLEARS"): data_stack.stack = [] #Clear data stack


        elif(instr.opcode in ("ADDS", "SUBS", "MULS", "IDIVS")): #Same as non stack instructions, but source values are taken from stack and popped
            Check_Args(instr, 0) #Check number of args
            source2_symb = data_stack.ptop(instr.opcode, ER.missing_value)
            source1_symb = data_stack.ptop(instr.opcode, ER.missing_value)

            Check_Types(source1_symb.type, source2_symb.type, "int")

            try: #Try changing args to int, if cant error
                source1 = int(source1_symb.value)
                source2 = int(source2_symb.value)
            except ValueError:
                ER.error_out(ER.xml_structure_error, "Error: Invalid XMl structure!\n")

            if(instr.opcode == "ADDS"): result = source1 + source2
            elif(instr.opcode == "SUBS"): result = source1 - source2
            elif(instr.opcode == "MULS"): result = source1 * source2
            else:
                if(source2 == 0): ER.error_out(ER.operand_wrong_value, "IDIVS: Division by 0!\n")
                result = source1 / source2

            data_stack.stack.append(symbol("int", result)) #update vars contents


        elif(instr.opcode in ("ANDS", "ORS", "NOTS")): #Same as non stack instructions, but source values are taken from stack and popped
            Check_Args(instr, 0) #Check number of args
            if(instr.opcode == "NOTS"):
                source1 = data_stack.ptop(instr.opcode, ER.missing_value)
            else:
                source1 = data_stack.ptop(instr.opcode, ER.missing_value)
                source2 = data_stack.ptop(instr.opcode, ER.missing_value)

            if(instr.opcode != "NOTS"):
                Check_Types(source1.type, source2.type, "bool")
                if(instr.opcode == "ANDS"): result = "true" if source_l[0].value == "true" and source2.value == "true" else "false"
                else: result = "true" if source1.value == "true" or source2 .value == "true" else "false"
                    
            else:
                Check_Types("bool", source1.type, "bool")
                result = "true" if source1.value == "false" else "false"

            data_stack.stack.append(symbol("bool", result))


        elif(instr.opcode == "INT2CHARS"): #Same as non stack instruction, but source value is taken from stack and popped
            Check_Args(instr, 0) #Check number of args
            source = data_stack.ptop(instr.opcode, ER.missing_value)
            Check_Types("int", source.type, "int")

            if(int(source.value) < 0 or int(source.value) > 1114111):
                ER.error_out(ER.string_error, "INT2CHARS: Unicode value out of bounds!\n")

            data_stack.stack.append((symbol("string", chr(int(source.value))))) #update vars contents


        elif(instr.opcode == "STRI2INTS"): #Same as non stack instruction, but source values are taken from stack and popped
            Check_Args(instr, 0) #Check number of args
            source2 = data_stack.ptop(instr.opcode, ER.missing_value)
            source1 = data_stack.ptop(instr.opcode, ER.missing_value)

            if(source1.type != "string" or source2.type != "int"):
                ER.error_out(ER.operand_type_error, "STR2INTS: Types not matching!\n")

            index = int(source2.value)
            str_len = len(source1.value)-1

            if(index < 0 or index > str_len):
                ER.error_out(ER.string_error, "STR2INTS: index out of range!\n")

            data_stack.stack.append(symbol("int", ord(source1.value[index:index+1:])))
            
        
        elif(instr.opcode in ("JUMPIFEQS", "JUMPIFNEQS")):#Same as non stack instructions, but source values are taken from stack and popped
            Check_Args(instr,1) #Check number of args
            source2 = data_stack.ptop(instr.opcode, ER.missing_value)
            source1 = data_stack.ptop(instr.opcode, ER.missing_value)
            label_name = instr.get_arg_content(1)
            found = False

            if(source1.type != source2.type and (source1.type != "nil" and source2.type != "nil")):
                ER.error_out(ER.operand_type_error, instr.opcode+": Types not matching!\n")

            for lab in label_list:
                if(lab.name == label_name):
                    counter = lab.instr_index
                    found = True

            if(found == False): ER.error_out(ER.semantic_error, instr.opcode+": Label '" + name +"' doesnt exist!\n")

            if(instr.opcode == "JUMPIFEQS"):
                if(source1.value == source2.value):
                    instr_counter = counter
                    instr_num += 1
                    continue
            else:
                if(source1.value != source2.value):
                    instr_counter = counter
                    instr_num += 1
                    continue


        else: #Instruction wasnt recognized
            ER.error_out(ER.xml_structure_error, instr.opcode+": Instruction not recognized!\n")
            

        instr_num += 1 
        instr_counter += 1
#------------------------------------------------------------------------


#Main Function-----------------------------------------------------------
def Interpret():
    root = tree.getroot()

    if(root.tag != "program" or root.get("language") != "IPPcode22"): #Check XML structure
        ER.error_out(ER.xml_structure_error, "Error: Invalid XMl structure!\n")

    for atribute in root.attrib:
        if(atribute not in ("language", "description", "name")): #Check XML structure
            ER.error_out(ER.xml_structure_error, "Error: Invalid XMl structure!\n")
    
    Instruction_list = Get_Instructions(root) #Get Instructions from XML tree
    Process_Instructions(Instruction_list) #Process instructions
#------------------------------------------------------------------------


Interpret()
exit(0)
