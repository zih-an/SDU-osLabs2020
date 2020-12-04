#include "ipc.h"

int main(int argc, char* argv[]) {
    int i; int rate;
    Msg_buf msg_arg;
    //可在在命令⾏第⼀参数指定⼀个进程睡眠秒数，以调解进程执⾏速度
    if (argv[1] != NULL) rate = atoi(argv[1]);
    else rate = 3;

    /* 理发师与消息请求、响应，椅子打交道 */
    // 联系一个账簿消息队列
    account_flg = IPC_CREAT | 0644;
    account_key = 201;
    account_id = set_msq(account_key, account_flg);

    // 联系一个椅子消息队列
    chair_flg = IPC_CREAT | 0644;
    chair_key = 202;
    chair_id = set_msq(chair_key, chair_flg);

    //联系⼀个请求消息队列
    quest_flg = IPC_CREAT | 0644;
    quest_key = 205;
    quest_id = set_msq(quest_key, quest_flg);

    //联系⼀个响应消息队列
    respond_flg = IPC_CREAT | 0644;
    respond_key = 206;
    respond_id = set_msq(respond_key, respond_flg);


    // 主循环
    int SLEEP = 0; int c_mid = 0;
    msg_arg.mid = getpid();
    while(1) {
        if(msgrcv(chair_id, &msg_arg, sizeof(msg_arg), CUSTOMER, IPC_NOWAIT) >= 0) {
            c_mid = msg_arg.mid;  // 记录顾客的进程号
            SLEEP = 0;
            printf("[%d] Barber is cutting!\n", msg_arg.mid);
            sleep(rate);
            
            // 告知顾客已结束，去找人付款
            msg_arg.mtype = c_mid;
            msgsnd(respond_id, &msg_arg, sizeof(msg_arg), 0);
        }
        else if(msgrcv(account_id, &msg_arg, sizeof(msg_arg), CASHQUEST, IPC_NOWAIT) >= 0) {
            c_mid = msg_arg.mid;  // 记录顾客的进程号
            SLEEP = 0;
            // 请求结账
            printf("[%d] Barber quest payment!\n", msg_arg.mid);
            msg_arg.mtype = CASHQUEST;
            msg_arg.mid = getpid();
            msgsnd(quest_id, &msg_arg, sizeof(msg_arg), 0);
            msgrcv(respond_id, &msg_arg, sizeof(msg_arg), msg_arg.mid, 0);  // 阻塞等待
            printf("[%d] Barber finished!\n", msg_arg.mid);

            // 告知顾客可离开
            msg_arg.mtype = c_mid;
            msgsnd(respond_id, &msg_arg, sizeof(msg_arg), 0);

            // 请求调整座位
            msg_arg.mtype = CUSTOMER;
            msg_arg.mid = getpid();
            printf("[%d] Barber quest next customer~\n", msg_arg.mid);
            msgsnd(quest_id, &msg_arg, sizeof(msg_arg), 0);
        }
        else {
            if(!SLEEP) { SLEEP = 1; printf("[%d] Barber sleeps~\n", msg_arg.mid); }
        }
    }



    return EXIT_SUCCESS;
}

