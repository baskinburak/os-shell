#include <iostream>
#include <unordered_map>
#include <string>
#include <vector>
#include <cstdlib>
#include <fstream>
#include <sstream>
#include <algorithm>
#include <unistd.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <cstring>

using namespace std;

enum parse_state {ARG, INP_PIPE, OUTP_PIPE};

class PROC {
	public:
		string path;
		vector<string> args;
		int inp_pipe;
		vector<int> outp_pipes;
		PROC() {}
		PROC(string p, vector<string> a, int i, vector<int> o):path(p), args(a), inp_pipe(i), outp_pipes(o) {
		}
};

class PIPE {
	public:
		int id;
		int read_desc;
		int write_desc;
		bool been_inp;
		bool been_outp;
		PIPE(int i, int r, int w):id(i), read_desc(r), write_desc(w), been_inp(false), been_outp(false) {}
		PIPE() {}
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

string after_last_slash(string inp) {
	string res;
	for(int i=inp.size()-1;i>=0;i--) {
		if(inp[i] == '/') break;
		res.push_back(inp[i]);
	}
	reverse(res.begin(),res.end());
	return res;
}

PROC parse_proc(string line) {
	stringstream inp(line);

	string path;
	string back_path;
	string token;
	vector<string> args;
	int inp_pipe = -1;
	vector<int> outp_pipes;

	inp >> path;
	back_path = path;

	for(int i=0;i<_path_env.size() && !file_exists(path);i++) {
		path = _path_env[i] + back_path;
	}

	args.push_back(after_last_slash(path));

	parse_state st = ARG; //0: arg read, 1: <| inp , 2: >| out
	while((inp >> token)) {
		if(st == ARG) {
			if(token == ">|") {
				st = OUTP_PIPE;
			} else if(token == "<|") {
				st = INP_PIPE;
			} else {
				args.push_back(token);
			}
		} else if(st == INP_PIPE) {
			if(token==">|") {
				st = OUTP_PIPE;
			} else {
				inp_pipe = atoi(token.c_str());
			}
		} else if(st == OUTP_PIPE) {
			if(token=="<|") {
				st = INP_PIPE;
			} else {
				outp_pipes.push_back(atoi(token.c_str()));
			}
		}
	}
	return PROC(path, args, inp_pipe, outp_pipes);
}
void debug_print_procs() {
	for(int i=0; i<_procs.size(); i++) {
		PROC& ref = _procs[i];
		cout << "Path: " << ref.path << endl;
		cout << "Args: ";
		for(int j=0;j<ref.args.size();j++) {
			cout << ref.args[j] << " ";
		}
		cout << endl;
		cout << "Input pipe: " << ref.inp_pipe << endl;
		cout << "Output pipes: ";
		for(int j=0;j<ref.outp_pipes.size();j++) {
			cout << ref.outp_pipes[j] << " ";
		}
		cout << endl << endl;
	}
}

void add_new_pipes(PROC& ref) {
	if(ref.inp_pipe != -1) {
		int pipe_id = ref.inp_pipe;
		if(_pipes.find(pipe_id) == _pipes.end()) {
			int fd[2];
			pipe(fd);
			_pipes[pipe_id] = PIPE(pipe_id, fd[0], fd[1]);
		}
		_pipes[pipe_id].been_inp = true;
	}

	for(int i=0; i<ref.outp_pipes.size(); i++) {
		int pipe_id = ref.outp_pipes[i];
		if(_pipes.find(pipe_id) == _pipes.end()) {
			int fd[2];
			pipe(fd);
			_pipes[pipe_id] = PIPE(pipe_id, fd[0], fd[1]);
		}
		_pipes[pipe_id].been_outp = true;
	}
}
bool pipes_connected() {
	bool ret = true;
	for(unordered_map<int,PIPE>::iterator it = _pipes.begin(); it != _pipes.end(); it++) {
		ret = ret && (it->second).been_inp && (it->second).been_outp;
	}
	return ret;
}

void EXEC(PROC& ref) {
	char** args = (char**) malloc(sizeof(char*)*(ref.args.size()+1));
	args[ref.args.size()] = NULL;
	for(int i=0;i<ref.args.size();i++) {
		args[i] = (char*)malloc(sizeof(char)*(ref.args[i].size()+1));
		strcpy(args[i],ref.args[i].c_str());
	}
	char* path = (char*)malloc(sizeof(char)*(ref.path.size()+1));
	strcpy(path, ref.path.c_str());
	execv(path, args);
}

void run_procs() {
	for(int i=0;i<_procs.size();i++) {
		if(fork() == 0) {
			//child process.this will turn into a new process.
			PROC& ref = _procs[i];

			//close unneeded input pipes
			for(unordered_map<int,PIPE>::iterator it=_pipes.begin(); it != _pipes.end(); it++) {
				if(it->first != ref.inp_pipe) {
					close((it->second).read_desc);
				}

				if(find(ref.outp_pipes.begin(), ref.outp_pipes.end(), it->first) == ref.outp_pipes.end()) {
					close((it->second).write_desc);
				}
			}

			if(ref.inp_pipe != -1) {
				dup2(_pipes[ref.inp_pipe].read_desc,0);
				close(_pipes[ref.inp_pipe].read_desc);
			}

			if(ref.outp_pipes.size() > 1) {
				//repeater case
				int repeater_pipe[2];
				pipe(repeater_pipe);
				if(fork() == 0) {
					//make this repeater
					close(repeater_pipe[1]);
					char buf[512];
					int read_count;
					while((read_count = read(repeater_pipe[0], buf, 512)) > 0) {
						for(int j=0; j < ref.outp_pipes.size(); j++) {
							write(_pipes[ref.outp_pipes[j]].write_desc, buf, read_count);
						}
					}
					for(int j=0; j < ref.outp_pipes.size(); j++) {
						close(_pipes[ref.outp_pipes[j]].write_desc);
					}
					EXIT();
				} else {
					//make this execve
					close(repeater_pipe[0]);
					dup2(repeater_pipe[1],1);
					close(repeater_pipe[1]);
				}
			} else if(ref.outp_pipes.size() == 1) {
				dup2(_pipes[ref.outp_pipes[0]].write_desc, 1);
				close(_pipes[ref.outp_pipes[0]].write_desc);
			}
			EXEC(ref);
		}
	}
	for(unordered_map<int,PIPE>::iterator it=_pipes.begin(); it != _pipes.end(); it++) {
		close((it->second).read_desc);
		close((it->second).write_desc);
	}
	while(waitpid(-1, NULL, 0)) {
		if(errno == ECHILD) {
			break;
		}
	}
	_procs.clear();
	_pipes.clear();
}

int main() {
	_path_env = get_path_vec(getenv("PATH"));
	string line;
	while(1) {
		getline(cin,line);
		line = trim(line);
		if(line=="quit") {
			EXIT();
		} else if(line=="") {
			continue;
		}
		PROC new_proc = parse_proc(line);
		_procs.push_back(new_proc);
		add_new_pipes(new_proc);
		if(pipes_connected()) {
			run_procs();
		}
	}
	return 0;
}
