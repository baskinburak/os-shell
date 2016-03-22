#include <iostream>
#include <unordered_map>
#include <string>
#include <vector>
#include <cstdlib>
#include <fstream>
#include <sstream>

using namespace std;
class PROC {
	public:
		string path;
		vector<string> args;
		int in_pipe;
		vector<int> out_pipes;
		PROC() {}
		PROC(string p, vector<string> a, int i, vector<int> o):path(p),args(a),in_pipe(i),out_pipes(o) {

		}
		void run_proc() {
			
		}
};

class PIPE {
	public:
		int id;
		int read_desc;
		int write_desc;
};

unordered_map<int,PIPE> _pipes;
vector<PROC> _procs;
vector<string> _path_env;


void EXIT() {
	exit(0);	
}

bool file_exists(string path) {
	ifstream file(path);
	bool exists = file.good();
	file.close();
	return exists;
}

vector<string> get_path_vec(char* env) {
	string ENV(env);
	string cur;
	vector<string> vars;
	for(int i=0;i<ENV.size();i++) {
		if(ENV[i]==':') {
			cur.push_back('/');
			vars.push_back(cur);
			cur = string();
			continue;
		}
		cur.push_back(ENV[i]);
	}
	cur.push_back('/');
	vars.push_back(cur);
	return vars;
}

string trim(string inp) {
	string res;
	int i,j;
	for(i=0; i<inp.size() && iswspace(inp[i]); i++); 
	for(j=inp.size()-1; j>=0 && iswspace(inp[j]); j--);
	for(int k=i;k<=j;k++) {
		res.push_back(inp[k]);
	}
	return res;
}

PROC parse_proc(string line) {
	stringstream inp(line);

	string path;
	string back_path;
	string token;


	inp >> path;
	back_path = path;

	for(int i=0;i<_path_env.size() && !file_exists(path);i++) {
		path = _path_env[i] + back_path;
	}

	cout << path << endl;
	int st = 0; //0: arg read, 1: <| inp , 2: >| out
	while((inp>>token)) {
		cout << token << endl;
	}
	
	return PROC();
	
}

int main() {
	_path_env = get_path_vec(getenv("PATH"));
	string line;
	while(1) {
		cout << ">";
		getline(cin,line);
		line = trim(line);
		if(line=="quit") {
			EXIT();
		} else if(line=="") {
			continue;
		}
		PROC new_proc = parse_proc(line);
	}
	return 0;
}
