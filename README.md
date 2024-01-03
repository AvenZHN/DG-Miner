# DG-Miner  
DG-Miner is a high average utility pattern miner for critical path analysis of processor dependence graph.  
  
## How to run:  
(1) Run DG-Miner using test trace:    
cd algorithm
make    
make test    
(2) Run DG-Miner using spec cpu2006 trace:    
./DG\_Miner simpoint\_weights critical\_paths pc\_mem\_trace output\_file minpau minu upbound minpecu minlen maxlen    
i.e. run 456.0 trace:    
./DG\_Miner ../database/spec06int\_10M/simpoint\_weights/456.0 ../database/spec06int\_10M/456.0/ ../database/spec06int\_10M/pc\_mem\_trace/456.0\_pc\_mem.trace 456.txt 10000 0 2 100 2 10  
