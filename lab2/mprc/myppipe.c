#include<stdio.h>
#include<unistd.h>
#include<stdlib.h>

int main(int argc, char *argv[]) {
    int i;  // loop index
    int pid1, pid2;  // 进程号 fx,fy
    int pipex[2], pipey[2];  // fx,fy
    int x, y;
    // set x,y
    x = 1, y = 1;  // default value
    if(argc == 3) {
        x = atoi(argv[1]);
        y = atoi(argv[2]);
    }
    

    // 创建2个无名管道，失败则返回
    if(pipe(pipex) < 0) {
        perror("pipe not create");
        exit(EXIT_FAILURE);
    }
    if(pipe(pipey) < 0) {
        perror("pipe not create");
        exit(EXIT_FAILURE);
    }

    // 创建子进程1
    if((pid1 = fork()) < 0) {
        perror("process not create");
        exit(EXIT_FAILURE);
    }
    else if(pid1 == 0) {  // 子进程1代码段  fx = fx-1 * x
        // 在pipex 的 1 端写
        close(pipex[0]);
        int fx = 1, prefx = 1;
        for(i=2; i<=x; i++) {
            fx = prefx * i;
            prefx = fx;
        }
        printf("fx: %d\n", fx);
        
        write(pipex[1], &fx, sizeof(int));
        close(pipex[1]);
        exit(EXIT_SUCCESS);
    }
    else {
        // 创建子进程2
        if((pid2 = fork()) < 0) {
            perror("process not create");
            exit(EXIT_FAILURE);
        }
        else if(pid2 == 0) {  // 子进程2代码段   fy = fy-1 + fy-2
            // 在pipey的1端写
            close(pipey[0]);
            int fy = 1, fy_1 = 1, fy_2 = 1;
            for(i=3; i<=y; i++) {
                fy = fy_1 + fy_2;
                fy_2 = fy_1;
                fy_1 = fy;
            }
            printf("fy: %d\n", fy);

            write(pipey[1], &fy, sizeof(int));
            close(pipey[1]);
            exit(EXIT_SUCCESS);
        }
        else {  // 父进程代码段   fxy = fx + fy
            close(pipex[1]);
            close(pipey[1]);
            // 计算
            int fx, fy;
            read(pipex[0], &fx, sizeof(int));
            read(pipey[0], &fy, sizeof(int));
            printf("f(x,y) = %d\n", fx + fy);
            // 关闭
            close(pipex[0]);
            close(pipey[0]);
        }
    }
    

    return EXIT_SUCCESS;
}