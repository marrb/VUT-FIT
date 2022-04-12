from interpret_source.error import Error as ER

#Stack class-----------------------------
class stack:
    stack = []

    def __init__(self):
        self.stack = []

    #Get the top of the stack
    def top(self):
        if(self.stack):
            return self.stack[-1]

    #Pop the top value
    def pop(self):
        self.stack.pop()


    #Return top and then pop
    def ptop(self, instr_opcode, error):
        if(not self.stack):
            ER.error_out(error, instr_opcode+": Stack empty!\n")
            
        result = self.top()
        self.pop()
        return result
#----------------------------------------