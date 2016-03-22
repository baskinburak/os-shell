#include <iostream>
#include <unordered_map>
#include <string>
#include <vector>
#include <cstdlib>
#include <fstream>

using namespace std;
class PROC {
	public:
		string path;
		vector<string> args;
		int in_pipe;
		vector<int> out_pipes;
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
	vars.push_back(cur);
	return vars;
}

int main() {
	vector<string> path_env = get_path_vec(getenv("PATH"));

	while(1) {
		cout << ">";
		PROC new_proc;
		int a;
		cin >> a;
	}
	return 0;
}
