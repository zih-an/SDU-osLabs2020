#include "myipc.h"

/*
* get_ipc_id() 从/proc/sysvipc/⽂件系统中获取 IPC 的 id 号
* pfile: 对应/proc/sysvipc/⽬录中的 IPC ⽂件分别为
* msg-消息队列,sem-信号量,shm-共享内存
* key: 对应要获取的 IPC 的 id 号的键值
*/
int get_ipc_id(char *proc_file, key_t key) {
    FILE *pf; int i, j;
    char line[BUFSZ], colum[BUFSZ];

    if ((pf = fopen(proc_file, "r")) == NULL) {
        perror("Proc file not open");
        exit(EXIT_FAILURE);
    }
    fgets(line, BUFSZ, pf);
    while (!feof(pf)) {
        i = j = 0;
        fgets(line, BUFSZ, pf);
        while (line[i] == ' ') i++;
        while (line[i] != ' ') colum[j++] = line[i++];
        colum[j] = '\0';
        if (atoi(colum) != key) continue;
        j = 0;
        while (line[i] == ' ') i++;
        while (line[i] != ' ') colum[j++] = line[i++];
        colum[j] = '\0';
        i = atoi(colum);
        fclose(pf);
        return i;
    }
    fclose(pf);
    return -1;
}

/*
* 信号灯上的down/up 操作  -> 阻塞/唤醒
* semid:信号灯数组标识符
* semnum:信号灯数组下标
* buf:操作信号灯的结构
*/
/* semop  0:阻塞，其余执行
sem_op 的值是负数
（2， -1）
-   如果相应信号灯的值⼤于等于 sem_op 的绝对值。在 semop 函数执⾏以后，相应信号
    灯的值会被减去 sem_op 的绝对值。***（表示可以执行，不阻塞，同时更新信号量）
（0，-1） IPC_NOWAIT已设置
-   如果相应信号灯的值⼩于 sem_op 的绝对值并且 (sem_flg & IPC_NOWAIT) ⾮0，即
    struct sembuf 的第三个参数设置了 IPC_NOWAIT，那么 semop 函数将会直接返回。 ***（可直接执行，什么也不做）
（0，-1） IPC_NOWAIT没设置 -》本实验设置了SEM_UNDO
-   如果相应信号灯的值⼩于 sem_op 的绝对值并且 (sem_flg & IPC_NOWAIT) 是0，即
    struct sembuf 的第三个参数没有设置 IPC_NOWAIT，那么在 semop 函数执⾏以后进程
    将会被挂起。当信号灯的值⼤于等于 sem_op 的绝对值后或者相应的信号灯被删除（即
    不存在）或者进程接收到信号的时候被挂起的进程会被唤醒. ***（表示不可执行，阻塞）

如果 sem_op 的值是正数
-   在 semop 函数执⾏以后，相应信号灯的值会被加上 sem_op。***（表示可以执行，同时更新信号量）
*/
int down(int sem_id) {  // 阻塞
    struct sembuf buf;
    buf.sem_op = -1;
    buf.sem_num = 0;  // 该操作的⽬标信号灯在数组中的下标
    buf.sem_flg = SEM_UNDO;
    if ((semop(sem_id, &buf, 1)) < 0) {
        perror("down error ");
        exit(EXIT_FAILURE);
    }
    return EXIT_SUCCESS;
}

int up(int sem_id) {  // 唤醒
    struct sembuf buf;
    buf.sem_op = 1;  // 在 semop 函数执⾏以后，相应信号灯的值会被加上 sem_op
    buf.sem_num = 0;  // 该操作的⽬标信号灯在数组中的下标
    buf.sem_flg = SEM_UNDO;
    if ((semop(sem_id, &buf, 1)) < 0) {
        perror("up error ");
        exit(EXIT_FAILURE);
    }
    return EXIT_SUCCESS;
}

/*
* set_sem 函数建⽴⼀个具有 n 个信号灯的信号量
* 如果建⽴成功，返回 ⼀个信号灯数组的标识符 sem_id
* 输⼊参数：
* sem_key 信号灯数组的键值
* sem_val 信号灯数组中信号灯的个数
* sem_flag 信号等数组的存取权限
*/
int set_sem(key_t sem_key, int sem_val, int sem_flg) {
    int sem_id;
    Sem_uns sem_arg;
    //测试由 sem_key 标识的信号灯数组是否已经建⽴
    if ((sem_id = get_ipc_id("/proc/sysvipc/sem", sem_key)) < 0 ) {
        //semget 新建⼀个信号灯,其标号返回到 sem_id
        if ((sem_id = semget(sem_key, 1, sem_flg)) < 0) {
            perror("semaphore create error");
            exit(EXIT_FAILURE);
        }
        //设置信号灯的初值
        sem_arg.val = sem_val;
        if (semctl(sem_id, 0, SETVAL, sem_arg) < 0) {
            perror("semaphore set error");
            exit(EXIT_FAILURE);
        }
    }
    return sem_id;
}

/*
* set_shm 函数建⽴⼀个具有 n 个字节 的共享内存区
* 如果建⽴成功，返回 ⼀个指向该内存区⾸地址的指针 shm_buf
* 输⼊参数：
* shm_key 共享内存的键值
* shm_val 共享内存字节的⻓度
* shm_flag 共享内存的存取权限
*/
char *set_shm(key_t shm_key, int shm_num, int shm_flg) {
    int i, shm_id;
    char *shm_buf;
    //测试由 shm_key 标识的共享内存区是否已经建⽴
    if ((shm_id = get_ipc_id("/proc/sysvipc/shm", shm_key)) < 0 ) {
        //shmget 新建 ⼀个⻓度为 shm_num 字节的共享内存,其标号返回shm_id
        if ((shm_id = shmget(shm_key, shm_num, shm_flg)) < 0) {
            perror("shareMemory set error"); exit(EXIT_FAILURE);
        }
        //shmat 将由 shm_id 标识的共享内存附加给指针 shm_buf
        if ((shm_buf = (char *)shmat(shm_id, 0, 0)) < (char *)0) {
            perror("get shareMemory error"); exit(EXIT_FAILURE);
        }
        for (i = 0; i < shm_num; i++) shm_buf[i] = 0; //初始为 0
    }
    //shm_key 标识的共享内存区已经建⽴,将由 shm_id 标识的共享内存附加给指针 shm_buf
    if ((shm_buf = (char *)shmat(shm_id, 0, 0)) < (char *)0) {
        perror("get shareMemory error");
        exit(EXIT_FAILURE);
    }
    return shm_buf;
}

/*
* set_msq 函数建⽴⼀个消息队列
* 如果建⽴成功，返回 ⼀个消息队列的标识符 msq_id
* 输⼊参数：
* msq_key 消息队列的键值
* msq_flag 消息队列的存取权限
*/
int set_msq(key_t msq_key, int msq_flg) {
    int msq_id;
    //测试由 msq_key 标识的消息队列是否已经建⽴
    if ((msq_id = get_ipc_id("/proc/sysvipc/msg", msq_key)) < 0 ) {
        //msgget 新建⼀个消息队列,其标号返回到 msq_id
        if ((msq_id = msgget(msq_key, msq_flg)) < 0) {
            perror("messageQueue set error"); exit(EXIT_FAILURE);
        }
    }
    return msq_id;
}