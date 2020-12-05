#include "oneway.h"


/* 信号量 */
Sema::Sema(int id) {
    sem_id = id;
}
Sema::~Sema() { }
int Sema::down() {
    struct sembuf buf;
    buf.sem_op = -1;
    buf.sem_num = 0;
    buf.sem_flg = SEM_UNDO;
    if ((semop(sem_id, &buf, 1)) < 0) {
        perror("down error ");
        exit(EXIT_FAILURE);
    }
    return EXIT_SUCCESS;
}
int Sema::up() {
    Sem_uns arg;
    struct sembuf buf;
    buf.sem_op = 1;
    buf.sem_num = 0;
    buf.sem_flg = SEM_UNDO;
    if ((semop(sem_id, &buf, 1)) < 0) {
        perror("up error ");
        exit(EXIT_FAILURE);
    }
    return EXIT_SUCCESS;
}


/* 锁 */
Lock::Lock(Sema *s) {
    sema = s;
}
Lock::~Lock() {}
void Lock::close_lock() {
    sema->down();
}
void Lock::open_lock() {
    sema->up();
}


/* 条件变量 */
Condition::Condition(Sema *Nsm, Sema *Ssm){
    N_sema = Nsm;
    S_sema = Ssm;
}
Condition::~Condition(){ }
void Condition::Wait(Lock *lock, int direction) {
    lock->open_lock();
    // 加入相应的等待队列
    if(direction == NtoS) { N_sema->down(); }
    else if(direction == StoN) { S_sema->down(); }
    lock->close_lock();
}
void Condition::Signal(int direction) {
    // wake up cars in the direction
    if(direction == NtoS) { N_sema->up(); }
    else if(direction == StoN) { S_sema->up(); }
}


/* 行车管程 */
// ipc 操作
int Railway::get_ipc_id(char *proc_file, key_t key) {
    #define BUFSZ 256
    FILE *pf;
    int i, j;
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
        if (atoi(colum) != key)
            continue;
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
int Railway::set_sem(key_t sem_key, int sem_val, int sem_flg) {
    int sem_id;
    Sem_uns sem_arg;
    //测试由 sem_key 标识的信号量是否已经建⽴
    if ((sem_id = get_ipc_id("/proc/sysvipc/sem", sem_key)) < 0 ) {
        //semget 新建⼀个信号灯,其标号返回到sem_id
        if ((sem_id = semget(sem_key, 1, sem_flg)) < 0) {
            perror("semaphore create error");
            exit(EXIT_FAILURE);
        }
    }
    //设置信号量的初值
    sem_arg.val = sem_val;
    if (semctl(sem_id, 0, SETVAL, sem_arg) < 0) {
        perror("semaphore set error");
        exit(EXIT_FAILURE);
    }
    return sem_id;
}
char *Railway::set_shm(key_t shm_key, int shm_num, int shm_flg) {
    int i, shm_id;
    char *shm_buf;

    //测试由 shm_key 标识的共享内存区是否已经建⽴
    if ((shm_id = get_ipc_id("/proc/sysvipc/shm", shm_key)) < 0) {
        //shmget 新建 ⼀个⻓度为 shm_num 字节的共享内存
        if ((shm_id = shmget(shm_key, shm_num, shm_flg)) < 0) {
            perror("shareMemory set error");
            exit(EXIT_FAILURE);
        }
        //shmat 将由 shm_id 标识的共享内存附加给指针 shm_buf
        if ((shm_buf = (char *)shmat(shm_id, 0, 0)) < (char *)0) {
            perror("get shareMemory error");
            exit(EXIT_FAILURE);
        }
        for (i = 0; i < shm_num; i++)
            shm_buf[i] = 0; //初始为 0
    }
    //共享内存区已经建⽴,将由 shm_id 标识的共享内存附加给指针 shm_buf
    if ((shm_buf = (char *)shmat(shm_id, 0, 0)) < (char *)0) {
        perror("get shareMemory error");
        exit(EXIT_FAILURE);
    }
    return shm_buf;
}
// 
Railway::Railway(int r, int m1d) {
    rate = r;
    MAX_1d = m1d;
    on_cnt = 0;
    pass_cnt = 0;
    S_waiting = 0;
    N_waiting = 0;
    //
    int ipc_flg = IPC_CREAT | 0644;
    int shm_key = 220;
    int shm_num = 1;
    int sem_key = 120;
    int sem_val = 0;
    int sem_id;
    int sem_N, sem_S;
    Sema *sema, *sema_N, *sema_S;

    //只允许一个进程进入的互斥锁设置
    if ((sem_id = set_sem(sem_key++, 1, ipc_flg)) < 0) {
        perror("Semaphor create error");
        exit(EXIT_FAILURE);
    }
    sema = new Sema(sem_id);
    lock = new Lock(sema);

    //设置公路可共享的状态，初始为空闲
    if ((way_status[0] = (char *)set_shm(shm_key++, shm_num, ipc_flg)) == NULL) {
        perror("Share memory create error");
        exit(EXIT_FAILURE);
    }
    *way_status[0] = spare;
    //设置on_cnt
    if ((on_cnt = (int *)set_shm(shm_key++, shm_num, ipc_flg)) == NULL) {
        perror("Share memory create error");
        exit(EXIT_FAILURE);
    }
    *on_cnt = 0;
    //设置pass_cnt
    if ((pass_cnt = (int *)set_shm(shm_key++, shm_num, ipc_flg)) == NULL) {
        perror("Share memory create error");
        exit(EXIT_FAILURE);
    }
    *pass_cnt = 0;
    //设置S_waiting
    if ((S_waiting = (int *)set_shm(shm_key++, shm_num, ipc_flg)) == NULL) {
        perror("Share memory create error");
        exit(EXIT_FAILURE);
    }
    *S_waiting = 0;
    //设置N_waiting
    if ((N_waiting = (int *)set_shm(shm_key++, shm_num, ipc_flg)) == NULL) {
        perror("Share memory create error");
        exit(EXIT_FAILURE);
    }
    *N_waiting = 0;
    

    //条件变量
    if ((sem_N = set_sem(sem_key++, sem_val, ipc_flg)) < 0) {
        perror("Semaphor create error");
        exit(EXIT_FAILURE);
    }
    sema_N = new Sema(sem_N); //N
    if ((sem_S = set_sem(sem_key++, sem_val, ipc_flg)) < 0) {
        perror("Semaphor create error");
        exit(EXIT_FAILURE);
    }
    sema_S = new Sema(sem_S); //S
    go = new Condition(sema_N, sema_S);
}
Railway::~Railway(){ }
int Railway::block(int d) {
    int con = *pass_cnt<MAX_1d 
        && (*way_status[0]==spare 
            || (*way_status[0]==d && *on_cnt<MAX_1d));
    return !con;
}
void Railway::drivein(int d) {
    lock->close_lock(); //上锁
    int flag=0; //for output message
    while(block(d)) { //阻塞等待
        if(d == NtoS) (*N_waiting)++;
        else if(d == StoN) (*S_waiting)++;
        if(!flag) { flag=1; printf("[Waiting] D-%d: %d\n", d, getpid()); }
        go->Wait(lock, d);
        if(d == NtoS) (*N_waiting)--;
        else if(d == StoN) (*S_waiting)--;
    }
    (*on_cnt)++; (*pass_cnt)++;
    *way_status[0] = d;
    printf("[Driving] D-%d: %d\n", d, getpid());
    printf("    [D: %d] On: %d\n", d, *on_cnt);
    lock->open_lock(); //开锁
    sleep(rate); //已进入行驶中
}
void Railway::arrive(int d) {
    int i; //loop index
    lock->close_lock(); //上锁
    (*on_cnt)--;
    printf("[Leaving] D-%d: %d\n", d, getpid());
    printf("    [D: %d] On: %d; N: %d; S:%d\n", d, *on_cnt, *N_waiting, *S_waiting);

    if(*on_cnt == 0) { 
        *pass_cnt = 0; *way_status[0] = spare; 
        printf("[Railway] SPARE!\n");
    }

    int nd, nd_cnt; int d_cnt; //反向及其计数、本向计数
    if(d == NtoS) { nd=StoN, nd_cnt=*S_waiting; d_cnt=*N_waiting; }
    else { nd=NtoS, nd_cnt=*N_waiting; d_cnt=*S_waiting; }

    if(nd_cnt > 0) { //对向有等待车，释放对向车
        for(i=0; i<min(nd_cnt, MAX_1d); i++) { go->Signal(nd); }
    }
    else if(d_cnt > 0) { //本向有等待车，释放本向车
        for(i=0; i<min(d_cnt, MAX_1d); i++) { go->Signal(d); }
    }
    lock->open_lock(); //开锁
}




// 主程序
int main(int argc, char* argv[]) {
    int i;
    int rate=5, max_1d=5, cars=15; //车速，最大单向通过车辆数，预计生成的总车辆数
    if(argc==4) {
        rate = atoi(argv[1]);
        max_1d = atoi(argv[2]);
        cars = atoi(argv[3]);
    }
    int *pids = new int[cars];
    Railway *railway = new Railway(rate, max_1d);

    //fork多个子进程并发模拟执行
    for(i=0; i<cars; i++) {
        pids[i] = fork();
        if(pids[i] < 0) exit(EXIT_FAILURE);
        else if(pids[i] == 0) {
            srand(getpid());
            int d = rand() % 2; //随机生成方向：0-NtoS; 1-StoN
            railway->drivein(d);
            railway->arrive(d);
            exit(EXIT_SUCCESS);
        }
    }
    //等待子进程退出
    for(i=0; i<cars; i++) {
        waitpid(pids[i], NULL, 0);
        printf("[Success] %d\n", i);
    }
    delete railway;

    return EXIT_SUCCESS;
}
