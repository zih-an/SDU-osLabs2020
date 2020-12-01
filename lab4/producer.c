#include "myipc.h"

int main(int argc, char* argv[]) {
    const char matrials[3][10] = { "tobacco", "paper", "glue"};
    int rate;
    if(argv[1]!=NULL) rate = atoi(argv[1]);
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
    // set producer position
    pput_num = 1;
    pput_ptr = (int *)set_shm(pput_key, pput_num, shm_flg);
    // set product
    pput_prod_num = 1;
    pput_prod_ptr = (int *)set_shm(pput_prod_key, pput_prod_num, shm_flg);

    // set semaphores
    sem_flg = IPC_CREAT | 0644;
    // 生产者同步+互斥
    sem_val = buff_num;
    prod_sem = set_sem(prod_key, sem_val, sem_flg);
    sem_val = 1;
    pmtx_sem = set_sem(pmtx_key, sem_val, sem_flg);
    // 消费者同步
    sem_val = 0;
    cons_sem = set_sem(cons_key, sem_val, sem_flg);

    // put products
    while(1) {
        down(prod_sem);  // buffer->full 阻塞
        down(pmtx_sem);  // 只允许1个生产者放

        buff_ptr[*pput_ptr] = *pput_prod_ptr;
        printf("[producer] PUT %s in BUFFER[%d]\n", matrials[*pput_prod_ptr], *pput_ptr);

        *pput_prod_ptr = (*pput_prod_ptr + 1) % 3;
        *pput_ptr = (*pput_ptr + 1) % 2;

        up(pmtx_sem);
        if((*pput_ptr) == 0)  // full
            up(cons_sem);
    }
    return EXIT_SUCCESS;
}