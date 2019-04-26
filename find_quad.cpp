# This is a simple version to find quadrilaterals in input files.
# It's written by Xiaoyuan Guo.

#include <map>
#include <string>
#include <fstream>
#include <sstream>
#include <vector>
#include <ostream>
#include <stdio.h>
#include <iostream>
#include <algorithm> 
#include <set> 
#include <stdlib.h>

using namespace std;

#define DELIMETER  " "

struct edge{
	int a;
	int b;
};

struct node{
	int id;
	vector<int> neighbors;
};

vector<string> split(const string& str, const string& delim);
edge parse_line(string line);
map<int, node> parse_file(string filename);
vector<int> intersectionCount2(vector<int> a, vector<int> b);


int main(int argc, char** argv){
	string filename; 
	cout<<argc<<endl;
	for(int i = 0; i < argc; i++)
		cout<<argv[i]<<endl;
	filename = argv[1];
	cout<<filename<<endl;
	int quad_sum = 0;
	set <vector<int> > result;
	map<int, node> nodes = parse_file(filename);
	
	for (auto n = nodes.begin(); n != nodes.end(); n++){
	 	cout<<"node "<<n->second.id<<endl<<"[";
	 	for(int i = 0; i < n->second.neighbors.size(); i++)
	 		cout<<n->second.neighbors[i]<<" ";
	 		cout<<"]"<<endl;
	}


	for (auto n = nodes.begin(); n != nodes.end(); n++){
		for (auto m = n; m!= nodes.end(); m++){
			if(m!=n){
				vector<int> temp = intersectionCount2(n->second.neighbors,m->second.neighbors);
				vector<int> qtemp;
				for(int i = 0; i < temp.size()-1; i++ ){
				 	for(int j = i+1; j < temp.size(); j++ ){
				 		qtemp.push_back(n->second.id);
						qtemp.push_back(m->second.id);
				 		qtemp.push_back(temp[i]);
				 		qtemp.push_back(temp[j]);
				 		sort(qtemp.begin(), qtemp.begin()+4);
				 		result.insert(qtemp);
				 		qtemp.clear();
				 		
				 	}

				 }
			}
		}
       
    }


    // cout<<"*************************************\n";

	for(auto &it : result){
		cout<<"[ ";
		for(int i = 0; i < it.size(); i++){
			cout<< it[i]<<" ";
		}
		cout<<"]\n";
	}

   	cout<<"quad_sum: "<<result.size()<<endl;

}


map<int, node> parse_file(string filename){
	string line;
	ifstream file(filename);
	map<int, node> nodes;
	file.seekg(0, file.beg);//Sets the position of the next character to be extracted from the input stream.
	if(file.is_open()){
		while(getline(file, line)){			
			if(line.at(0) == '#')
				continue;
			else{
				edge e = parse_line(line);
				nodes[e.a].id = e.a;
				nodes[e.a].neighbors.push_back(e.b);
				nodes[e.b].id = e.b;
				nodes[e.b].neighbors.push_back(e.a);
			}
		}
		file.close();
		
	}else{
		printf("Unable to open file\n");
	}

	return nodes;

}

vector<string> split(const string& str, const string& delim)
{
    vector<string> tokens;
    size_t prev = 0, pos = 0;

    do
    {
        pos = str.find(delim, prev);
        if (pos == string::npos) pos = str.length();
        string token = str.substr(prev, pos-prev);
        if (!token.empty()) tokens.push_back(token);
        prev = pos + delim.length();
    }
    while (pos < str.length() && prev < str.length());

    return tokens;
}


edge parse_line(string line){
	edge e;
	vector<string> sep = split(line, DELIMETER);
	e.a = stoi(sep[0]);
	e.b = stoi(sep[1]);
	return e;
}


vector<int> intersectionCount2(vector<int> a, vector<int> b){
	// this for quadrilateral counting situation
	// e.g. (1 2 3 4) vs (2 3 4 5)->(2 3 4)->6 
	//int sum = 0;
	vector<int> v;
	//cout<<"\n************************************\n";
	for(int i = 0; i < a.size(); i++){
		for(int j = 0; j< b.size(); j++){
			if(a[i] == b[j]){
				//cout<<a[i]<<" ";
				v.push_back(a[i]);
			}
		}
	}
	//cout<<"\n************************************\n";

	return v;

}



