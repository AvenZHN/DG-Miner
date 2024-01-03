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
