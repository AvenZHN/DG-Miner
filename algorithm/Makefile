# Target: all
# Build multiple versions of the program with different compilation flags
all:
	g++ -std=c++17 DG_Miner.cpp -lstdc++fs -o DG_Miner
	g++ -std=c++17 DG_Miner.cpp -lstdc++fs -DPRINT -o DG_Miner_prt
	g++ -std=c++17 DG_Miner.cpp -lstdc++fs -DMAXBOUND -o DG_Miner_maxbound
	g++ -std=c++17 DG_Miner.cpp -lstdc++fs -DNOCPJM -o DG_Miner_no_cpjm
	g++ -std=c++17 DG_Miner.cpp -lstdc++fs -DPECU -o DG_Miner_pecu
	g++ sort_event.cpp -o sort_event

# Target: test
# Run the program with specific parameters for testing
test:
	./DG_Miner ../database/test/test.simpoint_weight ../database/test/test ../database/test/test.pc_mem_trace result_trace.txt 2 10 0.2

