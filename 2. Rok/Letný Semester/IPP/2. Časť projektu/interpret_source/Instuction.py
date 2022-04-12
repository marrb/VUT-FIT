from interpret_source.error import Error as ER

#Argument class------------------------------
class Argument:
    def __init__(self, type, order, content):
        self.type = type.lower()
        self.order = order
        self.content = content

    #Structured printing
    def __str__(self):
        return "| Order: "+str(self.order)+", Type: "+self.type+", Content: "+self.content+" |"
#-------------------------------------------

#Instruction class--------------------------
class Instruction:
    def __init__(self, opcode, order, arguments):
        self.opcode = opcode.upper()
        self.order = int(order)
        self.arguments = arguments

    #Structured printing
    def __str__(self):
        arguments = ""
        for arg in self.arguments:
            if(arg):
                arguments += str(arg) + "\n"

        return "Opcode: "+self.opcode + ", Order: " + str(self.order) +  ", Arguments:\n" + arguments

    #Gets name of a variable
    def get_var_name(self, order):
        order -= 1
        if(self.arguments[order].type != "var"): 
            ER.error_out(ER.xml_structure_error, "Error: Invalid XMl structure!\n")

        return self.arguments[order].content[3::]

    #Gets frame of a variable
    def get_var_frame(self, order):
        order -= 1
        if(self.arguments[order].type != "var"): 
            ER.error_out(ER.xml_structure_error, "Error: Invalid XMl structure!\n")
            
        return self.arguments[order].content[:2:]

    #Gets type of an instruction argument
    def get_arg_type(self, order):
        order -= 1
        return self.arguments[order].type

    #Gets value of an instruction argument
    def get_arg_content(self, order):
        order -= 1
        return self.arguments[order].content
#-------------------------------------------