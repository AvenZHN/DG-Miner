
# DG-Miner

DG-Miner is a high average utility pattern miner designed for critical path analysis of processor dependence graphs.

## Table of Contents

- [Introduction](#introduction)
- [Building](#building)
- [Running Tests](#running-tests)
- [Usage](#usage)

## Introduction

DG-Miner is a tool designed for mining high average utility patterns to analyze critical paths in processor dependence graphs. It is particularly useful for understanding dependencies and performance characteristics in processor execution.

## Building

To build DG-Miner, follow these steps:

```bash
cd algorithm
make
```

Following DG\_Miner version will be generated:  
DG\_Miner: the mining frequent pattern program with concise pattern judgment mechanism, upper bound is adjustable, parameter setting for general analysis.  
DG\_Miner\_prt: compile with -DPRINT, DG\_Miner + program analysis + dumpout pattern trace file.  
DG\_Miner\_maxbound: compile with -DMAXBOUND, DG\_Miner but maximum utility as upper bound.  
DG\_Miner\_no\_cpjm: compile with -DNOCPJM, DG\_Miner without concise pattern judgment mechanism.  
DG\_Miner\_pecu: compile with -DPECU and -DPRINT, DG\_Miner but parameter setting for peculiarity analysis.  

## Running Tests

To run tests on DG-Miner using the provided test trace, use the following commands:

```bash
cd algorithm
make test
```

The print out information format is as follows:
```code
++++++++++++++++++++++++++++++++++++++++
trace:"<critical path trace file>"
minpau:<minimum database average utility threshold>
minu:<minimum occurrence average utility threshold>
upbound:<upper bound>
minpecusup:<minimum peculiar utility threshold>
[n]<candidate from frequent patterns>:<support>:<average utility>:<peculiarity>|<region0>:<occurrence>,...
[c]<candidate from high upper bound patterns>:<support>:<average utility>:<peculiarity>
================== result =====================
[n]<frequent pattern>

concise patts:
<concise pattern>
-------------------- top pc region -----------------------------
<when define PRINT>
<region number>:<occurrence>
[<freqatt number>]:<occurrence>,
	pc:code

---------------------------------------------------------------
<summary information>
minpau=... minpecusup=... minlen=... maxlen=...
minu=... upbound=...
The number of frequent patterns:...
The time-consuming:...ms. 
The number of calculation:...
The number of concise patts:...
Max len of frequent patts:...
The peak memory usage:...Mb.
The curr memory usage:...Mb.
```

## Usage

To run DG-Miner using a spec cpu2006/2017 trace, use the following command format:

```bash
cd algorithm
./DG_Miner simpoint_weights critical_paths pc_mem_trace output_file minlen maxlen topk
```

For example, to run DG-Miner on the 456.hmmer trace from spec cpu2006:

```bash
cd algorithm
./DG_Miner_prt ../database/test/test.simpoint_weight ../database/spec2006_archexp/critical_path/456.hmmer_0/ ../database/spec2006_archexp/pc_mem_trace/trace_1M-456.hmmer_0_simpoint_instruction_flow spec2006-456.hmmer_0.txt 2 10 0.2 | tee spec2006-456.hmmer_0.log
```

## Statistical analysis

Following additional statistical codes are used to generate the results in the paper "A Dependence Graph Pattern Mining Method for Processor Performance Analysis".

### Stat utility number of database (Figure 4)

The stat\_utility program aids in understanding the distribution of weights within the trace files, providing insights into the prevalence of different instruction types or events based on their assigned weights.

```bash
cd algorithm
g++ -std=c++17 stat_utility.cpp -lstdc++fs -o stat_utility
./stat_utility ../database/spec2006_archexp/critical_path/ ../database/spec2017_archexp/critical_path/
```

### Critical dependent chain (Section 5.3.1)

The sort\_event program assists in understanding the relative importance of different events in the log by analyzing their cumulative weights.
Usage example of spec2006 456.hmmer:

```bash
cd algorithm
g++ sort_event.cpp -o sort_event
./sort_event spec2006-456.hmmer_0.txt 1500777
```
The file spec2006-456.hmmer\_0.txt is generate by DG\_Miner\_prt.

### Critical program region (Section 5.3.2)

The visualize\_code\_region.py script provides a clear overview of the weights associated with different events in the specified code region, aiding in performance analysis and optimization efforts.

Usage example of spec2017 600.perlbench\_s:

```bash
cd algorithm
python gen_pc_code.py spec2017-600.perlbench_s_0.log 600.trace
python visualize_code_region.py 600.trace spec2017-600.perlbench_s_0.txt 0xb88e0 0xb8910
```

The 600.trace file can by generate by truncating the printout produced by DG\_Miner\_prt using gen\_pc\_code.py.
And the spec2017-600.perlbench\_s\_0.txt is the result file of DG\_Miner\_prt.

