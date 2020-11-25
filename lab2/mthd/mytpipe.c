#include<stdio.h>
#include<unistd.h>
#include<pthread.h>
#include<stdlib.h>

void fx(int *);  // fx = fx-1 * x
void fy(int *);  // fy = fy-1 + fy-2
void fxy(int *);  // fxy = fx + fy
int pipex[2], pipey[2];  // 2个无名管道
int x, y;  // value
pthread_t thrdx, thrdy, thrdxy;  // 分别计算的3个线程


int main(int argc, char *argv[]) {
    int ret;
    int tnum1 = 1, tnum2 = 2, tnum3 = 3;
    // set value
    x = 1, y = 1;
    if(argc == 3) {
        x = atoi(argv[1]);
        y = atoi(argv[2]);
    }
    // 创建无名管道
    if(pipe(pipex) < 0) {
        perror("pipe not create");
        exit(EXIT_FAILURE);
    }
    if(pipe(pipey) < 0) {
        perror("pipe not create");
        exit(EXIT_FAILURE);
    }

    // 创建线程1 fx
    ret = pthread_create(&thrdx, NULL, (void *) fx, (void *) &tnum1);
    if (ret) {
        perror("pthread_create: fx");
        exit(EXIT_FAILURE);
    }
    // 创建线程2 fy
    ret = pthread_create(&thrdy, NULL, (void *) fy, (void *) &tnum2);
    if (ret) {
        perror("pthread_create: fy");
        exit(EXIT_FAILURE);
    }
    // 创建线程3 fxy
    ret = pthread_create(&thrdxy, NULL, (void *) fxy, (void *) &tnum3);
    if (ret) {
        perror("pthread_create: fxy");
        exit(EXIT_FAILURE);
    }

    // 挂起当前线程，切换至 t3
    pthread_join(thrdxy, NULL);
    // 挂起当前线程，切换至 t2
    pthread_join(thrdy, NULL);
    // 挂起当前线程，切换至 t1
    pthread_join(thrdx, NULL);

    return EXIT_SUCCESS;
}

// t1:fx
void fx(int *tnum) {
    int fx = 1, prefx = 1;
    int i;
    for(i=2; i<=x; i++) {
        fx = prefx * i;
        prefx = fx;
    }
    printf("thread: %d   fx: %d\n", *tnum, fx);
    write(pipex[1], &fx, sizeof(int));
    // 写完成，关闭管道
    close(pipex[1]);
}
// t2: fy
void fy(int *tnum) {
    //close(pipey[0]);
    int fy = 1, fy_1 = 1, fy_2 = 1;
    int i;
    for(i=3; i<=y; i++) {
        fy = fy_1 + fy_2;
        fy_2 = fy_1;
        fy_1 = fy;
    }
    printf("thread: %d   fy: %d\n", *tnum, fy);
    write(pipey[1], &fy, sizeof(int));
    // 写完成，关闭管道
    close(pipey[1]);
}
// t3: fxy
void fxy(int *tnum) {
    // 计算
    int fx, fy;
    read(pipex[0], &fx, sizeof(int));
    read(pipey[0], &fy, sizeof(int));
    printf("thread: %d   f(x,y) = %d\n",*tnum, fx + fy);
    // 关闭
    close(pipex[0]);
    close(pipey[0]);
}