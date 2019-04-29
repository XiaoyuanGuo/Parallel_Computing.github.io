// This file is for sort input edge pairs, it works very well for quadrilateral case, not sure for triangle
// usage: g++ sort_edgepairs.cpp -w -o sort
// ./sort input.txt

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>
#include <fstream>
#include <iterator>
#include <string.h>
#include <vector>
#include <iostream>
#include <iterator>
#include <algorithm>

using namespace std;

#define max  2312497

bool sortinrev(const pair<int,int> &a,  const pair<int,int> &b) { 
       return (a.first < b.first || (a.second < b.second && a.first == b.first)); 
} 
  

int main(int argc, char* argv[]){
	char const* const fileName = argv[1];
	FILE* file = fopen(fileName, "r");
	char line[256];
	
	char* pch;
	vector< pair <int,int> > vect;
	int i = 0;
	while(fgets(line, sizeof(line), file)){
		pch = strtok(line," ");
		int a = atoi(pch);
		pch = strtok(NULL," ");
		int b = atoi(pch);
		vect.push_back( make_pair(a,b) );  
		
	}

	sort(vect.begin(), vect.end(),sortinrev);
	ofstream output_file("sorted2.txt");
	for (int i=0; i<vect.size(); i++) 
    { 
        output_file << vect[i].first << " "
             << vect[i].second << endl; 
    } 
 
	fclose(file);
	cout<<"file has"<<vect.size()<<" edges\n";	

	
	return 0;
}
