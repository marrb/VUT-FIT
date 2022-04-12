import sys

class Error:
    #Error Codes
    Parameter_error = 10
    input_open_file_error = 11
    output_open_file_error = 12
    internal_error = 99
    xml_format_error = 31
    xml_structure_error = 32
    semantic_error = 52
    operand_type_error = 53
    var_not_defined = 54
    frame_existence_error = 55
    missing_value = 56
    operand_wrong_value = 57
    string_error = 58

    #Function for printing out errors
    def error_out(error_type, msg):
        print(msg, file = sys.stderr)
        sys.exit(error_type) 