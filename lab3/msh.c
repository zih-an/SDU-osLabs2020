#include <wait.h>
#include <unistd.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>


typedef enum {FALSE = 0, TRUE = !FALSE} bool;  // c language doesn't have bool type...
char* args[100][100];
int stdout_id = 101, stdin_id = 100;  // preserved io descriptors
//
int pids[100], num;  // preserve child pids
typedef void (*sighandler_t) (int);
void sigcat() {  // kill all children
    int i; for(i=0; i<num; i++) kill(pids[i], SIGINT);
}

void execCmd(char* cmd[], int);
void pipe_single(char* cmd[], int);


// main
int main(int argc, char* argv[]) {
    // preserve io descriptors first
    dup2(0, stdin_id);
    dup2(1, stdout_id);
    signal(SIGINT, (sighandler_t)sigcat);  // register

    //
    int i=0,j=0,z=0; // index
    char c;
    char *attr = (char*)malloc(20*sizeof(char));

    while((c = getchar()) != EOF) {
        if(c == '\n' || c == ' ') {  // append an attribute
            attr[i] = '\0';
            args[z][j++] = attr;
            i = 0;
            //
            char* newAttr = (char*)malloc(20*sizeof(char));
            attr = newAttr;
            if(c != '\n') continue;
        }

        if(c == '\n') {  // a new command
            args[z][j] = NULL;
            // excute
            execCmd(args[z], j);
            z++; j=0;
        }
        else {  // add character to attributes
            attr[i++] = c;
        }
    }

    return EXIT_SUCCESS;
}


// execute command
void execCmd(char* cmd[], int len) {
    int tmp=0;  // index
    bool redct_output, redct_input;  // flags
    int fd_in, fd_out;  // files

    // fork a child process to execute the command
    int pid_cmd = fork();
    if(pid_cmd < 0) exit(EXIT_FAILURE);
    pids[num++] = pid_cmd;

    if(pid_cmd == 0) {  // child: must use child process to excute execvp, for multiple commands 
        int cnt_pipe = 0;  // count numbers of pipe
        
        // examine pipes first
        for(tmp=0; tmp < len; tmp++){
            // pipe connection
            if(strcmp(cmd[tmp], "|") == 0) {
                cnt_pipe++;
                cmd[tmp] = NULL;  // truncation
            }
        }
        
        // examine redirections when pipes are not existed
        for(tmp=0; cnt_pipe<=0 && tmp < len; tmp++) {
            // redirect: >
            if(strcmp(cmd[tmp], ">") == 0) {
                redct_output = 1;
                // open file
                fd_out = open(cmd[tmp+1], O_CREAT|O_WRONLY, 0777);
                dup2(fd_out, 1);
                cmd[tmp] = NULL;  // truncation
            }
            // redirect: <
            else if(strcmp(cmd[tmp], "<") == 0) {
                redct_input = 1;
                // open file
                fd_in = open(cmd[tmp+1], O_RDONLY);
                dup2(fd_in, 0);
                cmd[tmp] = NULL;  // truncation
            }
        }
        
        // handle with |
        if(cnt_pipe > 0) pipe_single(cmd, cnt_pipe + 1);
        // others
        else execvp(cmd[0], cmd);
        exit(EXIT_SUCCESS);
    }
    else {  // father process
        waitpid(pid_cmd, NULL, 0);
        num--;
        // refresh
        if(redct_input) close(fd_in);
        if(redct_output) close(fd_out);
        dup2(stdin_id, 0);
        dup2(stdout_id, 1);
    }
}

void pipe_single(char* argv[], int all) {
    // pipe preparations
    int cmd_beg=0, cnt=0, i=0, pipe_rd=0, pipe_wt=1;
    char* cmd[100];
    int pipes[100][2];  for(i=0; i<all; i++) pipe(pipes[i]);

    // run
    for(cnt=0; cnt<all; cnt++) {
        // get one cmd
        i=0;
        while(argv[cmd_beg] == NULL) cmd_beg++;  // skip NULL
        while(argv[cmd_beg] != NULL) cmd[i++] = argv[cmd_beg++];
        cmd[i] = NULL;

        // fork a child process to execute the command
        int pid_pipe = fork();
        if(pid_pipe < 0) exit(EXIT_FAILURE);
        pids[num++] = pid_pipe;
        
        if(pid_pipe == 0) {
            close(pipes[pipe_rd][1]);  // close read-write
            close(pipes[pipe_wt][0]);  // close write-read
            if(cnt != 0) dup2(pipes[pipe_rd][0], 0);
            if(cnt != all-1) dup2(pipes[pipe_wt][1], 1);

            // execute here
            execCmd(cmd, i);

            // finish
            close(pipes[pipe_rd][0]);
            close(pipes[pipe_wt][1]);
            dup2(stdin_id, 0);
            dup2(stdout_id, 1);
            exit(EXIT_SUCCESS);
        }
        else {
            // close redundant port  -> can't be removed
            close(pipes[pipe_rd][0]);
            close(pipes[pipe_wt][1]);

            // wait for the child process
            waitpid(pid_pipe, NULL, 0);
            num--;
            // finish | refresh pipe & stdio  -> must be removed
            //close(pipes[pipe_rd][1]);
            //close(pipes[pipe_wt][0]);
            pipe_wt++; pipe_rd++;
            
            dup2(stdin_id, 0);
            dup2(stdout_id, 1);
        }
    }
}

