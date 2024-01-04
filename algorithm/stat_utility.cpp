/**************************************************************************
 * Overview:
 * ------------------------------------------------------------------------
 * The stat_utility.cpp program analyzes trace files to extract information 
 * about the distribution of weights or utility numbers within the traces. 
 * It recursively processes all files in the specified directories, counting 
 * occurrences of different weights and calculating their percentages in the 
 * total count. It then outputs the unique weights, their frequencies, and 
 * their percentages in the overall dataset.
 * ========================================================================
 * Functionality:
 * ------------------------------------------------------------------------
 * 1. Input Processing:
 * The program reads and processes trace files provided in a specified directory.
 * Each line in the trace files contains information about an instruction, 
 * including its number, vertex type, weight, and stall event type.
 * 2. Weight Distribution Analysis:
 * The program collects statistics on the distribution of weights (utility 
 * numbers) found in the trace files.
 * It maintains a count of occurrences for each unique weight using a map 
 * (`utility_number`).
 * 3. Output:
 * After processing all trace files, the program outputs the unique weights, 
 * their frequencies, and their percentages in the overall dataset.
 * The output includes both the raw count and the percentage of each weight 
 * relative to the total number of occurrences.
 * 4. Usage:
 * The program expects two command-line arguments, representing the paths 
 * to the directories containing the trace files.
 * It recursively processes all files in each directory, extracting and 
 * analyzing weight information.
 * Example Usage:
 * ./stat_utility /path/to/trace_directory1 /path/to/trace_directory2
 * The trace_directory1 usually is the spec2006, and the trace_directory2
 * is the spec2017.
 * =======================================================================
 * Output:
 * +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
 * trace:/path/to/trace_directory1/trace_file1.txt
 * trace:/path/to/trace_directory1/trace_file2.txt
 * ...
 * trace:/path/to/trace_directory2/trace_file1.txt
 * trace:/path/to/trace_directory2/trace_file2.txt
 * ...
 * Unique Weights: 1, 2, 3, ...
 * Frequencies: 10, 25, 15, ...
 * Percentages: "1 (20.00%)", "2 (50.00%)", "3 (30.00%)", ...
 * =======================================================================
 * Conclusion:
 * This program aids in understanding the distribution of weights within 
 * the trace files, providing insights into the prevalence of different 
 * instruction types or events based on their assigned weights.
 */


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
using namespace std;

    map<int,int> utility_number;
    int total = 0;
vector<string> split (const string &s, char delim) {
    vector<string> result;
    stringstream ss (s);
    string item;

    while (getline (ss, item, delim)) {
        result.push_back (item);
    }

    return result;
}


void read_file(const filesystem::path namepath)
{
    /* format:
     * instrNum vertexType weight stallEventType
     * (the first vertex: instrNum=0, weight = 0.)
     * ...
     */
    const char * namefile = namepath.c_str();
    ifstream file;
    string buff;
    bool first_line = true;
    file.open(namefile,ios::in);   //open weight file
    while(getline(file, buff))
    {
        if (first_line) {
    	 first_line = false;
    	 continue;
        }
    vector<string> buffv = split(buff,' ');
    int weight = atoi(buffv[2].c_str());
    if (weight > 10000)
	cout << "big" << weight<< endl;
    total++;
    if (utility_number.find(weight) == utility_number.end()) {
	utility_number[weight] = 1;
    } else {
	utility_number[weight] += 1;
    }
    //cout << weight << ":" << utility_number[weight] << endl;
    }
    file.close();
}

int main(int argc, const char *argv[])
{   
    string path = argv[1];
    cout << "+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++" << endl;
    for (const auto & entry : filesystem::recursive_directory_iterator(path))
    {
        cout << "trace:" << entry.path() << endl;
        read_file(entry.path());
    }
    path = argv[2];
    for (const auto & entry : filesystem::recursive_directory_iterator(path))
    {
        cout << "trace:" << entry.path() << endl;
        read_file(entry.path());
    }
    for (auto ui : utility_number) {
	 cout << ui.first << ", ";
    }
    cout << endl;
    for (auto ui : utility_number) {
	 cout << ui.second << ", ";
    }
    cout << endl;
    for (auto ui : utility_number) {
	 cout << "\"" << ui.first << " (" << setprecision(2) << (ui.second*100.0/total) << "%)\", ";
    }
    cout << endl;

	return 0;
	
}
