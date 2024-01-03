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
	int id;                  // sequence id
	vector<int> instrNum; 
	vector<unsigned int> pc; 
	vector<int> vertexType;              // sequence
    vector<int> event; 
    vector<int> weight;              // horizontal weight
    double simpoint_weight;          // vertical weight
}; 
vector<seqdb> sDB;                 // sequence database
///////////////////////////

// Calipers Model
class CalipersModel {
public:
    std::array<std::string,5> nodes;
    std::vector<std::vector<int>> edges;
    std::array<std::string,22> events;

    CalipersModel() {
        // Initialize caliperNodes and caliperEvents
	nodes[0] = "InstrFetch";
	nodes[1] = "InstrDispatch";
	nodes[2] = "InstrExecute";
	nodes[3] = "MemExecute";
	nodes[4] = "InstrCommit";
     /*
                     {VertexType::InstrFetch,    VertexType::InstrFetch},
                     {VertexType::InstrFetch,    VertexType::InstrDispatch}, 
                     {VertexType::InstrDispatch, VertexType::InstrDispatch},
                     {VertexType::InstrDispatch, VertexType::InstrExecute},
                     {VertexType::InstrExecute,  VertexType::InstrFetch},
                     {VertexType::InstrExecute,  VertexType::InstrExecute},
                     {VertexType::InstrExecute,  VertexType::MemExecute},
                     {VertexType::InstrExecute,  VertexType::InstrCommit},
                     {VertexType::MemExecute,    VertexType::InstrExecute}, // new 3-2
                     {VertexType::MemExecute,    VertexType::MemExecute},
                     {VertexType::MemExecute,    VertexType::InstrCommit},
                     {VertexType::InstrCommit,   VertexType::InstrFetch},
                     {VertexType::InstrCommit,   VertexType::InstrCommit}
      */

	edges = {{0,0},{0,1},{1,1},{1,2},{2,0},{2,2},{2,3},{2,4},{3,2},{3,3},{3,4},{4,0},{4,4}};

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
    }
};

// ArchExplorer Model
class ArchexpModel {
public:
    std::array<std::string,13> nodes;
    std::vector<std::vector<int>> edges;
    std::array<std::string,17> events;

    ArchexpModel() {
        // Initialize archexpNodes and archexpEvents
    	nodes[0] = "F1";             // request to fetch cache line
    	nodes[1] = "F2";             // receive from cache 
    	nodes[2] = "F";              // fetch
    	nodes[3] = "F2F";            // merge F2 & F
    	nodes[4] = "D";              // decode
    	nodes[5] = "R";              // rename
    	nodes[6] = "DS";             // dispatch
    	nodes[7] = "I";              // issue
    	nodes[8] = "DI";             // merge dispatch & issue
    	nodes[9] = "M";              // memory
    	nodes[10] =  "P";              // complete
    	nodes[11] =  "MP";             // merge memory & complete
    	nodes[12] =  "C";             // commit
        edges = {
{0,1},
{0,10},
{0,3},
{0,5},
{0,7},
{0,8},
{1,2},
{10,0},
{10,12},
{11,12},
{2,4},
{3,4},
{4,5},
{5,10},
{5,5},
{5,6},
{5,7},
{5,8},
{6,7},
{7,0},
{7,10},
{7,11},
{7,5},
{7,7},
{7,8},
{8,10},
{8,11},
{8,5},
{8,7},
{8,8}
          };
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
    }

};
//////////////////////////

vector <vector<int>> *freArr;  //store frequent patterns
vector <vector<int>> *canArr;  //store frequent patterns
vector <vector<int>> candidate; 
vector <vector<int>> simplepatt; 

unsigned int minpc, maxpc, deltapc;

map<int, string> pc_code;

int DELTA = DELTA_DEF;
//find the first position of cand in the level of freArr by binary search
int compare(vector<int> a, vector<int> b, int len)
{
    int result = 0;
    for (int i = 0; i < len; i++)
    {
        if (a[i] != b[i])
        {
            if (a[i] < b[i])
            {
                return -1;
            }
            else
            {
                return 1;
            }
        }
    }
    if (a.size()==len)
    {
        return 0;
    }
    else if (a.size() < len)
    {
        return -1;
    }
    else
        return 1;
}
int binary_search(int level,vector<int> cand,int low,int high)
{
	int mid,start;
	if (low > high)
	{
		return -1;
	}
	while (low<=high)
	{
		mid=(high+low)/2;
		int result=compare(cand,canArr[level-1][mid],level-1); //To avoid multiple calls the same function
		if (result == 0)
		{
			int slow=low;
			int shigh=mid;
			int flag=-1;
			if (compare(cand,canArr[level-1][low],level-1) == 0)
			{
				start=low;
			}
			else
			{
				while (slow<shigh)
				{
					
					start=(slow+shigh)/2;
					int sresult=compare(cand,canArr[level-1][start],level-1); 
					if (sresult==0)   //Only two cases of ==0 and >0
					{
						shigh=start;
						flag=0;
					}
					else
					{
						slow=start+1;
					}				
				}
				start=slow;
			}
			return start;
		}
		else if (result<0)
		{
			high = mid-1;
		}
		else
		{
			low = mid+1;
		}
	}
	return -1;
}

/*
void gen_candidate(int level)//使用canArr数组的模式逐层生成候选模式——拼接
{ // todo: other combination way? aaba + baaa = baaaba?
	int size = canArr[level-1].size();
	int start = 0;
	for(int i=0;i<size;i++)
	{
		vector<int> Q,R;
        int si = 1;
        int pi = 0;
        for (int j = 0; j < level-1; j++, si++, pi++)
        {
            //suffix pattern of canArr[level-1][i]
            R.push_back(canArr[level-1][i][si]);
            //prefix pattern of canArr[level-1][start]
            Q.push_back(canArr[level-1][start][pi]);
        }
		if(R!=Q)
		{
			start = binary_search(level, R, 0, size-1);
		}
		if (start<0 || start>=size)     //if not exist, begin from the first
			start=0;
		else
		{
            Q.clear();
            for (int fi = 0; fi < level-1; fi++)
            {
                Q.push_back(canArr[level-1][start][fi]);
            }
			while(Q == R)
			{
                vector<int> cand;
                for(int ci = 0; ci < level; ci++)
                {
                    cand.push_back(canArr[level-1][i][ci]);
                }
                cand.push_back(canArr[level-1][start][level-1]);
				candidate.push_back(cand);
				start=start+1;
				if (start >= size) 
				{
					start=0;
					break;
				}
                Q.clear();
                for (int fi = 0; fi < level-1; fi++)
                {
                    Q.push_back(canArr[level-1][start][fi]);
                }
			}
		}
	}
}
*/

void gen_candidate(int level)
{
    for (auto p : canArr[level-1]) {
	vector<int> p_suffix(p.begin()+1, p.begin()+level);
	for (auto q : canArr[level-1]) {
	    vector<int> q_prefix(q.begin(), q.begin()+level-1);
	    if (p_suffix == q_prefix) {
	         vector<int> cand = p;
	         cand.push_back(q.back());
	         bool hascand = false;
	         for (auto ci: candidate) {
	    	 if (cand == ci) {
	                 hascand = true;
	    	     break;
	    	 }
	         }
	         if (!hascand) {
	         	candidate.push_back(cand);
	         }
	    }
	}
    }
}

// Function to convert a map<key,value> to a multimap<value,key>
multimap<int, int> invert(map<int, int> & mymap)
{
	multimap<int, int> multiMap;

	map<int, int> :: iterator it;
	for (it=mymap.begin(); it!=mymap.end(); it++) 
	{
		multiMap.insert(make_pair(it->second, it->first));
	}

	return multiMap;
}

void gen_param(double & minsup, double & minu, double & bound, int & minpecusup, double topk)
{
    int candnum = candidate.size();
    map<int, int> udist = {};
    vector<int> cand_pau;
    int total_utility = 0;
    int total_sup = 0;

    for (int ci = 0; ci < candnum; ci++) {
	vector<int> p = candidate[ci];
	int pau = 0;
	for (int t = 0; t < sDB.size(); t++) {
	    for (int si=0;si<=sDB[t].vertexType.size()-2;si++) {  // p size == 2
	        if(sDB[t].vertexType[si]==p[0] and sDB[t].vertexType[si+1]==p[1]) {
                    int itemWeight = sDB[t].weight[si+1];
		    pau += itemWeight;
		    total_sup++;
            	    if (udist.find(itemWeight) == udist.end()) {
			udist[itemWeight] = 1;
			total_utility += itemWeight;
		    } else {
			udist[itemWeight] += 1;
		    }
		}
	    }
	}
	if (pau > 0)
	    cand_pau.push_back(pau);
    }
    //multimap<int, int> wdist = invert(udist);
    //for (auto di : wdist) {
    //	cout << di.first << ":" << di.second << endl;

    //}
    //for (auto pi : cand_pau) {
    //    cout << pi << endl;
    //}
    //cout << total_utility << endl;
    //cout << total_sup << endl;
    sort(cand_pau.begin(), cand_pau.end(),greater<int>());
    minsup = cand_pau[int(cand_pau.size()*topk)];
    cout << "minsup:" << minsup << endl;

    int sum_sup = 0;
    for (auto ui = udist.begin(); ui != udist.end(); ui++) {
	//cout << ui->first << ui->second << endl;
	sum_sup += ui->second;
	if (ui->first == 0) continue;
	if (sum_sup * 1.0 / total_sup > 0.99) {
	//if (sum_sup * (ui->first) > minsup) {
	     bound = ui->first;
	     minpecusup = (++ui)->first;
	     break;
	}
    }

    minu = udist.begin()->first;
#ifdef PECU
    minu = minpecusup;
#endif
    cout << "minu:" << minu << endl;
#ifdef MAXBOUND
    // use max utility as bound
    bound = udist.rbegin()->first;
#endif
    cout << "bound:" << bound << endl;

    //int sum_utility = 0;
    //for (auto ui = udist.rbegin(); ui != udist.rend(); ui++) {
    //    //cout << ui->first << " " << ui->second << endl;
    //    sum_utility ++;
    //    if (sum_utility * 1.0 / udist.size() >= 0.01) {
    //         minpecusup = ui->first;
    //         break;
    //    }
    //    
    //}
    cout << "minpecusup:" << minpecusup << endl;
}

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
     * (the first vertex: instrNum=0, weight = -1.)
     * ...
     */
    const char * namefile = namepath.c_str();
	ifstream file;
	string buff;
	file.open(namefile,ios::in);   //open weight file
    int id = atoi(namepath.filename().c_str());
    //cout << "trace" << namefile << " id" << id << endl;
    bool found = false;
    for(int i = 0; i < sDB.size(); i++)
    {
        if (id == sDB[i].id)
        {
            found = true;
            break;
        }
    }
    if (!found)
        cout << "cannot get file id " << namefile << " in simpoint weight data" << endl;
    assert(found);

	while(getline(file, buff))
	{
        vector<string> buffv = split(buff,' ');
        sDB[id].instrNum.push_back(atoi(buffv[0].c_str()));
        sDB[id].vertexType.push_back(atoi(buffv[1].c_str()));
        sDB[id].weight.push_back(atoi(buffv[2].c_str()));
        sDB[id].event.push_back(atoi(buffv[3].c_str()));
	}
    file.close();
}

void read_simpoint_weight_file(const char * namefile)
{
    /* format:
     * weight0 id0 
     * weight1 id1 
     * ...
     */
    ifstream file;
    string buff;
    file.open(namefile,ios::in);   //open weight file
    int i = 0;
    while(getline(file, buff))
    {
     vector<string> buffv = split(buff,' ');
     sDB.push_back(seqdb());
     sDB[i].id = atoi(buffv[1].c_str());
     sDB[i].simpoint_weight = atof(buffv[0].c_str());
     //cout << "simpoint : " << i << ":" << sDB[i].id << " " << sDB[i].simpoint_weight << endl;
     i++;
    }
    file.close();
}

void read_pc_mem_trace(const char * namefile)
{
    /* format:
     * @I pc code
     * @F xx
     * @B xx
     * @I pc code @A addr
     * @F xx
     * @B xx
     * @M xx
     * ...
     * line number is instrNum
     */
    ifstream file;
    vector<string> lines;
    string buff;
    file.open(namefile,ios::in);
    while(getline(file, buff))
    {
	if (buff.rfind("@I", 0) == 0) {
            lines.push_back(buff);
        //cout << "line" << lines.size() << ": "<< buff << endl;
	}
    }
    for (int i = 0; i < sDB.size(); i++)
    {
        for (int j = 0; j < sDB[i].instrNum.size(); j++)
        {
            int itemInstrNum = sDB[i].instrNum[j]; // sDB.instrNum start from 0
            string line = lines[itemInstrNum];
            vector<string> linev = split(line, ' ');
            unsigned int itempc = stoul(linev[1], nullptr, 16);
            string code = linev[2];
            for (int si = 3; si < linev.size(); si++) {
                if (linev[si] == "@A")
                    break;
                code += ' ';
                code += linev[si];
            }
            //cout << "pc:" << itempc << " code:" << code << endl;
            if (pc_code.find(itempc) == pc_code.end()) {
                pc_code[itempc] = code;
            }
            sDB[i].pc.push_back(itempc);
            //cout << itemInstrNum << "-" << line << ":"<<itempc << endl;

            // update minpc, maxpc
            if ((i == 0) && (j == 0))
            {
                minpc = itempc;
                maxpc = itempc;
            }
            else
            {
                if (itempc < minpc)
                    minpc = itempc;
                if (itempc > maxpc)
                    maxpc = itempc;
            }
        }
    }
    if ((maxpc - minpc)<=DELTA)
        DELTA = 1;
    deltapc = (maxpc - minpc)/DELTA;
    if (deltapc < 10) {
    // if the delta pc is too small, regenerate DELTA and deltapc
        DELTA /= 100;
	deltapc = (maxpc - minpc)/DELTA;
    }
    cout << "minpc:"<<minpc <<" maxpc:" << maxpc << " deltapc:" << deltapc << endl;
    file.close();
}

unsigned long GetTickCount()
{
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return (ts.tv_sec * 1000 + ts.tv_nsec / 1000000);
}

bool gen_shortpatt (int level) {
    bool gennew = false;
    for (auto ph: freArr[level]) {
        vector<int> short_patt;
        short_patt.push_back(ph[0]);
        short_patt.push_back(ph[1]);
        if (ph.size()>2) {
            short_patt.push_back(ph[2]);
            for (int phi = 2; phi < ph.size()-1; phi++) {
                if (not (ph[phi] == ph[phi-1] and ph[phi+1] == ph[phi])) {
                    short_patt.push_back(ph[phi+1]);
                }
            }
        }
        bool had = false;
        for (auto sph : simplepatt) {
            if (sph.size() == short_patt.size()) {
                bool found = true;
                for (int si = 0; si < sph.size(); si++) {
                    if (sph[si] != short_patt[si]) {
                        found =false;
                        break;
                    }
                }
                if (found) {
                    had = true;
                    break;
                }
            }
        }
        if (!had) {
            gennew = true;
            simplepatt.push_back(short_patt);
        }
    }
    return gennew;
}

void print_patts(vector <vector<int>> patts) {
    for (auto ffl: patts) {
        for (auto ffli : ffl) {
            cout << ffli;
        }
        cout << "  ";
    }
    cout << endl;
}

/*
bool check_simple(int level)
{
    vector <vector<int>> patt_lowlevel = gen_shortpatt(level-1);
    vector <vector<int>> patt_highlevel = gen_shortpatt(level);
#ifdef PRINT
    cout << "freqpatt of level " << level-1 << endl;
    print_patts(freArr[level-1]);
    cout << "shortpatt of level " << level-1 << endl;
    print_patts(patt_lowlevel);
    cout << "freqpatt of level " << level << endl;
    print_patts(freArr[level]);
    cout << "shortpatt of level " << level << endl;
    print_patts(patt_highlevel);
#endif
    int pattnum = patt_highlevel.size();
    int same = 0;
    if (patt_lowlevel.size() >= patt_highlevel.size()) {
        for (int pi = 0; pi < patt_highlevel.size(); pi++) {
            for (int pj = 0; pj < patt_lowlevel.size(); pj++) {
                if (patt_lowlevel[pj].size() == patt_highlevel[pi].size()) {
                    bool found = true;
                    for (int i = 0; i < patt_lowlevel[pj].size(); i++) {
                        if (patt_lowlevel[pj][i] != patt_highlevel[pi][i]) {
                            found = false;
                            break;
                        }
                    }
                    if (found) {
                        same++;
                        break;
                    }
                }
            }
        }
        if (same == pattnum) {
            return true;
        } else {
            return false;
        }
    } else {
        return false;
    }
}
*/

int main(int argc, const char *argv[])
{   
    assert(argc >= 8);
    // read simpoint weight and build sdb
    read_simpoint_weight_file(argv[1]);
    // read trace in dir
    string path = argv[2];
    cout << "+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++" << endl;
    for (const auto & entry : filesystem::directory_iterator(path))
    {
        cout << "trace:" << entry.path() << endl;
        read_file(entry.path());
    }
#ifdef PRINT    
    cout << "pc trace:" << argv[3] << endl;
    read_pc_mem_trace(argv[3]);
#endif
    ofstream outfile;
    outfile.open(argv[4],ios::out);
    //length constraint
    int minlen=atoi(argv[5]);
    int maxlen=atoi(argv[6]);
    double topk = atof(argv[7]);
    // other params
    double minsup= 0;
    double minu= 0;
    double bound= 0;
    int minpecusup = 0; // minimum peculiarity value support
    if (argc > 8) {
        minsup= atof(argv[8]);
        minu= atof(argv[9]);
        bound= atof(argv[10]);
        minpecusup = atoi(argv[11]); // minimum peculiarity value support
    }

    int frenum = 0;  //frequents number
    freArr = new vector <vector<int>>[maxlen];
    canArr = new vector <vector<int>>[maxlen];
    vector <double> *freArrHUP = new vector <double>[maxlen]; // freq patterns average utility support
    vector <int> *freArrSUP = new vector <int>[maxlen]; // freq patterns support
    vector <double> *freArrPecu = new vector <double>[maxlen]; // freq patterns Peculiarity value number
    int numbS = sDB.size();
	int cannum=0;
    int global_range_pc[DELTA]; 
    vector<vector<int>> global_range_pc_patt;
    for (int gi = 0; gi < DELTA; gi++)
    {
        global_range_pc[gi] = 0;
        global_range_pc_patt.push_back({});
    }
	int f_level=1; // min len of pattern is 2
#if defined(CALIPERS)
    CalipersModel model;
#elif defined(ARCHEXP)
    ArchexpModel model;
#else
    cerr << "unknown model" << endl;
#endif
    candidate = model.edges;
    //for (auto ci : candidate) cout << ci[0] << "," << ci[1] << " "; cout << endl;
    gen_param(minsup, minu, bound, minpecusup, topk);
	DWORD begintime=GetTickCount();
	while(candidate.size()!=0)
	{
		for(int i=0;i<candidate.size();i++)
		{
            vector<int> p = candidate[i];
			cannum++;
			double supd=0; //sdb sup
            int sup=0;
            int ptn_len = p.size();
#ifdef PRINT
            vector <int> occurence[numbS]; // start position of occurence
            int local_range_pc[DELTA]; 
            for (int ri = 0; ri < DELTA; ri++)
            {
                local_range_pc[ri] = 0;
            }
            int pecu = 0; // peculiarity value number of cand
            //int minpecusup = minsup * (ptn_len-1);
            vector<int> allpos;
	    ostringstream os;
#endif            
			for(int t = 0; t < numbS; t++)
			{
				if(sDB[t].vertexType.size() > 0)
				{
					int sups=0; // seq sup
	                for (int si=0;si<=sDB[t].vertexType.size()-ptn_len;si++)
	                {
	                	if(sDB[t].vertexType[si]==p[0])
	                	{
                           bool success = true;
                           for (int pi=1;pi<ptn_len;pi++)
                           {
                               if (sDB[t].vertexType[si+pi]!=p[pi]) {
                                   success = false;
                                   break;
                               }
#ifdef ARCHEXP
			       // ArchExplorer use Virtual(event 16) to connect DEG,
			       // but Virtual has no use in performance.
			       // So get rid of it in frequent patterns.
			       if (sDB[t].event[si+pi] == 16) {
				   success = false;
                                   break;
			       }
#endif
                           }
                           if (success)
                           {
#ifdef PRINT
                               occurence[t].push_back(si);
#endif
                               sup++;
                               int pu = 0;
                               for (int wi = 1; wi < ptn_len; wi++)
                               {
                                   int itemWeight = sDB[t].weight[si+wi];
                                   pu += itemWeight;
#ifdef PRINT
                                   if (itemWeight >= minpecusup)
                                   {
                                       pecu++;
                                   }
#endif
                               }
                               if (pu >= minu*(ptn_len-1)) {
                                    sups += pu;
#ifdef PRINT
                                    int startpc = sDB[t].pc[si]; // start pc
                                    int pos = (startpc-minpc)/deltapc;
                                    if (pos >= DELTA)
                                        pos = DELTA - 1;
                                    local_range_pc[pos]++;
                                    global_range_pc[pos]++;
                                    allpos.push_back(pos);

                                    // dump out trace
                                    for (auto pi:p) {
                                        os << pi;
                                    }
				    os << ' ';
				    // the event type store in end point instruction, so start at si+1
				    for (int ei = si+1; ei < si+ptn_len; ei++) {
					os << model.events[sDB[t].event[ei]];
					if (ei < si+ptn_len-1)
 					    os << "-";
			       	    }
                                    os << ' ' << dec << si << ' ' << hex << "0x" << startpc << ' ' << dec << pu << endl;
                                    for (int pcci = 0; pcci < ptn_len; pcci++) {
                                        int patt_pc = sDB[t].pc[si+pcci];
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
            double hup = supd/(ptn_len-1);
#ifdef PRINT
            int occnum = 0;
            double avgpecu = 0.0;
            for (int oci = 0; oci < numbS; oci++)
            {
                occnum += occurence[oci].size();
            }
            if (occnum > 0)
                avgpecu = (double)pecu/(double)occnum;
#endif
			if(hup>=double(minsup))
			{
				freArr[f_level].push_back(p);
				canArr[f_level].push_back(p);
#ifdef PRINT
		outfile << os.str();
                freArrSUP[f_level].push_back(occnum);
				freArrHUP[f_level].push_back(hup);
                freArrPecu[f_level].push_back(avgpecu);
#endif
                
                if ((f_level >= minlen-1) && (f_level <= maxlen-1))
                {
#ifdef PRINT
                    for (int gi = 0; gi < DELTA; gi++)
                    {
                        global_range_pc_patt[gi].push_back(0);
                    }

                    for (auto posi:allpos) {
                        int old = global_range_pc_patt[posi].back();
                        global_range_pc_patt[posi].pop_back();
                        global_range_pc_patt[posi].push_back(old+1);
                    }

                    // print result of frequent candidate
                    cout << "[" << frenum << "]";
                    for (auto pi:p)
                    {
                        cout << pi;
                    }
                    cout << ":" << occnum << ":" << hup << ":" << avgpecu << "|";
                    for (int ii = 0; ii < DELTA; ii++)
                    {
                        if (local_range_pc[ii]>DELTA)
                            cout << ii << ":" << local_range_pc[ii] << ",";
                    }
                    cout << endl;
#endif
                    frenum++;
                }
			}
            else 
            {
			    if(sup*bound>=double(minsup))
                {
				    canArr[f_level].push_back(p);
#ifdef PRINT
                    cout << "[c]";
                    for (auto pi:p)
                    {
                        cout << pi;
                    }
                    cout << ":" << sup << ":" << hup << ":" << avgpecu << endl;
#endif
                }
            }
            /*
            for (auto voi: occurence)
            {
                for (auto oi: voi)
                {
                    cout << "p" << p << "occ" << oi << endl;
                }
            }
            */
		}
		f_level++;
#ifndef NOCPJM		
        if (not gen_shortpatt(f_level-1)) break;
#endif
        if (f_level >= maxlen) break;
		candidate.clear();
		gen_candidate(f_level);
	}
	DWORD endtime=GetTickCount();
    size_t peak_memu = memu_get_peak_rss();
    size_t curr_memu = memu_get_curr_rss();
    cout << "================== result =====================" << endl;
    int top1i, top1j, top2i, top2j, top3i, top3j;
    double huptop1=0;
    double huptop2=0;
    double huptop3=0;
    int freid = 0;
	for(int i = minlen-1; (i<maxlen) && (i<f_level); i++)
	{
		for(int j = 0; j<freArr[i].size();j++)
		{
            cout << "[" << freid++ << "]";
            for (auto ff: freArr[i][j])
            {
                cout << ff;
            }
#ifdef PRINT
			cout<<":"<<freArrSUP[i][j] << ":"<<freArrHUP[i][j]<<":"<<freArrPecu[i][j]<<"\t";

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
		cout<<endl;
	}

    cout << "concise patts:" << endl;
    for(int j = 0; j<simplepatt.size();j++)
    {
        for (auto ff: simplepatt[j])
        {
            cout << ff;
        }
        cout << "  ";
    }
    cout << endl;
#ifdef PRINT
    if (frenum> 3) {
        cout << "top hup: 1.";
        for (auto t:freArr[top1i][top1j]) {
            cout << t;
        }
        cout << " 2.";
        for (auto t:freArr[top2i][top2j]) {
            cout << t;
        }
        cout << " 3.";
        for (auto t:freArr[top3i][top3j]) {
            cout << t;
        }
        cout << endl;
    }
    cout << "-------------------- top pc region -----------------------------" << endl;
    int max_global_range_pc = global_range_pc[0];
    for (int ii = 0; ii < DELTA; ii++){
        if (global_range_pc[ii]>max_global_range_pc) {
		max_global_range_pc = global_range_pc[ii];
	}
    }
    for (int ii = 0; ii < DELTA; ii++)
    {
        //if (global_range_pc[ii]>DELTA*(maxlen-minlen)) {
        if (global_range_pc[ii]>max_global_range_pc*0.5) {
            cout << ii << ":" << global_range_pc[ii] << endl;
            for (int fi = 0; fi < global_range_pc_patt[ii].size(); fi++) {
            //for (int fi = 0; fi < frenum; fi++) {
                cout << "["<< fi << "]:"<<global_range_pc_patt[ii][fi] << ",";
            }
            cout << endl;
            int endpc = (ii == DELTA-1) ? maxpc+1 : (ii+1)*deltapc + minpc;
            for (int pci = ii * deltapc + minpc; pci < endpc; pci++) {
                if (pc_code.find(pci) != pc_code.end()) {
                    cout << "\t0x" << hex << pci << dec <<":" << pc_code[pci] << endl;
                }
            }
        }
    }
#endif
    cout << endl << "----------------------------------------------------------------" << endl;

    cout<<"minsup="<<minsup<<" minpecusup="<<minpecusup<<" minlen="<<minlen<<" maxlen="<<maxlen << endl;
    cout<<"minu="<<minu<<" bound="<<bound<<endl;
	cout<<"The number of frequent patterns:"<<frenum<<endl;
	cout <<"The time-consuming:"<<endtime-begintime<<"ms. "<<endl;
	cout <<"The number of calculation:"<<cannum<<" "<<endl;
	cout <<"The number of simple patts:"<<simplepatt.size()<<" "<<endl;
	cout <<"Max len of frequent patts:"<<f_level<<" "<<endl;
    cout <<"The peak memory usage:" << (double)peak_memu/1024.0/1024.0/8.0 << "Mb." <<endl;
    cout <<"The curr memory usage:" << (double)curr_memu/1024.0/1024.0/8.0 << "Mb." <<endl;
    cout << endl;
	
	time_t t = time(0); 
	char tmp[32];
	strftime(tmp, sizeof(tmp), "%Y-%m-%d %H:%M:%S",localtime(&t)); 
	ofstream resultfile; 

	resultfile.open("result_file.txt",ios::app); 

	resultfile<<tmp<<"\t"<<"simpoint weight:"<<argv[1]<<"\tcritical path trace:"<<argv[2]<<"\tpc_mem_trace:"<<argv[3]<<"\tminsup:"<<minsup<<"\tminpecusup:"<<minpecusup<<"\t[minlen,maxlen]:"<<"["<<minlen<<","<<maxlen<<"]\n"<<"time-consuming:"<<endtime-begintime<<"ms\t#freq pattern:"<<frenum<<"\t#candidate:"<<cannum<<"\tpeak memory usage:"<<peak_memu<<"B\t\tcurrent memory usage:"<<curr_memu<<"B\n";

	for(int m = 0; m<f_level; m++)
	{
		for(int j = 0; j<freArr[m].size();j++)
		{
            for (auto ff: freArr[m][j])
            {
                resultfile << ff;
            }
#ifdef PRINT
			resultfile<<":"<<freArrSUP[m][j]<<":"<<freArrHUP[m][j]<<":"<<freArrPecu[m][j]<<"\t";
#endif
		}
	}
	resultfile<<endl;
	resultfile.close();

    outfile.close();
	
	return 0;
	
}
