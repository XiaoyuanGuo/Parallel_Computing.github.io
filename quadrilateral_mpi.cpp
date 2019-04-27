#include <mpi.h>
#include <string.h>
#include <iostream>
#include <sstream>
#include <fstream>
#include <ostream>
#include <vector>
#include <map>
#include <typeinfo>
#include <stdio.h>
#include <algorithm> 
#include <set> 

using namespace std;

#define DELIMETER  " "
#define MAXBUFFER 50000

struct edge{
	int a;
	int b;
};

struct node{
	int id;
	vector<int> neighbors;
};

void display(vector<int> P);
vector<string> split(const string& str, const string& delim);
long get_file_size(string filename);
edge parse_line(string line);
int* partition(string filename, int size);
map<int, node> parse_file(string filename);
int intersectionCount(vector<int> a, vector<int> b);
int node_proc_owner(int boundaries[], int size, int node);
int* seriealizeNodes(map<int, node> nodes);
int* seriealizeQuad(set <vector<int> > result);
set <vector<int> >  deseriealizeQuad(int buffer[], int n, set <vector<int> > result);
map<int, node> deseriealizeNodes(int buffer[]);
node recv_incoming_node(MPI_Status &status);
int surrogateCount(map<int, node>nodes, node n);
set <vector<int> > ComputeIntersect(map<int, node> nodes1,map<int, node> nodes2,set <vector<int> > result);
vector<int> intersectionCount2(vector<int> a, vector<int> b);


int main(int argc, char* argv[]){
	// read files
	string filename;
	if(argc == 2)
		filename = argv[1];
	else{
		printf("Wrong input!\n");
		exit(0);
	}

	// initialize 
	int rank, size;
	MPI_Status status;
	MPI_Request request = MPI_REQUEST_NULL;

	MPI_Init(NULL, NULL);
	MPI_Comm_rank(MPI_COMM_WORLD, &rank);
	MPI_Comm_size(MPI_COMM_WORLD, &size);

	// parameters
	int* boundaries = new int[size];
	int buffer_size = 0;
	int* sendNodes_buffer;
	
	set <vector<int> > result;

	// split files into several ones
	if(rank == 0){
		printf("Split the input graph into several subgraphs....\n");
		boundaries = partition(filename,size);
		printf("done\n");
	}
	MPI_Bcast(boundaries, size, MPI_INT, 0 , MPI_COMM_WORLD);
	printf("rank %d has boundaries %d\n",rank, boundaries[rank] );

	//each processor reads its chuck
	map<int, node> nodes = parse_file(to_string(rank)+".txt");

	printf("process %d starts computing %d nodes ...\n",rank, nodes.size());
	int *send_buffer = nullptr;
	
	//compute quad in self 
	result = ComputeIntersect(nodes,nodes,result);
	//wait for everyone finishing computing
	
	printf("rank %d has found %d results \n", rank, result.size());
	MPI_Barrier(MPI_COMM_WORLD);
	// if(rank == 0){
	// 	sendNodes_buffer  = seriealizeNodes(nodes);
	// 	buffer_size = (int)(sendNodes_buffer [0] + 1);		
	// }
	// MPI_Bcast(sendNodes_buffer, 5000, MPI_INT, 0, MPI_COMM_WORLD);
	// printf("rank %d sendNodes_buffer size is %d\n",rank, sendNodes_buffer[0]+1 );

	for(int t = 0; t < size-1; t++){
		
		if(rank == t){

			printf("in rank %d == %d\n",rank, t);
			sendNodes_buffer  = seriealizeNodes(nodes);
			buffer_size = sendNodes_buffer[0]+1;
			printf("sendNodes_buffer %d is ready\n",buffer_size);
			for(int des = t+1; des < size; des++){
				printf("rank %d broadcast %d data done!\n",rank, buffer_size );
				MPI_Send(&buffer_size, 1, MPI_INT, des, 0, MPI_COMM_WORLD);
				MPI_Send(sendNodes_buffer, buffer_size, MPI_INT, des, 2, MPI_COMM_WORLD);
			}
		}
		printf("out rank == t\n");
		
		MPI_Barrier(MPI_COMM_WORLD);

		if(rank > t){

			MPI_Recv(&buffer_size, 1, MPI_INT, t, 0, MPI_COMM_WORLD, &status);
			sendNodes_buffer = (int*)malloc(sizeof(int)*buffer_size);
			MPI_Recv(sendNodes_buffer,buffer_size, MPI_INT, t, 2, MPI_COMM_WORLD, &status);

			printf("rank %d sendNodes_buffer size is %d\n",rank, sendNodes_buffer[0]+1 );
			map<int, node> nodes2 =  deseriealizeNodes(sendNodes_buffer);
			printf("rank %d uses broadcast data to compute!\n",rank);
			result = ComputeIntersect(nodes,nodes2,result);
			printf("rank %d uses broadcast data to get %d results!\n",rank, result.size());
			// nodes2.clear();
			free(sendNodes_buffer);
			buffer_size = 0;
			
		}
		printf("I stop here2\n");
		MPI_Barrier(MPI_COMM_WORLD);

	}//end for

	MPI_Barrier(MPI_COMM_WORLD);
	
	printf("Send results to 0...\n");
    if(rank != 0){
    	if(request != MPI_REQUEST_NULL){
			MPI_Wait(&request, &status);
			free(send_buffer);
		}

    	send_buffer = seriealizeQuad(result);
		int buffer_size = (int)(result.size()*4);
		//Begins a nonblocking send
		MPI_Isend(send_buffer,buffer_size,MPI_INT,0, 1, MPI_COMM_WORLD,&request);
	}

	if(rank == 0){
		int flag = 1;
		while(flag){
			//Nonblocking test for a message			
			MPI_Iprobe(MPI_ANY_SOURCE, 1, MPI_COMM_WORLD, &flag, &status);
			// printf("before if flag = %d\n", flag);
			if(flag){
				// printf("in if flag = %d\n", flag);
				int count = 0;
				//number of received elements (integer)  
				MPI_Get_count(&status, MPI_INT, &count);
				int * buffer = new int[count];
				MPI_Recv(buffer, count, MPI_INT, MPI_ANY_SOURCE, 1, MPI_COMM_WORLD, &status);
				// printf("before rank %d has result %d \n", rank,result.size() );
				result = deseriealizeQuad(buffer, count, result);
				// printf("after rank %d has result %d \n",rank, result.size() );
				free(buffer);
			}
		}

	}

	MPI_Barrier(MPI_COMM_WORLD);


// printf("begin to print results ...\n");

if(rank == 0){

    for(auto &it : result){
		cout<<"[ ";
		for(int i = 0; i < it.size(); i++){
			cout<< it[i]<<" ";
		}
		cout<<"]\n";
	}
	printf("global_sum: %d\n", result.size());
}

	

	MPI_Finalize();
	return 0;
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

long get_file_size(string filename){
	ifstream file(filename, ios::binary | ios::ate);
	long size = file.tellg();
	file.close();
	return size;
}

edge parse_line(string line){
	edge e;
	vector<string> sep = split(line, DELIMETER);
	e.a = stoi(sep[0]);
	e.b = stoi(sep[1]);
	return e;
}

int* partition(string filename, int size){
	string line;
	ifstream file(filename);
	long file_size = get_file_size(filename);
	long chunk_size = file_size/size;
	long split_boundary = chunk_size;
	bool needsplit = false;
	int* boundaries = new int [size];
	edge last, now;
	ofstream temp_file;
	int current = 0; 
	temp_file.open(to_string(current)+".txt");
	printf("\t-splitting chunk %d ...\n",current );
	if(file.is_open()){
		while(getline(file, line)){
			if(line.at(0) == '#')
				continue;
			now = parse_line(line);
			if(needsplit){
				if(last.a != now.a){
					temp_file.close();
					needsplit = false;
					boundaries[current++] = last.a;
					temp_file.open(to_string(current)+".txt");
					printf("\t-splitting chunk %d ...\n",current );
				}
			}else{
				if(file.tellg() > split_boundary){
					last = parse_line(line);
					needsplit = true;
					split_boundary += chunk_size;
				}
			}
			temp_file<<line<<endl;
		}
		file.close();
		temp_file.close();
		boundaries[current] = now.a;
	}else{
		printf("Cannot open file\n");
	}
	return boundaries;

}


map<int, node> parse_file(string filename){
	string line;
	ifstream file(filename);
	map<int, node> nodes;
	file.seekg(0, file.beg);//Sets the position of the next character to be extracted from the input stream.
	if(file.is_open()){
		while(getline(file, line) && line.at(0) != '#'){
			if(line.at(0) != '#'){
				edge e = parse_line(line);
				nodes[e.a].id = e.a;
				nodes[e.a].neighbors.push_back(e.b);
			}
		}
		file.close();
		//remove(filename.c_str());
	}else{
		printf("Unable to open file\n");
	}
	//cout<<"I am here!\n"<<endl;
	return nodes;

}


int intersectionCount(vector<int> a, vector<int> b){
	int sum = 0;
	for(int i = 0; i < a.size(); ++i){
		for(int j = 0; j < b.size(); ++j){
			if(a[i] == b[j])
				sum++;
		}
	}
}




int node_proc_owner(int boundaries[], int size, int node){
	for(int i = 0; i < size; i++){
		if(node <= boundaries[i])
			return i;
	}
	return 0;
}

int* seriealizeNodes(map<int, node> nodes){
	// stop sign = -1
	vector<int> vtemp;

	for (auto n = nodes.begin(); n != nodes.end(); n++){
		vtemp.push_back(n->second.id);
		for(int i = 0; i < n->second.neighbors.size(); i++)
			vtemp.push_back(n->second.neighbors[i]);
		vtemp.push_back(-1);//to indicate this node ends
	}		

	int *s = new int[vtemp.size() + 1];
	s[0] = vtemp.size();
	for(int i = 0; i < vtemp.size();++i){
		s[i+1] = vtemp[i];
	}
	return s;
}

int* seriealizeQuad(set <vector<int> > result){
	int *s = new int[result.size() * 4];
	int t = 0;

	for(auto &it : result){
		for(int i = 0; i < it.size(); i++){
			s[t++] = it[i];
		}
	}
	return s;
}

map<int, node> deseriealizeNodes(int buffer[]){
	// buffer[0] indicates how big the buffer is, skip
	int size = buffer[0]+1;
	map<int, node> nodes;
	int i, j;
	for( i = 1; i < size; i++){
		if(i == 1 || buffer[i-1] == -1){
			nodes[buffer[i]].id = buffer[i];
			for(j = i+1; buffer[j] != -1 && j < size; j++)
				nodes[buffer[i]].neighbors.push_back(buffer[j]);
			i = j;
		}

	}

	return nodes;

}


set <vector<int> >  deseriealizeQuad(int buffer[], int n, set <vector<int> > result){
	vector<int> qtemp;
	for (int i = 0; i < n; i+=4){
		qtemp.push_back(buffer[i]);
		qtemp.push_back(buffer[i+1]);
		qtemp.push_back(buffer[i+2]);
		qtemp.push_back(buffer[i+3]);
		result.insert(qtemp);
		qtemp.clear();
	}
	return result;

}


int surrogateCount(map<int, node>nodes, node n){
	int sum = 0;
	for(int i = 0; i <n.neighbors.size(); ++i){
		int id = n.neighbors[i];
		if(nodes.find(id) != nodes.end()){
			sum += intersectionCount(nodes[id].neighbors, n.neighbors);
		}
	}
	return sum;

}

vector<int> intersectionCount2(vector<int> a, vector<int> b){
	// this for quadrilateral counting situation
	// e.g. (1 2 3 4) vs (2 3 4 5)->(2 3 4)->6 
	//int sum = 0;
	vector<int> v;
	int count = 0;
	for(int i = 0; i < a.size(); i++){
		for(int j = 0; j< b.size(); j++){
			if(a[i] == b[j]){
				//cout<<a[i]<<" ";
				v.push_back(a[i]);
				count++;
			}
		}
	}
	
	return v;

}

void display(vector<int> P){
	for(int i = 0; i < P.size(); i++)
		printf("%d ", P[i]);
	printf("\n");
}


set <vector<int> > ComputeIntersect(map<int, node> nodes1,map<int, node> nodes2,set <vector<int> > result){
	for (auto n = nodes1.begin(); n != nodes1.end(); n++){
		for (auto m = nodes2.begin(); m!= nodes2.end(); m++){
			if(n->second.id != m->second.id){
				// printf("<%d %d>\n", n->second.id, m->second.id);
				vector<int> temp = intersectionCount2(n->second.neighbors,m->second.neighbors);
				if (temp.size() <= 1){
					temp.clear();
					continue;
				}
				vector<int> qtemp;
				for(int i = 0; i < temp.size()-1; i++ ){
				  	for(int j = i+1; j < temp.size(); j++ ){
				 		qtemp.push_back(n->second.id);
						qtemp.push_back(m->second.id);
				 		qtemp.push_back(temp[i]);
				 		qtemp.push_back(temp[j]);
				 		sort(qtemp.begin(), qtemp.end());
				 		result.insert(qtemp);
				 		qtemp.clear();
				  	}

				}
			}
		}
       
    }
    return result;

}


