/*
* main.c :description
* copyright : (C) by zhanghonglie
* Function : 利⽤管道实现在在线程间传递整数
*/
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <pthread.h>
void task1(int *); // 线程 1 执⾏函数原型
void task2(int *); // 线程 2 执⾏函数原型
int pipe1[2], pipe2[2]; // 存放的两个⽆名管道标号
pthread_t thrd1, thrd2; // 存放的两个线程标识


int main(int argc, char *arg[]) {
    int ret;
    int num1, num2;
    // 使⽤ pipe() 系统调⽤建⽴两个⽆名管道。建⽴不成功程序退出，执⾏终⽌
    if (pipe(pipe1) < 0) {
        perror("pipe1 not create");
        exit(EXIT_FAILURE);
    }
    if (pipe(pipe2) < 0) {
        perror("pipe2 not create");
        exit(EXIT_FAILURE);
    }
    // 使⽤ pthread_create 系统调⽤建⽴两个线程。建⽴不成功程序退出，执⾏终⽌
    num1 = 1;
    ret = pthread_create(&thrd1, NULL, (void *) task1, (void *) &num1);
    if (ret) {
        perror("pthread_create: task1");
        exit(EXIT_FAILURE);
    }
    num2 = 2;
    ret = pthread_create(&thrd2, NULL, (void *) task2, (void *) &num2);
    if (ret) {
        perror("pthread_create: task2"); exit(EXIT_FAILURE);
    }
    // 挂起当前线程切换到 thrd2 线程
    pthread_join(thrd2, NULL);
    // 挂起当前线程切换到 thrd1 线程
    pthread_join(thrd1, NULL);
    exit(EXIT_SUCCESS);
}


// 线程 1 执⾏函数，它⾸先向管道写，然后从管道读
void task1(int *num) {
    int x = 1;
    // 每次循环向管道 1 的 1 端写⼊变量 X 的值
    // 并从管道 2 的 0 端读⼀整数写⼊ X 再对 X 加 1，直到 X ⼤于 10
    do {
        printf("thread%d read: %d\n", *num, x++);
        write(pipe1[1], &x, sizeof(int));
        read(pipe2[0], &x, sizeof(int));
    } while (x <= 9);
    // 读写完成后，关闭管道
    close(pipe1[1]);
    close(pipe2[0]);
}

// 线程 2 执⾏函数，它⾸先从管道读，然后向管道写
void task2(int *num) {
    int x;
    // 每次循环从管道 1 的 0 端读⼀个整数放⼊变量 X 中
    // 并对 X 加 1 后写⼊管道 2 的 1 端，直到 X ⼤于 10
    do {
        read(pipe1[0], &x, sizeof(int));
        printf("thread2 read: %d\n", x++);
        write(pipe2[1], &x, sizeof(int));
    } while (x <= 9);
    // 读写完成后，关闭管道
    close(pipe1[0]);
    close(pipe2[1]);
}