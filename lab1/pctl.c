/*
* Filename: pctl.c
* copyright : (C) 2006 by zhanghonglie
* Function: ⽗⼦进程的并发执⾏
*/
#include "pctl.h"
int main(int argc, char *argv[]) {
// 如果在命令⾏上没输⼊⼦进程要执⾏的命令，则执⾏缺省的命令
    int i;
    int pid; // 存放⼦进程号
    int status; // 存放⼦进程返回状态
    char *args[] = {"/bin/ls", "-a", NULL}; // ⼦进程执⾏缺省的命令
    signal(SIGINT, (sighandler_t)sigcat); // 注册⼀个本进程处理中断信号的函数
    pid = fork() ; // 建⽴⼦进程，成功则返回⼦进程号，否则返回 -1
    if (pid < 0) { // 建⽴⼦进程失败
        printf("Create Process failed!\n");
        exit(EXIT_FAILURE);
    }

    if (pid == 0) { // ⼦进程执⾏代码段
        // 报告⽗⼦进程进程号
        printf("I am Child process %d, my Father is %d\n", getpid(), getppid());
        // 唤醒⽗进程
        if (argv[1] == NULL && kill(getppid(), SIGINT) >= 0)
            printf("Child %d Wakeup Father %d\n", getpid(), getppid());
        // 暂停，等待中断信号唤醒
        pause();
        // ⼦进程被键盘中断信号唤醒继续执⾏
        printf("Child %d is Running: \n", getpid());

        if (argv[1] != NULL) {
        // 如果在命令⾏上输⼊了⼦进程要执⾏的命令，则执⾏输⼊的命令
            for (i = 1; argv[i] != NULL; i++)
                printf("%s ", argv[i]); printf("\n");
            // 装⼊并执⾏新的程序
            status = execve(argv[1], &argv[1], NULL);
        }
        else {
            // 如果在命令⾏上没输⼊⼦进程要执⾏的命令，则执⾏缺省的命令
            for (i = 0; args[i] != NULL; i++)
                printf("%s ", args[i]); printf("\n");
            // 装⼊并执⾏新的程序
            status = execve(args[0], args, NULL);
        }
    } 
    else { // ⽗进程执⾏代码段
        // 报告⽗进程进程号
        printf("I am Parent process %d\n", getpid());
        if (argv[1] != NULL) {
        // 如果在命令⾏上输⼊了⼦进程要执⾏的命令，则⽗进程等待⼦进程执⾏结束
            printf("Parent %d is Waiting for Child done\n\n", getpid());
            // 等待⼦进程结束
            waitpid(pid, &status, 0);
            printf("\nMy Child Exit with status = %d\n\n", status);
        } 
        else {
            // 暂停，等待中断信号唤醒
            pause();
            // 唤醒⼦进程，与⼦进程并发执⾏不等待⼦进程执⾏结束
            if (kill(pid, SIGINT) >= 0)
                printf("\nParent %d Wakeup Child %d\n", getpid(), pid);
            printf("Parent %d don't Wait for Child done\n\n", getpid());
        }
    }
    return EXIT_SUCCESS;
}