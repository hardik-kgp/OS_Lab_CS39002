#include <bits/stdc++.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
using namespace std;

#define SHELL_TOK_DELIM " \t\r\n\a"
#define SHELL_PIPE_DELIM "|"
#define STDIN_FILENO 0
#define STDOUT_FILENO 1

class Command
{
    public:
        char* cmd;
        vector <char*> args;
        string fin;
        string fout;
        int bkg;
        Command(char*);
        void parse();
        void io_redirect();
};

Command::Command(char *cmd){
    cmd=cmd;
    fin="";
    fout="";
    bkg=0;
}

void Command::parse(){
    vector <string> tokens;
    char *token = strtok(cmd, SHELL_TOK_DELIM);
    while(token != NULL){
        tokens.emplace_back(token);
        token = strtok(NULL, SHELL_TOK_DELIM);
    }
    for(int i=0; i<tokens.size(); i++){
        if(tokens[i]=="&"){
            bkg=1;
        }
        else if(tokens[i]=="<"){
            fin=tokens[i+1]; i++;
        } 
        else if(tokens[i]==">"){
            fout=tokens[i+1]; i++;
        }
        else if(token[0]!='\0'){
            args.emplace_back(token);
        }
    }
    return;
}

void Command::io_redirect(){
    if(fin != ""){
        dup2(STDIN_FILENO, open(&fin[0], O_WRONLY));
    }
    if(fout != ""){
        dup2(STDOUT_FILENO, open(&fout[0], O_WRONLY));
    }
}

int main(int argc, char **argv){
    shell_loop();
    return EXIT_SUCCESS;
}

void shell_loop(){
    int status = 1;
    string line;
    do
    {
        printf(">> ");
        getline(cin, line);
        vector <Command> cmds = split_line(line);
        /*Code to run the commands*/
    } while (status);
}

vector <Command> split_line(string line){
    char *token = strtok(&line[0], SHELL_PIPE_DELIM);
    vector <Command> cmds;
    while(token != NULL){
        token = strtok(NULL, SHELL_PIPE_DELIM);
        Command temp = Command(token); temp.parse();
        cmds.emplace_back(temp);
    }
    return cmds;
}