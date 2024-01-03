
# DG-Miner

DG-Miner is a high average utility pattern miner designed for critical path analysis of processor dependence graphs.

## Table of Contents

- [Introduction](#introduction)
- [Prerequisites](#prerequisites)
- [Building](#building)
- [Running Tests](#running-tests)
- [Usage](#usage)

## Introduction

DG-Miner is a tool designed for mining high average utility patterns to analyze critical paths in processor dependence graphs. It is particularly useful for understanding dependencies and performance characteristics in processor execution.

## Prerequisites

Before using DG-Miner, ensure you have the following dependencies installed:

- [C++ Compiler](#) - Replace with the appropriate link for installing a C++ compiler.

## Building

To build DG-Miner, follow these steps:

```bash
cd algorithm
make
```

## Running Tests

To run tests on DG-Miner using the provided test trace, use the following commands:

```bash
cd algorithm
make test
```

## Usage

To run DG-Miner using a spec cpu2006/2017 trace, use the following command format:

```bash
cd algorithm
./DG\_Miner simpoint\_weights critical\_paths pc\_mem\_trace output\_file minlen maxlen topk
```

For example, to run DG-Miner on the 456.hmmer trace from spec cpu2006:

```bash
cd algorithm
./DG\_Miner ../database/test/test.simpoint\_weight ../database/spec2006\_archexp/critical\_path/456.hmmer\_0/ ../database/spec2006\_archexp/pc\_mem\_trace/trace\_1M-456.hmmer\_0\_simpoint\_instruction\_flow spec2006-456.hmmer\_0.txt 2 10 0.2
```

## stat utility number of database

```bash
cd algorithm
g++ -std=c++17 stat\_utility.cpp -lstdc++fs -o stat\_utility
./stat\_utility ../database/spec2006\_archexp/critical\_path/ ../database/spec2017\_archexp/critical\_path/
```

