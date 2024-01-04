# Code Overview:
# The provided Python script is a versatile tool for visualizing code regions based on trace and log files. 
# It parses trace and log data to generate a tabulated summary, highlighting the weights associated with 
# specific events within a specified memory region. The user can define the start and end program counters
# (PCs) in hexadecimal format to focus on a particular code segment.
# 
# Functionality:
# 
# 1. File Parsing:
#   a. Parses the provided trace file, which contains PC (program counter) and code information.
#   b. Parses the log file, which includes event-related data such as PC, event name, and weight.
# 
# 2. Data Processing:
#   a. Filters relevant trace events based on the specified PC range.
#   b. Calculates the sum of weights associated with unique event names occurring within the defined PC range.
# 
# 3. Visualization:
#   a. Creates a table with columns representing PC:Code and the sum of weights for each unique event name.
#   b. Outputs the tabulated data for easy visualization of the code region.
# 
# 4. Command-Line Interface:
#   a. Accepts command-line arguments for the trace file, log file, start PC, and end PC.
#   b. Users can execute the script with the specified parameters to analyze different code segments.
# 
# Usage:
# python visualize_code_region.py trace_file.log log_file.txt start_pc end_pc
# Example:
# python visualize_code_region.py 600.trace spec2017-600.perlbench_s_0.txt 0xb88e0 0xb8910
#
# trace_file.log format:
# pc:code
# e.g.:
# 0xb88d8:bltu s10, s2, 16
# 0xb88e0:c_addi s10, 1
# 0xb88e2:c_sdsp s10, 88(sp)
# log_file.txt is generate by DG_Miner_prt
#
# Output:
# +------------+--------------+--------------+--------------+
# | PC:Code    | Event1       | Event2       | Event3       |
# +------------+--------------+--------------+--------------+
# | 0xb88e0:...| 10           | *            | 5            |
# | 0xb88e2:...| *            | 20           | *            |
# | ...        | ...          | ...          | ...          |
# +------------+--------------+--------------+--------------+
# 
# Conclusion:
# This script provides a clear overview of the weights associated with different events in the specified 
# code region, aiding in performance analysis and optimization efforts.

from tabulate import tabulate
import matplotlib.pyplot as plt
from collections import defaultdict
import argparse

def parse_trace_file(trace_file_path):
    trace_events = []
    with open(trace_file_path, 'r') as file:
        for line in file:
            parts = line.strip().split(':')
            if len(parts) == 2 and parts[0].strip().isalnum():
                trace_events.append({
                    'pc': int(parts[0], 16),  # Convert PC to integer
                    'code': parts[1].strip(),
                })
    return trace_events

def parse_log_file(log_file_path):
    events = []
    with open(log_file_path, 'r') as file:
        for line in file:
            parts = line.split()
            if len(parts) >= 5 and parts[0].isdigit():
                events.append({
                    'pc': parts[3],
                    'event_name': parts[1],
                    'weight': int(parts[4]),
                })

    return events

def update_2d_dict(thedict, key_a, key_b, add_val):
    if key_a in thedict:
        thedict[key_a].update({key_b: thedict[key_a].get(key_b, 0) + add_val})
    else:
        thedict.update({key_a: {key_b : add_val}})

def visualize_code_region(trace_file, log_file, start_pc, end_pc):
    trace_events = parse_trace_file(trace_file)
    log_events = parse_log_file(log_file)

    relevant_trace_events = [event for event in trace_events if start_pc <= event['pc'] <= end_pc]

    # Calculate the sum of weights for each unique event
    event_sum_weights = {}
    for log_event in log_events:
        log_event_name = log_event['event_name']
        log_event_weight = log_event['weight']
        log_event_pc = log_event['pc']
        if start_pc <= int(log_event_pc, 16) <= end_pc:
            event_key = log_event_name
            update_2d_dict(event_sum_weights, event_key, log_event_pc, log_event_weight)

    # Filter out columns where sum of weights is not 0
    non_empty_columns = [col for col in event_sum_weights.keys() if event_sum_weights[col][list(event_sum_weights[col].keys())[0]] != 0]
    #non_empty_columns = [col for col in event_sum_weights.keys() if event_sum_weights[col][list(event_sum_weights[col].keys())[0]] > 10000]
    columns_to_display = ['PC:Code'] + non_empty_columns

    # Prepare table data
    table_data = [columns_to_display]

    for trace_event in relevant_trace_events:
        row = [f"{hex(trace_event['pc'])}:{trace_event['code']}"]

        for event_name in non_empty_columns:
            got = False
            for event_pc in event_sum_weights[event_name].keys():
                if event_pc is not None and int(event_pc, 16) == trace_event['pc']:
                    row.append(event_sum_weights[event_name][event_pc])
                    got = True
                    break
            if not got:
                row.append('*')

        table_data.append(row)

    print(tabulate(table_data, headers="firstrow", tablefmt="grid"))


if __name__ == "__main__":
    parser = argparse.ArgumentParser(description='Visualize code region based on trace and log files.')
    parser.add_argument('trace_file', help='Path to the trace file')
    parser.add_argument('log_file', help='Path to the log file')
    parser.add_argument('start_pc', type=lambda x: int(x, 0), help='Start PC in hexadecimal format')
    parser.add_argument('end_pc', type=lambda x: int(x, 0), help='End PC in hexadecimal format')

    args = parser.parse_args()

    visualize_code_region(args.trace_file, args.log_file, args.start_pc, args.end_pc)
