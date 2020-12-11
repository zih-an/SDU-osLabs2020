#include "myipc.h"

int main(int argc, char* argv[]) {
    //
    const char matrials[3][10] = { "tobacco", "paper", "glue"};
    int mtl;
    if(argv[1]==NULL) return EXIT_FAILURE;
    else  mtl = atoi(argv[1]);
    if(mtl > 2) return EXIT_FAILURE;
    
    int rate;
    if(argv[2]!=NULL) rate = atoi(argv[2]);
    else rate = 3;

    // set keys
    buff_key = 101;
    pput_key = 102;
    pput_prod_key = 103;
    prod_key = 201;
    pmtx_key = 202;
    cons_key = 301;
    cmtx_key = 302;

    // set buffer
    shm_flg = IPC_CREAT | 0644; // 共享内存读写权限
    buff_num = 2;  // 2 in all
    buff_ptr = (char *)set_shm(buff_key, buff_num, shm_flg);

    // set semaphores
    sem_flg = IPC_CREAT | 0644;
    // 消费者同步+互斥
    sem_val = 0;
    cons_sem = set_sem(cons_key, sem_val, sem_flg);
    sem_val = 3;
    cmtx_sem = set_sem(cmtx_key, sem_val, sem_flg);
    // 生产者同步
    sem_val = buff_num;
    prod_sem = set_sem(prod_key, sem_val, sem_flg);

    while(1) {
        down(cons_sem);  // 没有满，阻塞
        down(cmtx_sem);  // 同时允许3个smoker，但只有一个会满足
        int flag = 0;
        sleep(rate);
        if( (buff_ptr[0] == (mtl+1)%3 && buff_ptr[1] == (mtl+2)%3)
            || (buff_ptr[1] == (mtl+1)%3 && buff_ptr[0] == (mtl+2)%3) ){
            printf("[%s smoker] SMOKE!\n", matrials[mtl]);
            flag = 1;
        }
        up(cmtx_sem);
        int i;
        for(i=0;i<2&&flag;i++) up(prod_sem);  // wake up producer twice
    }
    
    return EXIT_SUCCESS;
}