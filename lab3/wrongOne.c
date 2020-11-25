

递归实现多管道（测试程序测试错误）

// consider 1 command and 2 pipes:  pipe_left -> cmd -> pipe_right
void pipe_run(char* cmd[], int len) {
    bool redct_output, redct_input;  // flags
    int fd_in, fd_out;  // files
    int tmp;
    for(tmp=0; tmp < len; tmp++) {
        // redirect: >
        if(strcmp(cmd[tmp], ">") == 0) {
            redct_output = 1;
            // open file
            fd_out = open(cmd[tmp+1], O_CREAT|O_WRONLY, 0777);
            dup2(fd_out, 1);
            cmd[tmp] = NULL;  // truncation
            break;
        }
        // redirect: <
        else if(strcmp(cmd[tmp], "<") == 0) {
            redct_input = 1;
            // open file
            fd_in = open(cmd[tmp+1], O_RDONLY);
            dup2(fd_in, 0);
            cmd[tmp] = NULL;  // truncation
            break;
        }
    }
    // execute here
    //execCmd(cmd, len);
    execvp(cmd[0], cmd);

    if(redct_input) {
        close(fd_in);
    }
    if(redct_output) {
        close(fd_out);
    }
}

// left output |to| right input
void pipe_build(int cmd_beg, char* argv[], int cnt, int *left_pipe) {
    if(cnt <= 0) return;
    // build a pipe
    int *pipe_right = NULL;
    if(cnt > 1) {
        pipe_right = (int*)malloc(2*sizeof(int));
        pipe(pipe_right);
    }
    // get cmd
    char *cmd[20];
    int i = 0;
    while(argv[cmd_beg] != NULL) {
        cmd[i++] = argv[cmd_beg++];
    }
    cmd[i] = NULL;

    int pid_2 = fork();
    if(pid_2 < 0) exit(EXIT_FAILURE);
    else if(pid_2 == 0) {  // child process: bulid grandson processes...
        if(left_pipe != NULL) {
            close(left_pipe[0]);
            close(left_pipe[1]);
        }
        pipe_build(cmd_beg + 1, argv, cnt - 1, pipe_right);  // recursion
        exit(EXIT_SUCCESS);
    }
    else {  // father process: execute current cmd
        //pipe_run(cmd, left_pipe, pipe_right, i);
    }
}
