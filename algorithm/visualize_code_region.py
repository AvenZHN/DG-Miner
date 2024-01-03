from tabulate import tabulate
import matplotlib.pyplot as plt
from collections import defaultdict

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
    ## Summary
    #event_summary = defaultdict(lambda: [0, ""])  # {event_name: [sum_of_weight, pc]}
    #for event in events:
    #    if start_pc <= int(event['pc'], 16) <= end_pc:
    #        event_summary[event['event_name']][0] += event['weight']
    #        event_summary[event['event_name']][1] = event['pc']

    ## Create a sorted list of tuples (event_name, (sum_of_weight, pc))
    #sorted_event_summary = sorted(event_summary.items(), key=lambda x: x[1][0], reverse=True)
    #print(sorted_event_summary)

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
    #event_pc_dict = {}
    for log_event in log_events:
        #log_event_name = log_event[0]
        #log_event_weight = log_event[1][0]
        #log_event_pc = log_event[1][1]
        log_event_name = log_event['event_name']
        log_event_weight = log_event['weight']
        log_event_pc = log_event['pc']
        #print(log_event_name, log_event_weight, log_event_pc)
        #print(start_pc, int(log_event_pc, 16), end_pc, start_pc <= int(log_event_pc, 16) <= end_pc)
        if start_pc <= int(log_event_pc, 16) <= end_pc:
            event_key = log_event_name
            #event_size = event_key.count('-') + 1
            #event_sum_weights[event_key][log_event_pc] = event_sum_weights.get(event_key, 0) + log_event_weight
            update_2d_dict(event_sum_weights, event_key, log_event_pc, log_event_weight)
            #event_pc_dict[event_key] = log_event_pc

    # Filter out columns where sum of weights is not 0
    #non_empty_columns = [col for col in event_sum_weights.keys() if event_sum_weights[col] != 0]
    non_empty_columns = [col for col in event_sum_weights.keys() if event_sum_weights[col][list(event_sum_weights[col].keys())[0]] != 0]
    #non_empty_columns = [col for col in event_sum_weights.keys() if event_sum_weights[col][list(event_sum_weights[col].keys())[0]] > 10000]
    columns_to_display = ['PC:Code'] + non_empty_columns

    # Prepare table data
    table_data = [columns_to_display]

    for trace_event in relevant_trace_events:
        row = [f"{hex(trace_event['pc'])}:{trace_event['code']}"]

        for event_name in non_empty_columns:
            #event_pc = next((event['pc'] for event in log_events if event['event_name'] == event_name), None)
            #event_pc = event_pc_dict[event_name]
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

# Example usage
trace_file_path = '600.trace'
log_file_path = 'spec2017-600.perlbench_s_0.txt'
start_pc = int('0xb88e0', 16)  # Convert start_pc to integer
end_pc = int('0xb8910', 16)    # Convert end_pc to integer


#trace_file_path = '429.trace'
#log_file_path = 'spec2006-429.mcf_0.txt'
#start_pc = int('0x116c0', 16)  # Convert start_pc to integer
#end_pc = int('0x116d2', 16)    # Convert end_pc to integer
#
#trace_file_path = '605.trace'
#log_file_path = 'spec2017-605.mcf_s_0.txt'
#start_pc = int('0x110e2', 16)  # Convert start_pc to integer
#end_pc = int('0x11106', 16)    # Convert end_pc to integer

visualize_code_region(trace_file_path, log_file_path, start_pc, end_pc)
