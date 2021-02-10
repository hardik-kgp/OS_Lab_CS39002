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

vector<string> mystrtok(string s, char c){
    /*Function to tokenize string s using delimiter c*/
    vector<string> tokens;
    stringstream ss(s);
    string temp;
    while (getline(ss, temp, c))
    {
        tokens.push_back(temp);
    }
    return tokens;
}

vector <string> built_ins {"cd", "exit", "help"};

int shell_cd(vector <string> args){
    /*Function to change directory*/
    if(chdir(args[1].c_str()) !=0){
        perror("Invalid path");
    }
    return EXIT_SUCCESS;
}

int shell_help(vector <string> args){
    /*Function to print shell help*/
    printf("Shell built-in commands: \n");
    for(auto built_in: built_ins) printf("%s\n", built_in.c_str());
    return EXIT_SUCCESS;
}

int shell_exit(vector <string> args){
    /*Function to exit shell*/
    exit(EXIT_SUCCESS);
    return EXIT_SUCCESS;
}

int (*built_in_func[]) (vector <string>) = {&shell_cd, &shell_exit, &shell_help};
class Command{
    /*Class to store data about the command*/
    public:
        string cmd;             // stores the command string
        vector<string> args;    // stores the args to execvp
        string fin;             // stores name of input file, if any
        string fout;            // stores name of output file, if any
        int bkg;                // flag to denote, if to run a process in background or not
        int built_in;
        Command(string);
        void parse();
        void io_redirect();
        void execute();
};

Command::Command(string cmd){
    /*Class Constructor*/
    this->cmd = cmd;
    fin = "";
    fout = "";
    bkg = 0;
    built_in = -1;
}

void Command::parse(){
    /*Function to parse command string*/
    vector<string> tokens = mystrtok(cmd, SHELL_TOK_DELIM);

    for (int i = 0; i < tokens.size(); i++){
        string token = tokens[i];
        if (token != ""){// Ignoring empty tokens
            if (token == "&"){// & denotes process to run in background
                bkg = 1;
            }
            else if (token == "<"){// input redirection
                if (i + 1 == tokens.size()){
                    cout << "Input file not specified!\n";
                    exit(EXIT_FAILURE);
                }
                fin = tokens[++i];
            }
            else if (token == ">"){// output redirection
                if (i + 1 == tokens.size()){
                    cout << "Output Not specified!\n";
                    exit(EXIT_FAILURE);
                }
                fout = tokens[++i];
            }
            else{// args
                args.push_back(token);
            }
        }
    }
    for(int i=0; i<built_ins.size(); i++){
        if(strcmp(args[0].c_str(), built_ins[i].c_str()) == 0)
            built_in = i;
    }
    return;
}

void Command::io_redirect(){
    /*Function to redirect input and output*/
    if (fin != ""){
        int fd = open(fin.c_str(), O_RDONLY); // open file in read only
        if (fd < 0){
            cout << "Cannot open file!" << endl;
            exit(EXIT_FAILURE);
        }
        int status = dup2(fd, STDIN_FILENO); // redirect from stdin to fin
        if (status < 0){
            cout << "Input redirection failed !" << endl;
            exit(EXIT_FAILURE);
        }
    }

    if (fout != ""){
        int fd = open(fout.c_str(), O_CREAT | O_WRONLY | O_TRUNC, S_IRWXU); // open file in write only
        if (fd < 0){
            cout << "Cannot open file!" << endl;
            exit(0);
        }
        int status = dup2(fd, STDOUT_FILENO); // redirect from stdout to fout
        if (status < 0){
            cout << "Output redirection failed" << endl;
            exit(0);
        }
    }
}

void Command::execute(){
    /*Function to execute command*/
    char *argv[args.size() + 1];
    int i = 0;
    for (string arg : args){
        /*convert strings to char* */
        argv[i++] = (char *)(arg.c_str());
    }
    argv[i++] = NULL;
    argv[0] = (char *)(args[0].c_str());
    execvp(argv[0], argv);
}

vector<Command> split_line(string line){
    /*Function to split piped command to simple commands*/
    vector<string> tokens = mystrtok(line, SHELL_PIPE_DELIM);
    vector<Command> cmds;
    for (auto token : tokens){
        Command cmd = Command(token);
        cmd.parse();
        cmds.push_back(cmd);
    }
    return cmds;
}

void shell_loop(){
    /*main loop*/
    int status = 1;
    string line;
    do{
        printf(">> ");  //shell prompt
        getline(cin, line);
        vector<Command> cmds = split_line(line);
        if(cmds.size() == 0){
            continue;
        }
        if (cmds.size() == 1){ // single command
            /*check for builtins*/
            if(cmds[0].built_in != -1){
                (*built_in_func[cmds[0].built_in])(cmds[0].args);
            }
            else{
                pid_t pid = fork(); // child process
                if (pid == 0){
                    cmds[0].io_redirect(); // execute command
                    cmds[0].execute();
                    exit(EXIT_FAILURE); // exit child process incase of failure
                }
                else{
                    if (cmds[0].bkg == 0){
                        wait(&status); // wait if child not a background process
                    }
                    else{
                        cout << "[BG] " << pid << endl;
                    }
                }   
            }
        }
        else {
            int num_cmds = cmds.size();
            int newFD[2], FD[2];
            for (int i = 0; i < num_cmds; i++){ // iterating through commands
                if (i + 1 < num_cmds)
                    pipe(newFD); // creating pipe
                pid_t pid = fork(); //creating child process
                if (pid == 0){
                    if (i == 0 || i + 1 == num_cmds){ // redirect to i/o
                        cmds[i].io_redirect();
                    }
                    if (i){// input piping
                        dup2(FD[0], 0);
                        close(FD[0]);
                        close(FD[1]);
                    }
                    if (i + 1 < num_cmds){ // output piping
                        close(newFD[0]);
                        dup2(newFD[1], 1);
                        close(newFD[1]);
                    }
                    cmds[i].execute();
                    exit(EXIT_FAILURE);
                }
                else{
                    if (cmds[i].bkg != 0){
                        wait(NULL); // wait if child not a background process
                    }
                }

                if (i != 0){ // closing previous pipes
                    close(FD[0]);
                    close(FD[1]);
                }
                if (i + 1 != num_cmds){ // piping fds
                    FD[0] = newFD[0], FD[1] = newFD[1];
                }
            }

            if (cmds.back().bkg == 0){
                while (wait(&status) > 0);
            }
        }
    } while (true);
}

int main(int argc, char **argv)
{
    shell_loop();
    return EXIT_SUCCESS;
}
