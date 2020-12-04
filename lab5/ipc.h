#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/sem.h>
#include <sys/msg.h>
#define BUFSZ 256
#define MAXVAL 100
#define STRSIZ 8

#define ENTERQUEST 1 //顾客进店请求标识
#define CUSTOMER 2  //顾客标识（椅子、沙发、等候室）
#define CASHQUEST 3 //请求结账标识


/*信号灯控制⽤的共同体*/
typedef union semuns {
    int val;
} Sem_uns;

/* 消 息 结 构 体 */
typedef struct msgbuf {
    long mtype;
    int mid;
} Msg_buf;


key_t buff_key;
int buff_num;
char *buff_ptr;
int shm_flg;


/* 消息队列 */
// 13个等候室消息
int wroom_flg;
key_t wroom_key;
int wroom_id;

// 4个沙发消息
int sofa_flg;
key_t sofa_key;
int sofa_id;

// 3张椅子消息
int chair_flg;
key_t chair_key;
int chair_id;

// 1个登记册消息
int account_flg;
key_t account_key;
int account_id;

// 请求消息队列
int quest_flg;
key_t quest_key;
int quest_id;

// 反馈消息队列
int respond_flg;
key_t respond_key;
int respond_id;

// 记录座位使用情况
int chair_cnt;
int sofa_cnt;
int wroom_cnt;



int get_ipc_id(char *proc_file, key_t key);
char *set_shm(key_t shm_key, int shm_num, int shm_flag);
int set_msq(key_t msq_key, int msq_flag);
int set_sem(key_t sem_key, int sem_val, int sem_flag);
int down(int sem_id);
int up(int sem_id);
