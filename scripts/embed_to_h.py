
import os
import sys
import binascii
from os.path import basename

# Binary

def read_hex_from_file(filename):
    with open(filename, "rb") as file:
        return binascii.hexlify(bytearray(file.read()))

def write_header(array_name, array_length):
    return '\n' \
        '#pragma once\n\n' \
        'constexpr int ' + array_name + '_data_length = ' + str(array_length) + ';\n\n' \
        'constexpr unsigned char ' + array_name + '_data[] = {\n'

def write_hex_data(hex_data):
    def chunker(seq, size):
        return (seq[pos:pos + size] for pos in xrange(0, len(seq), size))
    
    hex_string = '\t'
    for hex in chunker(hex_data, 2):
        hex_string += '0x' + hex.upper() + ', '
    return hex_string[0:-2]

def write_footer():
    return '\n};\n'


# Ascii

def read_ascii_data_from_file(filename):
    with open(filename, "r") as file:
        return file.read()

def write_ascii_data(variable_name, data):
    output_string = '\n' \
        '#pragma once\n\n' \
        'constexpr const char* ' + variable_name + '_data = R"(\n'
    
    output_string += data
    output_string += ')";\n'

    return output_string

def convert_file(data_type, input_file):
    if data_type != '-binary' and data_type != '-ascii':
        print 'No data type specified, or unknown: ' + data_type
        return

    output_file = os.path.splitext(input_file)[0] + '.h'
    base_filename = os.path.splitext(basename(input_file))[0]

    output_string = ''

    if data_type is '-binary':
        hex_bytes = read_hex_from_file(input_file)
        output_string = write_header(base_filename, len(hex_bytes) / 2)
        output_string += write_hex_data(hex_bytes)
        output_string += write_footer()
    else:
        ascii_data = read_ascii_data_from_file(input_file)
        output_string = write_ascii_data(base_filename, ascii_data)

    with open(output_file, "w") as output:
        output.write(output_string)

if __name__  == "__main__":
    if len(sys.argv) > 2:
        convert_file(sys.argv[1], sys.argv[2])
    else:
        print 'Invalid amount of arguments'
