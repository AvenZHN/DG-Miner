#include <vector>
#include <string>
#include <iostream>
#include <sstream>
#include <stdlib.h>
#include <time.h>
#include <bits/stdc++.h>
#include <fstream>
#include <time.h>
#include <cassert>
#include <filesystem>
//#define PRINT
#define ARCHEXP
typedef unsigned long DWORD;
#define MEMU_STATIC
#define MEMU_IMPLEMENTATION
#include "memu.h"
using namespace std;

#define DELTA_DEF 10000

//////////////////////////
struct seqdb                 
{
    int id;                         // Sequence ID
    vector<int> instrNum;           // Instruction numbers
    vector<unsigned int> pc;        // Program counters
    vector<int> vertexType;         // Vertex types for the sequence
    vector<int> event;              // Events in the sequence
    vector<int> weight;             // Horizontal weight
    double simpoint_weight;         // Vertical weight for Simpoint
}; 

vector<seqdb> sDB;                 // Sequence database
///////////////////////////

// Calipers Model
class CalipersModel {
public:
    std::array<std::string, 5> nodes;       // Nodes representing different stages in the Calipers model
    std::vector<std::vector<int>> edges;    // Edges defining the transitions between Calipers model stages
    std::array<std::string, 22> events;     // Events associated with the Calipers model

    CalipersModel() {
        // Initialize nodes and events
        nodes[0] = "InstrFetch";
        nodes[1] = "InstrDispatch";
        nodes[2] = "InstrExecute";
        nodes[3] = "MemExecute";
        nodes[4] = "InstrCommit";

        events[0] = "DispatchAfterFetch";   // F-D
        events[1] = "ExecuteAfterDispatch";   // D-E
        events[2] = "MemoryExecuteAfterInstructionExecute";   // E-M
        events[3] = "CommitAfterMemoryExecute";   // M-C
        events[4] = "CommitAfterExecute";   // E-C
        events[5] = "LimitedFetchBandwidth";   // F-F
        events[6] = "LimitedDispatchBandwidth";   // D-D
        events[7] = "LimitedCommitBandwidth";   // C-C
        events[8] = "LimitedMemoryCommitBandwidth";   // C-C
        events[9] = "BadFetch";   // E-F
        events[10] = "GoodFetch";   // F-F
        events[11] = "InOrderDispatch";  // D-D
        events[12] = "InOrderCommit";  // C-C
        events[13] = "LimitedInstructionBufferSize";   // C-F
        events[14] = "LimitedMemoryIssueBandwidth";   // M-M
        events[15] = "StoreToLoadForwarding_StoreAfterLoadStoreFromToSameAddress";   // M-M
        events[16] = "RegisterDataDependenceByLoad";   // M-E
        events[17] = "RegisterDataDependenceByOther";   // E-E
        events[18] = "LimitedIssueBandwidth";   // E-E
        events[19] = "LimitedInstructionQueueSize";   // E-E
        events[20] = "ResourceDependence";   // E-E
        events[21] = "LimitedLoadStoreQueueSize";    // M-M
        // ...

        // Initialize edges representing transitions between Calipers model stages
        edges = {
            // Commented transitions {from, to}
            {0, 0}, {0, 1}, {1, 1}, {1, 2}, {2, 0}, {2, 2}, {2, 3}, {2, 4},
            {3, 2}, {3, 3}, {3, 4}, {4, 0}, {4, 4}
        };
    }
};

// ArchExplorer Model
class ArchexpModel {
public:
    std::array<std::string, 13> nodes;          // Nodes representing different stages in the ArchExplorer model
    std::vector<std::vector<int>> edges;        // Edges defining the transitions between ArchExplorer model stages
    std::array<std::string, 17> events;         // Events associated with the ArchExplorer model

    ArchexpModel() {
        // Initialize nodes and events
        nodes[0] = "F1";
        nodes[1] = "F2";
        nodes[2] = "F";
        nodes[3] = "F2F";
        nodes[4] = "D";
        nodes[5] = "R";
        nodes[6] = "DS";
        nodes[7] = "I";
        nodes[8] = "DI";
        nodes[9] = "M";
        nodes[10] = "P";
        nodes[11] = "MP";
        nodes[12] = "C";

        events[0] = "Base";
        events[1] = "IcacheMiss";
        events[2] = "DcacheMiss";
        events[3] = "BPMiss";
        events[4] = "LackROB";
        events[5] = "LackLQ";
        events[6] = "LackSQ";
        events[7] = "LackIntRF";
        events[8] = "LackFpRF";
        events[9] = "LackIQ";
        events[10] = "LackIntAlu";
        events[11] = "LackIntMultDiv";
        events[12] = "LackFpAlu";
        events[13] = "LackFpMultDiv";
        events[14] = "LackRdWrPort";
        events[15] = "RAW";
        events[16] = "Virtual";

        // Initialize edges representing transitions between ArchExplorer model stages
        edges = {
            // Commented transitions {from, to}
            {0, 1}, {0, 10}, {0, 3}, {0, 5}, {0, 7}, {0, 8},
            {1, 2}, {10, 0}, {10, 12}, {11, 12}, {2, 4}, {3, 4},
            {4, 5}, {5, 10}, {5, 5}, {5, 6}, {5, 7}, {5, 8},
            {6, 7}, {7, 0}, {7, 10}, {7, 11}, {7, 5}, {7, 7},
            {7, 8}, {8, 10}, {8, 11}, {8, 5}, {8, 7}, {8, 8}
        };
    }
};

/////////////////////////

vector<vector<int>> *freArr;       // Pointer to store frequent patterns
vector<vector<int>> *canArr;       // Pointer to store candidate patterns
vector<vector<int>> candidate;     // Vector to store candidate patterns
vector<vector<int>> simplepatt;    // Vector to store simple patterns

unsigned int minpc, maxpc, deltapc;   // Variables to store minimum, maximum, and delta values for program counters

map<int, string> pc_code;    // Map to associate program counter values with corresponding codes

int DELTA = DELTA_DEF;    // Default value for the number of equally divided segments based on program counters (PCs)

// Function to compare two vectors up to a specified length
int compare(vector<int> a, vector<int> b, int len) {
    int result = 0;
    for (int i = 0; i < len; i++) {
        if (a[i] != b[i]) {
            if (a[i] < b[i]) {
                return -1;
            } else {
                return 1;
            }
        }
    }
    if (a.size() == len) {
        return 0;
    } else if (a.size() < len) {
        return -1;
    } else {
        return 1;
    }
}

// Binary search to find the first position of cand in the level of freArr
int binary_search(int level, vector<int> cand, int low, int high) {
    int mid, start;
    if (low > high) {
        return -1;
    }
    while (low <= high) {
        mid = (high + low) / 2;
        int result = compare(cand, canArr[level - 1][mid], level - 1);  // To avoid multiple calls to the same function
        if (result == 0) {
            int slow = low;
            int shigh = mid;
            int flag = -1;
            if (compare(cand, canArr[level - 1][low], level - 1) == 0) {
                start = low;
            } else {
                while (slow < shigh) {
                    start = (slow + shigh) / 2;
                    int sresult = compare(cand, canArr[level - 1][start], level - 1);
                    if (sresult == 0) {  // Only two cases of ==0 and >0
                        shigh = start;
                        flag = 0;
                    } else {
                        slow = start + 1;
                    }
                }
                start = slow;
            }
            return start;
        } else if (result < 0) {
            high = mid - 1;
        } else {
            low = mid + 1;
        }
    }
    return -1;
}

// Generate candidate patterns for the specified level
void gen_candidate(int level) {
    for (auto p : canArr[level - 1]) {
        // Extract suffix of pattern p
        vector<int> p_suffix(p.begin() + 1, p.begin() + level);
        for (auto q : canArr[level - 1]) {
            // Extract prefix of pattern q
            vector<int> q_prefix(q.begin(), q.begin() + level - 1);
            if (p_suffix == q_prefix) {
                // Concatenate patterns p and q to create a candidate pattern
                vector<int> cand = p;
                cand.push_back(q.back());

                // Check if the candidate pattern already exists in the result
                bool hascand = false;
                for (auto ci : candidate) {
                    if (cand == ci) {
                        hascand = true;
                        break;
                    }
                }

                // Add the candidate pattern to the result if it's unique
                if (!hascand) {
                    candidate.push_back(cand);
                }
            }
        }
    }
}


// Function to invert the key-value mapping of a map to a multimap
// Input: map<int, int> mymap - Original map to be inverted
// Output: multimap<int, int> - Inverted multimap with values as keys and keys as values
multimap<int, int> invert(map<int, int> & mymap) {
    multimap<int, int> multiMap; // Create an empty multimap to store the inverted mapping

    // Iterate over each key-value pair in the original map
    map<int, int>::iterator it;
    for (it = mymap.begin(); it != mymap.end(); it++) {
        // Insert the inverted pair into the multimap (value as key, key as value)
        multiMap.insert(make_pair(it->second, it->first));
    }

    // Return the inverted multimap
    return multiMap;
}

// Function to generate parameters for mining based on candidate patterns and utility distribution
// Input: 
//   - double & minsup: Minimum support threshold to be determined
//   - double & minu: Minimum utility threshold to be determined
//   - double & bound: Upper bound for utility to be determined
//   - int & minpecusup: Minimum utility for performance exception to be determined
//   - double topk: Top-k percentage for determining minsup
void gen_param(double & minsup, double & minu, double & bound, int & minpecusup, double topk) {
    int candnum = candidate.size();  // Number of candidate patterns
    map<int, int> udist = {};  // Utility distribution map
    vector<int> cand_pau;  // Vector to store pattern utility sums
    int total_utility = 0;  // Total utility across all patterns
    int total_sup = 0;  // Total support across all patterns

    // Loop over each candidate pattern
    for (int ci = 0; ci < candnum; ci++) {
        vector<int> p = candidate[ci];  // Current candidate pattern
        int pau = 0;  // Pattern utility sum

        // Loop over each sequence in the sequence database
        for (int t = 0; t < sDB.size(); t++) {
            for (int si = 0; si <= sDB[t].vertexType.size() - 2; si++) {
                // Check if the current sequence contains the candidate pattern
                if (sDB[t].vertexType[si] == p[0] and sDB[t].vertexType[si + 1] == p[1]) {
                    int itemWeight = sDB[t].weight[si + 1];  // Utility of the second item in the pattern
                    pau += itemWeight;  // Add the utility to the pattern utility sum
                    total_sup++;  // Increment total support count

                    // Update utility distribution map
                    if (udist.find(itemWeight) == udist.end()) {
                        udist[itemWeight] = 1;
                        total_utility += itemWeight;
                    } else {
                        udist[itemWeight] += 1;
                    }
                }
            }
        }

        // If pattern utility sum is greater than 0, add it to the cand_pau vector
        if (pau > 0)
            cand_pau.push_back(pau);
    }

    // Sort the cand_pau vector in descending order
    sort(cand_pau.begin(), cand_pau.end(), greater<int>());

    // Determine minsup based on top-k percentage
    minsup = cand_pau[int(cand_pau.size() * topk)];
    cout << "minsup:" << minsup << endl;

    int sum_sup = 0;

    // Determine bound and minpecusup based on utility distribution
    for (auto ui = udist.begin(); ui != udist.end(); ui++) {
        sum_sup += ui->second;

        // Check if the cumulative support reaches a threshold
        if (ui->first == 0) continue;
        if (sum_sup * 1.0 / total_sup > 0.99) {
            bound = ui->first;  // Set bound to the current utility value
            minpecusup = (++ui)->first;  // Set minpecusup to the next utility value
            break;
        }
    }

    // Set minu to the minimum utility value in the utility distribution
    minu = udist.begin()->first;
#ifdef PECU
    minu = minpecusup;
#endif
    cout << "minu:" << minu << endl;

#ifdef MAXBOUND
    // Set bound to the maximum utility value in the utility distribution
    bound = udist.rbegin()->first;
#endif
    cout << "bound:" << bound << endl;

    cout << "minpecusup:" << minpecusup << endl;
}

// Function to split a string into a vector of substrings based on a delimiter
// Input:
//   - const string &s: Input string to be split
//   - char delim: Delimiter character for splitting the string
// Output:
//   - vector<string>: Vector of substrings obtained by splitting the input string
vector<string> split(const string &s, char delim) {
    vector<string> result;  // Vector to store the resulting substrings
    stringstream ss(s);  // Create a stringstream with the input string
    string item;  // Variable to store each substring

    // Loop through each substring separated by the specified delimiter
    while (getline(ss, item, delim)) {
        result.push_back(item);  // Add the substring to the result vector
    }

    return result;  // Return the vector of substrings
}

// Function to read data from a file and populate the sDB (sequence database) structure
// Input:
//   - const filesystem::path namepath: File path to read data from
//
// Input format:
// instrNum vertexType weight stallEventType 
// (the first vertex: instrNum=0, weight = -1/0.)
// ...
void read_file(const filesystem::path namepath) {
    const char *namefile = namepath.c_str();  // Convert filesystem path to C-style string
    ifstream file;  // Input file stream
    string buff;  // Buffer to store each line of the file

    file.open(namefile, ios::in);  // Open the file for reading
    int id = atoi(namepath.filename().c_str());  // Extract the numerical ID from the file name

    // Check if the file ID exists in the sDB vector
    bool found = false;
    for (int i = 0; i < sDB.size(); i++) {
        if (id == sDB[i].id) {
            found = true;
            break;
        }
    }

    // Display an error message if the file ID is not found in the sDB vector
    if (!found)
        cout << "cannot get file id " << namefile << " in simpoint weight data" << endl;
    assert(found);

    // Read data from the file line by line
    while (getline(file, buff)) {
        vector<string> buffv = split(buff, ' ');  // Split the line into a vector of substrings

        // Populate the sDB structure with the parsed data
        sDB[id].instrNum.push_back(atoi(buffv[0].c_str()));
        sDB[id].vertexType.push_back(atoi(buffv[1].c_str()));
        sDB[id].weight.push_back(atoi(buffv[2].c_str()));
        sDB[id].event.push_back(atoi(buffv[3].c_str()));
    }

    file.close();  // Close the file after reading
}

// Function to read simpoint weight data from a file and populate the sDB (sequence database) structure
// Input:
//   - const char *namefile: C-style string representing the file path
//
// Input format:
// weight0 id0 
// weight1 id1 
// ...
void read_simpoint_weight_file(const char *namefile) {
    ifstream file;  // Input file stream
    string buff;    // Buffer to store each line of the file
    file.open(namefile, ios::in);  // Open the file for reading

    int i = 0;  // Counter for iterating through the sDB vector
    while (getline(file, buff)) {
        vector<string> buffv = split(buff, ' ');  // Split the line into a vector of substrings

        // Create a new seqdb object and populate its fields with parsed data
        sDB.push_back(seqdb());
        sDB[i].id = atoi(buffv[1].c_str());
        sDB[i].simpoint_weight = atof(buffv[0].c_str());

        // Display debug information
        // cout << "simpoint : " << i << ":" << sDB[i].id << " " << sDB[i].simpoint_weight << endl;

        i++;  // Increment the counter
    }

    file.close();  // Close the file after reading
}

// Function to read PC memory trace data from a file and populate the sDB (sequence database) structure
// Input:
//   - const char *namefile: C-style string representing the file path
//
// Input format:
// @I pc code
// @F xx
// @B xx
// @I pc code @A addr
// @F xx
// @B xx
// @M xx
// ...
// line number is instrNum
void read_pc_mem_trace(const char *namefile) {
    ifstream file;          // Input file stream
    vector<string> lines;    // Vector to store lines containing PC information
    string buff;             // Buffer to store each line of the file
    file.open(namefile, ios::in);  // Open the file for reading

    // Iterate through the file and collect lines starting with "@I"
    while (getline(file, buff)) {
        if (buff.rfind("@I", 0) == 0) {
            lines.push_back(buff);
            // cout << "line" << lines.size() << ": "<< buff << endl;  // Display debug information
        }
    }

    // Iterate through sDB entries and match with corresponding PC information
    for (int i = 0; i < sDB.size(); i++) {
        for (int j = 0; j < sDB[i].instrNum.size(); j++) {
            int itemInstrNum = sDB[i].instrNum[j];  // sDB.instrNum starts from 0
            string line = lines[itemInstrNum];
            vector<string> linev = split(line, ' ');

            // Extract PC and code information from the line
            unsigned int itempc = stoul(linev[1], nullptr, 16);
            string code = linev[2];
            for (int si = 3; si < linev.size(); si++) {
                if (linev[si] == "@A")
                    break;
                code += ' ';
                code += linev[si];
            }

            // Display debug information
            // cout << "pc:" << itempc << " code:" << code << endl;

            // Update pc_code map with PC and code information
            if (pc_code.find(itempc) == pc_code.end()) {
                pc_code[itempc] = code;
            }

            // Populate sDB fields with PC information
            sDB[i].pc.push_back(itempc);

            // Display debug information
            // cout << itemInstrNum << "-" << line << ":" << itempc << endl;

            // Update minpc, maxpc based on PC values
            if ((i == 0) && (j == 0)) {
                minpc = itempc;
                maxpc = itempc;
            } else {
                if (itempc < minpc)
                    minpc = itempc;
                if (itempc > maxpc)
                    maxpc = itempc;
            }
        }
    }

    // Adjust DELTA and deltapc based on minpc, maxpc, and specified threshold values
    if ((maxpc - minpc) <= DELTA)
        DELTA = 1;
    deltapc = (maxpc - minpc) / DELTA;
    if (deltapc < 10) {
        // If the delta pc is too small, regenerate DELTA and deltapc
        DELTA /= 100;
        deltapc = (maxpc - minpc) / DELTA;
    }

    // Display debug information
    cout << "minpc:" << minpc << " maxpc:" << maxpc << " deltapc:" << deltapc << endl;

    file.close();  // Close the file after reading
}

// Function to retrieve the current system tick count in milliseconds
// Returns:
//   - unsigned long: Current system tick count in milliseconds
unsigned long GetTickCount() {
    struct timespec ts;                           // Structure to store timespec information
    clock_gettime(CLOCK_MONOTONIC, &ts);          // Get the current monotonic time
    return (ts.tv_sec * 1000 + ts.tv_nsec / 1000000);  // Convert to milliseconds and return
}

// Function to generate short patterns from frequent patterns at a specific level
// Parameters:
//   - int level: The level for which short patterns are generated
// Returns:
//   - bool: Indicates whether new short patterns were generated (true) or not (false)
bool gen_shortpatt(int level) {
    bool gennew = false;  // Flag to track whether new short patterns were generated

    // Iterate through the frequent patterns at the specified level
    for (auto ph : freArr[level]) {
        vector<int> short_patt;  // Vector to store the generated short pattern
        short_patt.push_back(ph[0]);  // Add the first element of the frequent pattern
        short_patt.push_back(ph[1]);  // Add the second element of the frequent pattern

        // Check if the frequent pattern has more than two elements
        if (ph.size() > 2) {
            short_patt.push_back(ph[2]);  // Add the third element of the frequent pattern

            // Iterate through the remaining elements of the frequent pattern
            for (int phi = 2; phi < ph.size() - 1; phi++) {
                // Check if the current element is not a repetition of the adjacent elements
                if (!(ph[phi] == ph[phi - 1] and ph[phi + 1] == ph[phi])) {
                    short_patt.push_back(ph[phi + 1]);  // Add the next unique element to the short pattern
                }
            }
        }

        bool had = false;

        // Iterate through existing simple patterns
        for (auto sph : simplepatt) {
            // Check if the size of the existing simple pattern matches the generated short pattern
            if (sph.size() == short_patt.size()) {
                bool found = true;

                // Compare each element of the existing simple pattern with the generated short pattern
                for (int si = 0; si < sph.size(); si++) {
                    if (sph[si] != short_patt[si]) {
                        found = false;
                        break;
                    }
                }

                // If a match is found, set the 'had' flag to true and break the loop
                if (found) {
                    had = true;
                    break;
                }
            }
        }

        // If the generated short pattern is not found in existing patterns, add it to simplepatt
        if (!had) {
            gennew = true;
            simplepatt.push_back(short_patt);
        }
    }

    // Return whether new short patterns were generated
    return gennew;
}

// Function to print patterns contained in a vector of vectors of integers
// Parameters:
//   - vector<vector<int>> patts: The vector of patterns to be printed
void print_patts(vector<vector<int>> patts) {
    // Iterate through the vector of patterns
    for (auto ffl : patts) {
        // Iterate through each pattern in the vector
        for (auto ffli : ffl) {
            cout << ffli;  // Print each element of the pattern
        }
        cout << "  ";  // Add a space between patterns for better readability
    }
    cout << endl;  // Move to the next line after printing all patterns
}

#include <iostream>
#include <iomanip>
#include <fstream>
#include <sstream>
#include <vector>
#include <cmath>
#include <cassert>
#include <filesystem>
#include <ctime>
#include <map>
#include <algorithm>
#include <unistd.h>
#include <sys/time.h>

using namespace std;

// Function to split a string into a vector of substrings based on a delimiter
vector<string> split(const string &s, char delim) {
    vector<string> result;
    stringstream ss(s);
    string item;

    while (getline(ss, item, delim)) {
        result.push_back(item);
    }

    return result;
}

// Structure to store information related to each sequence database
struct seqdb {
    int id;
    vector<int> instrNum;
    vector<int> vertexType;
    vector<int> weight;
    vector<int> event;
    vector<int> pc;
    double simpoint_weight;
};

// Function to read a file containing simpoint weights and populate the seqdb structure
void read_simpoint_weight_file(const char *namefile);

// Function to read a file containing trace information and populate the seqdb structure
void read_file(const filesystem::path namepath);

// Function to read a file containing PC and memory trace information and populate pc_code
void read_pc_mem_trace(const char *namefile);

// Function to convert a map<key, value> to a multimap<value, key>
multimap<int, int> invert(map<int, int> &mymap);

// Function to generate candidate patterns for a given level
void gen_candidate(int level);

// Function to compare two vectors of integers up to a specified length
int compare(vector<int> a, vector<int> b, int len);

// Function to perform binary search on the candidate array
int binary_search(int level, vector<int> cand, int low, int high);

// Function to generate candidate patterns for a given level based on frequent patterns of the previous level
void gen_candidate(int level);

// Function to compare two vectors of integers up to a specified length
int compare(vector<int> a, vector<int> b, int len);

// Function to perform binary search on the candidate array
int binary_search(int level, vector<int> cand, int low, int high);

// Function to generate frequent patterns by eliminating redundant elements
bool gen_shortpatt(int level);

// Function to print patterns contained in a vector of vectors of integers
void print_patts(vector<vector<int>> patts);

// Function to get the current system time in milliseconds
unsigned long GetTickCount();

// Function to calculate various parameters needed for the algorithm
void gen_param(double &minsup, double &minu, double &bound, int &minpecusup, double topk);

int main(int argc, const char *argv[]) {
    assert(argc >= 8);

    // Read simpoint weight and build seqdb
    read_simpoint_weight_file(argv[1]);

    // Read trace files in the specified directory
    string path = argv[2];
    cout << "+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++" << endl;
    for (const auto &entry : filesystem::directory_iterator(path)) {
        cout << "trace:" << entry.path() << endl;
        read_file(entry.path());
    }

#ifdef PRINT
    cout << "pc trace:" << argv[3] << endl;
    read_pc_mem_trace(argv[3]);
#endif

    ofstream outfile;
    outfile.open(argv[4], ios::out);

    // Length constraint
    int minlen = atoi(argv[5]);
    int maxlen = atoi(argv[6]);
    double topk = atof(argv[7]);

    // Other parameters
    double minsup = 0;
    double minu = 0;
    double bound = 0;
    int minpecusup = 0;  // Minimum peculiarity value support

    if (argc > 8) {
        minsup = atof(argv[8]);
        minu = atof(argv[9]);
        bound = atof(argv[10]);
        minpecusup = atoi(argv[11]);  // Minimum peculiarity value support
    }

    int frenum = 0;  // Number of frequent patterns
    vector<vector<int>> *freArr;
    vector<vector<int>> *canArr;
    vector<double> *freArrHUP;
    vector<int> *freArrSUP;
    vector<double> *freArrPecu;

    freArr = new vector<vector<int>>[maxlen];
    canArr = new vector<vector<int>>[maxlen];
    freArrHUP = new vector<double>[maxlen];  // Frequency patterns average utility support
    freArrSUP = new vector<int>[maxlen];    // Frequency patterns support
    freArrPecu = new vector<double>[maxlen]; // Frequency patterns Peculiarity value

    int numbS = sDB.size();
    int cannum = 0;
    int global_range_pc[DELTA];
    vector<vector<int>> global_range_pc_patt;

    for (int gi = 0; gi < DELTA; gi++) {
        global_range_pc[gi] = 0;
        global_range_pc_patt.push_back({});
    }

    int f_level = 1; // Minimum length of pattern is 2

#if defined(CALIPERS)
    CalipersModel model;
#elif defined(ARCHEXP)
    ArchexpModel model;
#else
    cerr << "unknown model" << endl;
#endif

    candidate = model.edges;

    gen_param(minsup, minu, bound, minpecusup, topk);
    DWORD begintime = GetTickCount();

    while (candidate.size() != 0) {
        for (int i = 0; i < candidate.size(); i++) {
            vector<int> p = candidate[i];
            cannum++;
            double supd = 0; // seq sup
            int sup = 0;
            int ptn_len = p.size();

#ifdef PRINT
            vector<int> occurence[numbS]; // Start position of occurrence
            int local_range_pc[DELTA];

            for (int ri = 0; ri < DELTA; ri++) {
                local_range_pc[ri] = 0;
            }

            int pecu = 0; // Peculiarity value number of candidate
            vector<int> allpos;
            ostringstream os;
#endif

            for (int t = 0; t < numbS; t++) {
                if (sDB[t].vertexType.size() > 0) {
                    int sups = 0; // seq sup

                    for (int si = 0; si <= sDB[t].vertexType.size() - ptn_len; si++) {
                        if (sDB[t].vertexType[si] == p[0]) {
                            bool success = true;

                            for (int pi = 1; pi < ptn_len; pi++) {
                                if (sDB[t].vertexType[si + pi] != p[pi]) {
                                    success = false;
                                    break;
                                }

#ifdef ARCHEXP
                                // ArchExplorer use Virtual (event 16) to connect DEG,
                                // but Virtual has no use in performance.
                                // So get rid of it in frequent patterns.
                                if (sDB[t].event[si + pi] == 16) {
                                    success = false;
                                    break;
                                }
#endif
                            }

                            if (success) {
#ifdef PRINT
                                occurence[t].push_back(si);
#endif
                                sup++;
                                int pu = 0;

                                for (int wi = 1; wi < ptn_len; wi++) {
                                    int itemWeight = sDB[t].weight[si + wi];
                                    pu += itemWeight;

#ifdef PRINT
                                    if (itemWeight >= minpecusup) {
                                        pecu++;
                                    }
#endif
                                }

                                if (pu >= minu * (ptn_len - 1)) {
                                    sups += pu;

#ifdef PRINT
                                    int startpc = sDB[t].pc[si]; // Start pc
                                    int pos = (startpc - minpc) / deltapc;

                                    if (pos >= DELTA)
                                        pos = DELTA - 1;

                                    local_range_pc[pos]++;
                                    global_range_pc[pos]++;
                                    allpos.push_back(pos);

                                    // Dump out trace
                                    for (auto pi : p) {
                                        os << pi;
                                    }

                                    os << ' ';

                                    // The event type store in the end point instruction, so start at si+1
                                    for (int ei = si + 1; ei < si + ptn_len; ei++) {
                                        os << model.events[sDB[t].event[ei]];

                                        if (ei < si + ptn_len - 1)
                                            os << "-";
                                    }

                                    os << ' ' << dec << si << ' ' << hex << "0x" << startpc << ' ' << dec << pu << endl;

                                    for (int pcci = 0; pcci < ptn_len; pcci++) {
                                        int patt_pc = sDB[t].pc[si + pcci];
                                        os << "*\t" << hex << "0x" << patt_pc << ":" << pc_code[patt_pc] << endl;
                                    }
#endif

                                    si += ptn_len - 1 - 1; // -1 for si++
                                }
                            }
                        }
                    }

                    supd += sups * sDB[t].simpoint_weight;
                }
            }

            double hup = supd / (ptn_len - 1);

#ifdef PRINT
            int occnum = 0;
            double avgpecu = 0.0;

            for (int oci = 0; oci < numbS; oci++) {
                occnum += occurence[oci].size();
            }

            if (occnum > 0)
                avgpecu = (double)pecu / (double)occnum;
#endif

            if (hup >= double(minsup)) {
                freArr[f_level].push_back(p);
                canArr[f_level].push_back(p);

#ifdef PRINT
                outfile << os.str();
                freArrSUP[f_level].push_back(occnum);
                freArrHUP[f_level].push_back(hup);
                freArrPecu[f_level].push_back(avgpecu);
#endif

                if ((f_level >= minlen - 1) && (f_level <= maxlen - 1)) {
#ifdef PRINT
                    for (int gi = 0; gi < DELTA; gi++) {
                        global_range_pc_patt[gi].push_back(0);
                    }

                    for (auto posi : allpos) {
                        int old = global_range_pc_patt[posi].back();
                        global_range_pc_patt[posi].pop_back();
                        global_range_pc_patt[posi].push_back(old + 1);
                    }

                    // Print result of frequent candidate
                    cout << "[" << frenum << "]";
                    for (auto pi : p) {
                        cout << pi;
                    }

                    cout << ":" << occnum << ":" << hup << ":" << avgpecu << "|";

                    for (int ii = 0; ii < DELTA; ii++) {
                        if (local_range_pc[ii] > DELTA)
                            cout << ii << ":" << local_range_pc[ii] << ",";
                    }

                    cout << endl;
#endif
                    frenum++;
                }
            } else {
                if (sup * bound >= double(minsup)) {
                    canArr[f_level].push_back(p);

#ifdef PRINT
                    cout << "[c]";

                    for (auto pi : p) {
                        cout << pi;
                    }

                    cout << ":" << sup << ":" << hup << ":" << avgpecu << endl;
#endif
                }
            }
        }

        f_level++;

#ifndef NOCPJM
        if (not gen_shortpatt(f_level - 1))
            break;
#endif

        if (f_level >= maxlen)
            break;

        candidate.clear();
        gen_candidate(f_level);
    }

    DWORD endtime = GetTickCount();
    size_t peak_memu = memu_get_peak_rss();
    size_t curr_memu = memu_get_curr_rss();

    cout << "================== result =====================" << endl;

    // Variables for storing top patterns and their hup values
    int top1i, top1j, top2i, top2j, top3i, top3j;
    double huptop1 = 0;
    double huptop2 = 0;
    double huptop3 = 0;
    int freid = 0;

    // Loop through the patterns
    for (int i = minlen - 1; (i < maxlen) && (i < f_level); i++) {
        for (int j = 0; j < freArr[i].size(); j++) {
            cout << "[" << freid++ << "]";

            // Print the current pattern
            for (auto ff : freArr[i][j]) {
                cout << ff;
            }

#ifdef PRINT
            // Print additional information if PRINT is defined
            cout << ":" << freArrSUP[i][j] << ":" << freArrHUP[i][j] << ":" << freArrPecu[i][j] << "\t";

            // Find the top 3 patterns with highest hup values
            if (frenum > 3) {
                double thishup = freArrHUP[i][j];
                if (thishup > huptop1) {
                    if (huptop1 > huptop2) {
                        if (huptop2 > huptop3) {
                            huptop3 = huptop2;
                            top3i = top2i;
                            top3j = top2j;
                        }
                        huptop2 = huptop1;
                        top2i = top1i;
                        top2j = top1j;
                    }
                    huptop1 = thishup;
                    top1i = i;
                    top1j = j;
                } else if (thishup > huptop2) {
                    if (huptop2 > huptop3) {
                        huptop3 = huptop2;
                        top3i = top2i;
                        top3j = top2j;
                    }
                    huptop2 = thishup;
                    top2i = i;
                    top2j = j;
                } else if (thishup > huptop3) {
                    huptop3 = thishup;
                    top3i = i;
                    top3j = j;
                }
            }
#endif
        }
        cout << endl;
    }

    // Print concise patterns
    cout << "concise patts:" << endl;
    for (int j = 0; j < simplepatt.size(); j++) {
        for (auto ff : simplepatt[j]) {
            cout << ff;
        }
        cout << "  ";
    }
    cout << endl;

#ifdef PRINT
    // Print top 3 patterns with highest hup values if PRINT is defined
    if (frenum > 3) {
        cout << "top hup: 1.";
        for (auto t : freArr[top1i][top1j]) {
            cout << t;
        }
        cout << " 2.";
        for (auto t : freArr[top2i][top2j]) {
            cout << t;
        }
        cout << " 3.";
        for (auto t : freArr[top3i][top3j]) {
            cout << t;
        }
        cout << endl;
    }

    // Print information about the top pc region
    cout << "-------------------- top pc region -----------------------------" << endl;
    int max_global_range_pc = global_range_pc[0];
    for (int ii = 0; ii < DELTA; ii++) {
        if (global_range_pc[ii] > max_global_range_pc) {
            max_global_range_pc = global_range_pc[ii];
        }
    }
    for (int ii = 0; ii < DELTA; ii++) {
        if (global_range_pc[ii] > max_global_range_pc * 0.5) {
            cout << ii << ":" << global_range_pc[ii] << endl;
            for (int fi = 0; fi < global_range_pc_patt[ii].size(); fi++) {
                cout << "[" << fi << "]:" << global_range_pc_patt[ii][fi] << ",";
            }
            cout << endl;
            int endpc = (ii == DELTA - 1) ? maxpc + 1 : (ii + 1) * deltapc + minpc;
            for (int pci = ii * deltapc + minpc; pci < endpc; pci++) {
                if (pc_code.find(pci) != pc_code.end()) {
                    cout << "\t0x" << hex << pci << dec << ":" << pc_code[pci] << endl;
                }
            }
        }
    }
#endif

    // Output footer
    cout << endl << "----------------------------------------------------------------" << endl;

    // Print parameters and statistics
    cout << "minsup=" << minsup << " minpecusup=" << minpecusup << " minlen=" << minlen << " maxlen=" << maxlen << endl;
    cout << "minu=" << minu << " bound=" << bound << endl;
    cout << "The number of frequent patterns:" << frenum << endl;
    cout << "The time-consuming:" << endtime - begintime << "ms. " << endl;
    cout << "The number of calculation:" << cannum << " " << endl;
    cout << "The number of simple patts:" << simplepatt.size() << " " << endl;
    cout << "Max len of frequent patts:" << f_level << " " << endl;
    cout << "The peak memory usage:" << (double) peak_memu / 1024.0 / 1024.0 / 8.0 << "Mb." << endl;
    cout << "The curr memory usage:" << (double) curr_memu / 1024.0 / 1024.0 / 8.0 << "Mb." << endl;
    cout << endl;

    // Get current time and write results to a file
    time_t t = time(0);
    char tmp[32];
    strftime(tmp, sizeof(tmp), "%Y-%m-%d %H:%M:%S", localtime(&t));
    ofstream resultfile;
    resultfile.open("result_file.txt", ios::app);

    resultfile << tmp << "\t"
               << "simpoint weight:" << argv[1] << "\tcritical path trace:" << argv[2] << "\tpc_mem_trace:" << argv[3]
               << "\tminsup:" << minsup << "\tminpecusup:" << minpecusup << "\t[minlen,maxlen]:"
               << "[" << minlen << "," << maxlen << "]\n"
               << "time-consuming:" << endtime - begintime << "ms\t#freq pattern:" << frenum
               << "\t#candidate:" << cannum << "\tpeak memory usage:" << peak_memu << "B\t\tcurrent memory usage:"
               << curr_memu << "B\n";

    // Write frequent patterns to the result file
    for (int m = 0; m < f_level; m++) {
        for (int j = 0; j < freArr[m].size(); j++) {
            for (auto ff : freArr[m][j]) {
                resultfile << ff;
            }
#ifdef PRINT
            resultfile << ":" << freArrSUP[m][j] << ":" << freArrHUP[m][j] << ":" << freArrPecu[m][j] << "\t";
#endif
        }
    }
    resultfile << endl;
    resultfile.close();

    // Close output file
    outfile.close();

    return 0;
}

