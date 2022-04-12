from interpret_source.error import Error as ER
import re

def my_replace(match): #Function for replacing unicode (\035) codes with symbols
        return chr(int(match.group()[1::]))

#Var class----------------------------------
class var:
    name = ""
    value = None
    type = None

    def __init__(self, name, type = None, value = None):
        self.name = name
        self.value = value
        self.type = type

    #Updates value and type of a variable
    def update_content(self, value, type = None):
        if(type == "int"):
            try: int(value)
            except ValueError: ER.error_out(ER.xml_structure_error, "Error: Invalid XMl structure!\n")
            self.value = int(value)
        elif(not value):
            self.value = ""
        else:
            self.value = str(re.sub(r"\\\d{3}", my_replace, value))
        if(type is not None):
            self.type = type

    #Structured printing
    def __str__(self):
        value = "None" if self.value is None else self.value
        type = "None" if self.type is None else self.type
        return str("Name: " + self.name + ", Value: " + str(value) + ", Type: " + type + "\n")
#-------------------------------------------


#Symbol class-------------------------------
class symbol:
    value = None
    type = None

    def __init__(self, type = None, value = None):
        if(type == "int"):
            try: int(value)
            except ValueError: ER.error_out(ER.xml_structure_error, "Error: Invalid XMl structure!\n")
            self.value = int(value)
        elif(not value):
            self.value = ""
        else:
            self.value = str(re.sub(r"\\\d{3}", my_replace, value))
        self.type = type
#-------------------------------------------

#Label class--------------------------------
class label:
    name = None
    instr_index = None

    def __init__(self, name, instr_index):
        self.name = name
        self.instr_index = int(instr_index)
#-------------------------------------------


#Frame class--------------------------------
class frame:
    var_list = []

    def __init__(self, list = None):
        if(list is None):
            self.var_list = []
        else:
            self.var_list = list

    #Structured printing
    def __str__(self):
        result = ""
        if(not self.var_list): result = "Empty\n"
        
        for var in self.var_list:
            result += str(var)

        return result
#-------------------------------------------