import subprocess as sp
import filecmp as fc
import os
from time import sleep

test_files_path = './test_files'
output_files_path = './output'

input_files = [
    os.path.join(test_files_path, 'input1.txt'),
    os.path.join(test_files_path, 'input2.txt'),
    os.path.join(test_files_path, 'input3.txt')
]

expected_output_files = [
    os.path.join(test_files_path, 'output1.txt'),
    os.path.join(test_files_path, 'output2.txt'),
    os.path.join(test_files_path, 'output3.txt'),
]

output_files = [
    os.path.join(output_files_path, 'out1.txt'),
    os.path.join(output_files_path, 'out2.txt'),
    os.path.join(output_files_path, 'out3.txt'),
]

def run_test():
    for i, input_file in enumerate(input_files):
        inFile = open(input_file, 'r')
        outFile = open(output_files[i], 'w')
        print(f'running "./line_processor < {input_file} > {output_files[i]}"')
        sp.call(['./line_processor'], stdin=inFile, stdout=outFile)
    sleep(.25)

    for i, output_file in enumerate(output_files):
        if not fc.cmp(output_file, expected_output_files[i]):
            return False
    
    return True
        

success = run_test()

if success:
    print("PASS")
else:
    print("FAIL")

for i in range(len(output_files)):
    os.remove(output_files[i])