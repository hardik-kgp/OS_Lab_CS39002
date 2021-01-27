#include <bits/stdc++.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
using namespace std;

#define SHELL_TOK_DELIM ' '
#define SHELL_PIPE_DELIM '|'
#define STDIN_FILENO 0
#define STDOUT_FILENO 1

class Command
{
    public:
        string cmd;
        vector <string> args;
        string fin;
        string fout;
        int bkg;
        Command(string);
        void parse();
        void io_redirect();
        void execute();
};

Command::Command(string cmd){
    this->cmd=cmd;
    fin="";
    fout="";
    bkg=0;
}

vector<string> mystrtok(string s, char c){
    vector<string> tokens;
    stringstream ss(s);
    string temp;
    while (getline(ss, temp, c))
    {
        tokens.push_back(temp);
    }
    return tokens;
    
}




void Command::parse(){
    
    vector<string> tokens = mystrtok(cmd, SHELL_TOK_DELIM);
    
    for(int i = 0;i<tokens.size();i++){
        string token = tokens[i];
        if(token != ""){
            if(token == "&"){
                bkg = 1;
            }   
            else if(token == "<"){
                if(i + 1 == tokens.size()){
                    cout << "Input Not specified!\n";
                    exit(0);
                }
                fin = tokens[++i];
            }
            else if(token == ">"){
                if(i + 1 == tokens.size()){
                    cout << "Output Not specified!\n";
                    exit(0);
                }
                fout = tokens[++i];
            }
            else{
                args.push_back(token);
            }
        }
    }
    return;
}

void Command::io_redirect(){  
    if(fin != ""){
        int fd = open(fin.c_str(), O_RDONLY);
        if(fd < 0){
            cout << "FILE CANNOT BE OPENED. ERROR !" << endl;
            exit(0);
        }
        
        int status = dup2(fd, STDIN_FILENO);
        if(status < 0){
            cout << "Input redirection failed !" << endl;
            exit(0);
        }
    }
    if(fout != ""){
        int fd = open(fout.c_str(),  O_CREAT|O_WRONLY|O_TRUNC, S_IRWXU);
        if(fd < 0){
            cout << "FILE CANNOT BE OPENED. ERROR !" << endl;
            exit(0);
        }
        
        int status = dup2(fd, STDOUT_FILENO);
        if(status < 0){
            cout << "OUTPUT redirection failed" << endl;
            exit(0);
        }
    }
}

void Command::execute(){
    char* argv[args.size() + 1];
    int i = 0;
    for(string arg: args){
        
        argv[i++] = (char *)(arg.c_str());
    }
    argv[i++] = NULL;
    argv[0] = (char *)(args[0].c_str());
    execvp(argv[0], argv);
}

vector <Command> split_line(string line){
    
    vector<string> tokens = mystrtok(line, SHELL_PIPE_DELIM);
    
    vector <Command> cmds;
    for(auto token : tokens){
        
        Command cmd = Command(token);
        cmd.parse();
        cmds.push_back(cmd);
    }
    return cmds;
}

void shell_loop(){
    int status = 1;
    string line;
    do
    {
        printf(">> ");
        getline(cin, line);
        vector <Command> cmds = split_line(line);
        if(cmds.size() == 1){
            pid_t pid = fork();
            if(pid == 0){
                cmds[0].io_redirect();
                cmds[0].execute();
                exit(0);
            }
            else{
                if(cmds[0].bkg == 0){
                    wait(&status);
                }
                else{
                    cout << "[BG] " << pid << endl;
                }
            }
            
        }
        else{
            int no_of_cmds = cmds.size();
            int newFD[2], FD[2];
            for(int i = 0;i<no_of_cmds;i++){
                if(i + 1< no_of_cmds) pipe(newFD);
                pid_t pid = fork();
                if(pid == 0){
                    if(i == 0 || i + 1 == no_of_cmds){
                        cmds[i].io_redirect();
                    }
                    if(i){
                        dup2(FD[0], 0);
                        close(FD[0]);
                        close(FD[1]);
                    }
                    if(i + 1 < no_of_cmds){
                        close(newFD[0]);
                        dup2(newFD[1], 1);
                        close(newFD[1]);
                    }

                    cmds[i].execute();
                }
                else{
                    if(cmds[i].bkg != 0){
                        wait(NULL);
                    }
                }
                
                if(i != 0){
                    close(FD[0]);
                    close(FD[1]);
                }
                if(i + 1 != no_of_cmds){
                    FD[0] = newFD[0], FD[1] = newFD[1];
                }
            }

            if(cmds.back().bkg == 0){
                
                while(wait(&status) > 0);
            }
        }
       
    } while (true);
}

int main(int argc, char **argv){
    shell_loop();
    
    return EXIT_SUCCESS;
}



