import os, sys
from subprocess import Popen, PIPE

class CustomError(Exception):
    def __init__(self, message):
        self.message = f"\033[91;1;4m{message}:\033[0m"
        super().__init__(self.message)

def delete_multiple_spaces(string: str) -> str:
    return ' '.join(string.split())

def line_diff_error_message(output_file: str, line_number: int, line: str, expected_line: str) -> str:
    return (f"\033[91mError at line {line_number} in file '{output_file}':\n"
            f"\toutput is \033[90m'{line}'\033[91m but expected: \033[90m'{expected_line}'\033[0m")

def line_diff(line: str, expected_line: str, context: dict) -> bool:
    # Normalize spaces in both lines.
    normalized_line = delete_multiple_spaces(line)
    normalized_expected = delete_multiple_spaces(expected_line)
    
    if normalized_line == normalized_expected:
        return True
    else:
        print(line_diff_error_message(context['output_file'], context['line_number'], line, expected_line), file=sys.stderr)
        return False

def check_output(output_file_name: str, expected_output_file_name: str) -> float:
    with open(output_file_name, 'r') as output_file:
        output = output_file.read().strip().split('\n')
    with open(expected_output_file_name, 'r') as expected_output_file:
        expected_output = expected_output_file.read().strip().split('\n')

    # Remove any empty lines.
    output = [line for line in output if line.strip()]
    expected_output = [line for line in expected_output if line.strip()]

    sum_matches = 0

    for i in range(min(len(output), len(expected_output))):
        if line_diff(output[i], expected_output[i], {'output_file': output_file_name, 'line_number': i}):
            sum_matches += 1

    return sum_matches / len(expected_output) if expected_output else 0.0

def run(executable: str, input_file_name: str, output_file_name: str, expected_output_file_name: str) -> float:
    with open(input_file_name, 'r') as input_file, open(output_file_name, 'w') as output_file:
        process = Popen([f'./{executable}'], stdout=PIPE, stdin=PIPE, universal_newlines=True)

        for line in input_file.readlines():
            prompt = process.stdout.read(3)
            if prompt != '>> ':
                continue

            process.stdin.write(line.rstrip('\n') + '\n')
            process.stdin.flush()

            if line.rstrip('\n') == 'exit':
                break

            # Write the response line to the output file.
            output_line = process.stdout.readline()
            output_file.write(output_line)

        input_file.close()
        process.stdin.close()
        process.stdout.close()

    return check_output(output_file_name, expected_output_file_name)

if __name__ == '__main__':
    if len(sys.argv) != 5:
        print('Usage: python3 checker.py <executable> <input_file> <output_file> <expected_output_file>')
        sys.exit(1)

    grade = run(sys.argv[1], sys.argv[2], sys.argv[3], sys.argv[4])
    print(grade)
