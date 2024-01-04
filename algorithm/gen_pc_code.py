import re
import argparse

def process_output(log_file_path, output_file_path):
    with open(log_file_path, 'r') as log_file:
        with open(output_file_path, 'w') as output_file:
            in_trace_section = False

            for line in log_file:
                if "-------------------- top pc region -----------------------------" in line:
                    in_trace_section = True
                elif "----------------------------------------------------------------" in line:
                    in_trace_section = False
                elif in_trace_section:
                    # Extract relevant information from the trace section
                    line = line.strip()
                    match = re.match(r"(0x[0-9a-fA-F]+):(.+)", line)
                    if match:
                        pc = match.group(1)
                        code = match.group(2)
                        output_file.write(f"{pc}:{code}\n")

if __name__ == "__main__":
    parser = argparse.ArgumentParser(description='Generate the top pc region from log file.')
    parser.add_argument('log_file', help='Path to the log file')
    parser.add_argument('output_file', help='Path to the output file')

    args = parser.parse_args()

    process_output(args.log_file, args.output_file)

