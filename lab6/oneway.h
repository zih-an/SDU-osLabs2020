#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/sem.h>
#include <sys/msg.h>
#include <sys/wait.h>

#define min(a,b) ((a)<(b)?(a):(b))

/*信号灯控制⽤的共同体*/
typedef union semuns { int val; } Sem_uns;

//当前道路的状态： 空闲、N-S、S-N
enum WayStatus { NtoS, StoN, spare };


//管程中使⽤的信号量
class Sema {
public:
    Sema(int id);
    ~Sema();
    int down(); //信号量加 1
    int up(); //信号量减 1
private:
    int sem_id; //信号量标识符
};

//管程中使⽤的锁
class Lock {
public:
    Lock(Sema *lock);
    ~Lock();
    void close_lock();
    void open_lock();
private:
    Sema *sema; //锁使⽤的信号量
};



//管程中使⽤的条件变量
class Condition {
public:
    Condition(Sema *Nsm, Sema *Ssm);
    ~Condition();
    void Wait(Lock *lock, int direction); //条件变量阻塞操作
    void Signal(int direction); //条件变量唤醒操作
private:
    Sema *N_sema; //车辆的信号量，N等待队列，每个车有一个信号量
    Sema *S_sema; //S等待队列
};

//行车管程的定义
class Railway {
public:
    Railway(int r, int m1d); //管程构造函数
    ~Railway();
    void drivein(int d);  // 驶入
    void arrive(int d);  // 到达驶出

    //建⽴或获取 ipc 信号量的⼀组函数的原型说明
    int get_ipc_id(char *proc_file, key_t key);
    int set_sem(key_t sem_key, int sem_val, int sem_flag);
    //创建共享内存，放车辆状态
    char *set_shm(key_t shm_key, int shm_num, int shm_flag);
private:
    int rate; //行车速度
    int MAX_1d;  //数量约束，共用
    /* 使用指针，多进程共享变量 */
    int *on_cnt; //路面上正在行驶的车辆计数
    int *pass_cnt; //某方向的过车数(到达+正在行驶)
    int *S_waiting; //S方向等待计数
    int *N_waiting; //N方向等待计数
    char *way_status[1]; //当前公路的行驶状态，enum WayStatus
    
    int block(int d); //阻塞的逻辑表达式

    Condition *go; //通过的条件变量 -以公路为主角，只有1个
    Lock *lock; //控制互斥进⼊管程的锁
};

