#include "pctl.h"

int main(int argc, char *argv[]) {
    // ⼦进程执⾏缺省的命令
    char *args1[] = {"/bin/ls", "-a", NULL}; 
    char *args2[] = {"/bin/ps", "-l", NULL};
    int pid1, pid2; // 子进程号
    int status1, status2; // 子进程状态
    signal(SIGINT, (sighandler_t)sigcat); // 注册⼀个本进程处理中断信号的函数
    
    // 子进程1
    pid1 = fork();
    if(pid1 < 0) { // 1 创建失败
        printf("Create Process failed!\n");
        exit(EXIT_FAILURE);
    }

    if(pid1 == 0) { // 子进程1代码段
        kill(getppid(), SIGINT);  // 唤醒父进程，确定pause已执行到
        pause();  // 暂停，等待信号唤醒
        status1 = execve(args1[0], args1, NULL);
    }
    else {
        // 子进程2
        pid2 = fork();
        if(pid2 < 0) {  // 2 创建失败
            printf("Create Process failed!\n");
            exit(EXIT_FAILURE);
        }

        if(pid2 == 0) {  // 子进程2代码段
            status2 = execve(args2[0], args2, NULL);
        }
        else {  // 父进程代码段
            pause(); // 先暂停等待子进程1唤醒，防止死锁
            waitpid(pid2, &status2, 0);  // 先等待子进程2执行完毕
            kill(pid1, SIGINT);  // 唤醒子进程1
            waitpid(pid1, &status1, 0);  // 等待子进程1执行完毕
        }
    }

    return EXIT_SUCCESS;
}