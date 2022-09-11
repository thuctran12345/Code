#include <iostream>

#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

#include <vector>
#include <string>

#include "Tokenizer.h"
#include <fstream>
#include <fcntl.h>
#include <unistd.h>
#include <vector>

// all the basic colours for a shell prompt
#define RED     "\033[1;31m"
#define GREEN	"\033[1;32m"
#define YELLOW  "\033[1;33m"
#define BLUE	"\033[1;34m"
#define WHITE	"\033[1;37m"
#define NC      "\033[0m"
       #include <sys/time.h>

using namespace std;
vector<pid_t> PIDS;
void check_background_processes()
{
    int s;
    for(__uint64_t i = 0; i < PIDS.size(); i++)
    {
        if((s = waitpid(PIDS[i], 0, WNOHANG)) > 0)
        {
            cout << "killed " << *(PIDS.begin() + i) << endl;
            PIDS.erase(PIDS.begin() + i);
            i--;
        }
    }
}

void create_process(int in, int out, Command *cmd)
{
    pid_t pid = fork();
    if (pid < 0) {  // error check
        perror("fork");
        exit(2);
    }

    if (pid == 0) {  // if child, exec to run command
        // run single commands with no arguments
        // file redirection
        //char* args[] = {(char*) tknr.commands.at(0)->args.at(0).c_str(), nullptr};
        if(cmd->hasOutput())
        {
            int fd = open(cmd->out_file.c_str(), O_WRONLY | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR);
            if(fd == -1)
            {
                perror("open");
                exit(1);
            }
            if(dup2(fd, STDOUT_FILENO) == -1)
            {
                perror("d2");
                exit(1);
            }
            close(fd);
        }
        else if(out!=1) dup2(out, 1);

        if(cmd->hasInput())
        {
            int fd = open(cmd->in_file.c_str(), O_RDONLY);
            if(fd == -1)
            {
                perror("open");
                exit(1);
            }
            if(dup2(fd, STDIN_FILENO) == -1)
            {
                perror("d2");
                exit(1);
            }
            close(fd);
        }
        else if(in!=0) dup2(in, 0);

        char **args = new char *[cmd->args.size() + 1];
        for(__uint64_t i = 0; i < cmd->args.size(); i++) args[i] = const_cast<char *> (cmd-> args[i].c_str());
        args[cmd->args.size()] = nullptr;

        if (execvp(args[0], args) < 0) {  // error check
            perror("execvp");
            exit(2);
        }
    }
            waitpid(pid, nullptr, 0);

}

void process_pipe(Tokenizer &tknr)
{
    int in = 0;
    int out;
    int fd[2];
    for(__uint64_t i = 0; i < tknr.commands.size(); i++)
    {
        pipe(fd);
        if(i== tknr.commands.size() - 1 && !tknr.commands.back()->hasOutput()) out = 1;
        else out = fd[1];
        create_process(in, out, tknr.commands[i]);
        if(out!=1) close(out);
        in = fd[0];
    }
}

int main () {
    vector<string> hist;

    for (;;) {
        // need date/time, username, and absolute path to current dir
        check_background_processes();
        char *cwdir = getcwd(NULL, 0);
        string currentDir(cwdir);
        hist.push_back(currentDir);
        free(cwdir);
        struct timeval tv;
        time_t t;
        struct tm *info;

        t = tv.tv_sec;

        gettimeofday(&tv, NULL);
        t = tv.tv_sec;
        info = localtime(&t);
        cout << YELLOW << asctime(info) << currentDir << NC << " ";
        // get user inputted command
        string input;
        getline(cin, input);

        if (input == "exit") {  // print exit message and break out of infinite loop
            cout << RED << "Now exiting shell..." << endl << "Goodbye" << NC << endl;
            break;
        }

        // get tokenized commands from user input
        Tokenizer tknr(input);
        if (tknr.hasError()) {  // continue to next prompt if input had an error
            continue;
        }

        // // print out every command token-by-token on individual lines
        // // prints to cerr to avoid influencing autograder
        // for (auto cmd : tknr.commands) {
        //     for (auto str : cmd->args) {
        //         cerr << "|" << str << "| ";
        //     }
        //     if (cmd->hasInput()) {
        //         cerr << "in< " << cmd->in_file << " ";
        //     }
        //     if (cmd->hasOutput()) {
        //         cerr << "out> " << cmd->out_file << " ";
        //     }
        //     cerr << endl;
        // }

        // cd


    if(tknr.commands.size() < 2)
    {
        if(tknr.commands[0] -> args[0] == "cd")
        {
            string cmand = tknr.commands[0]->args[1];

            if(tknr.commands[0]->args[1] == "-")
            {
                cmand = hist.rbegin()[1];
                if(hist.size() == 1)
                {
                    continue;
                }
                else
                {
                    cmand = hist.rbegin()[1];
                }
            }
            
            if(chdir(cmand.c_str()) < 0)
            {
                cerr<<"baed\n";
            }
            continue;
        }
        pid_t pid = fork();
        if (pid < 0) {  // error check
            perror("fork");
            exit(2);
        }

        if (pid == 0) {  // if child, exec to run command
            // run single commands with no arguments
            // file redirection
            //char* args[] = {(char*) tknr.commands.at(0)->args.at(0).c_str(), nullptr};
            Command *cmd = tknr.commands.back();
            if(cmd->hasOutput())
            {
                int fd = open(cmd->out_file.c_str(), O_WRONLY | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);
                if(fd == -1)
                {
                    perror("open");
                    exit(1);
                }
                if(dup2(fd, STDOUT_FILENO) == -1)
                {
                    perror("d2");
                    exit(1);
                }
                close(fd);
            }
            if(cmd->hasInput())
            {
                int fd = open(cmd->in_file.c_str(), O_RDONLY);
                if(fd == -1)
                {
                    perror("open");
                    exit(1);
                }
                if(dup2(fd, STDIN_FILENO) == -1)
                {
                    perror("d2");
                    exit(1);
                }
                close(fd);
            }
            

            char **args = new char *[cmd->args.size() + 1];
            for(__uint64_t i = 0; i < cmd->args.size(); i++) args[i] = const_cast<char *> (cmd-> args[i].c_str());
            args[cmd->args.size()] = nullptr;

            if (execvp(args[0], args) < 0) {  // error check
                perror("execvp");
                exit(2);
            }
        }
        else {  // if parent, wait for child to finish
            int status = 0;
            waitpid(pid, &status, 0);

            if (status > 1) {  // exit if child didn't exec properly
                exit(status);
            }
        }
    }
        // for each command...

        // fork to create child
    else process_pipe(tknr);
    }
}