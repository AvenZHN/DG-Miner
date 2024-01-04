/****************************************************************************************
 * The sort_event.cpp program is designed to analyze a log file containing entries related 
 * to events and instructions. It calculates and displays the sum of weights associated 
 * with each unique event_name in the log, presenting a sorted list based on the accumulated 
 * weights. This can provide insights into the significance of different events within the log.
 * ======================================================================================
 * Functionality:
 * --------------------------------------------------------------------------------------
 * 1. Log Entry Struct:
 * Defines a LogEntry struct to represent individual entries in the log file. Entries can be 
 * of two types: "instruction" or "event."
 * 2. Parsing Log Lines:
 * Implements a parseLogLine function to parse each line from the log file into a LogEntry struct.
 * Recognizes and differentiates between lines starting with "*" as instructions and regular 
 * log entries.
 * 3. Weight Accumulation:
 * Reads the log file, accumulates weights associated with each unique event_name, and stores 
 * the results in a std::map (`eventNameWeightMap`).
 * 4. Sorting Events:
 * Creates a sorted vector of pairs (event_name, sum_of_weight) based on the accumulated weights.
 * Sorts the vector in descending order of weight.
 * 5. Displaying Results:
 * Outputs a sorted list of events along with their accumulated weights and percentages relative 
 * to the total sum of weights.
 * 6. Command-Line Usage:
 * The program expects two command-line arguments: the path to the log file (logFilePath) and the 
 * total sum of weights (sumweight).
 * Example Usage:
 * ./sort_event log_file.txt 1000
 * Output:
 * Sorted list of events by sum of weight:
 * Event Name: Event1, Sum of Weight: 500, (50.00%)
 * Event Name: Event2, Sum of Weight: 300, (30.00%)
 * Event Name: Event3, Sum of Weight: 200, (20.00%)
 * =======================================================================================
 * Conclusion:
 * This program assists in understanding the relative importance of different events in the log 
 * by analyzing their cumulative weights.
 */
#include <iostream>
#include <fstream>
#include <sstream>
#include <map>
#include <vector>
#include <algorithm>

struct LogEntry {
    std::string type;
    std::string event_id;
    std::string event_name;
    std::string position;
    std::string pc;
    int weight;
    std::string instruction_code;
};

// Function to parse a line from the log file
LogEntry parseLogLine(const std::string& line) {
    LogEntry entry;
    std::istringstream iss(line);
    
    if (line[0] == '*') {
        // Line starting with "*" represents an instruction
        entry.type = "instruction";
        iss.ignore(2);  // Skip "* "
        std::getline(iss, entry.pc, ':');  // Read the PC until ':'
        std::getline(iss >> std::ws, entry.instruction_code);  // Read the rest of the line as instruction_code
    } else {
        // Regular log entry
        entry.type = "event";
        iss >> entry.event_id >> entry.event_name >> entry.position >> entry.pc >> entry.weight;
    }

    return entry;
}

int main(int argc, char* argv[]) {
    if (argc != 3) {
        std::cerr << "Usage: " << argv[0] << " <logFilePath> <sumweight>" << std::endl;
        return 1;
    }

    std::string logFilePath = argv[1];
    int sumweight = atoi(argv[2]);

    std::ifstream logFile(logFilePath);
    if (!logFile.is_open()) {
        std::cerr << "Unable to open the log file." << std::endl;
        return 1;
    }

    std::map<std::string, int> eventNameWeightMap;

    // Read and process each line in the log file
    std::string line;
    while (std::getline(logFile, line)) {
        LogEntry entry = parseLogLine(line);

        if (entry.type == "event") {
            // Accumulate weights for each event_name
            eventNameWeightMap[entry.event_name] += entry.weight;
        }
    }

    logFile.close();

    // Create a sorted vector of pairs (event_name, sum_of_weight)
    std::vector<std::pair<std::string, int>> sortedEventWeights(
        eventNameWeightMap.begin(), eventNameWeightMap.end());
    std::sort(sortedEventWeights.begin(), sortedEventWeights.end(),
              [](const auto& a, const auto& b) { return a.second > b.second; });

    // Display the sorted list
    std::cout << "Sorted list of events by sum of weight:" << std::endl;
    for (const auto& pair : sortedEventWeights) {
        std::cout << "Event Name: " << pair.first << ", Sum of Weight: " << pair.second << ", ("<< pair.second*100.0/sumweight << "%)"<< std::endl;
    }

    return 0;
}

