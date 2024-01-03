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

