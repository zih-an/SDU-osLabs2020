#include "ipc.h"

int main(int argc, char* argv[]) {
    int i; int rate;
    Msg_buf msg_arg;
    //可在在命令⾏第⼀参数指定⼀个进程睡眠秒数，以调解进程执⾏速度
    if (argv[1] != NULL) rate = atoi(argv[1]);
    else rate = 3;

    /* 顾客只与结账、消息请求、响应打交道 */
    // 联系一个账簿消息队列
    account_flg = IPC_CREAT | 0644;
    account_key = 201;
    account_id = set_msq(account_key, account_flg);

    //联系⼀个请求消息队列
    quest_flg = IPC_CREAT | 0644;
    quest_key = 205;
    quest_id = set_msq(quest_key, quest_flg);

    //联系⼀个响应消息队列
    respond_flg = IPC_CREAT | 0644;
    respond_key = 206;
    respond_id = set_msq(respond_key, respond_flg);

    
    // 请求进店
    msg_arg.mid = getpid();
    msg_arg.mtype = ENTERQUEST;
    msgsnd(quest_id, &msg_arg, sizeof(msg_arg), 0);
    printf("[%d] Customer quest...\n", msg_arg.mid);
    // 等待反馈消息（离店或入座）
    msgrcv(respond_id, &msg_arg, sizeof(msg_arg), msg_arg.mid, 0);
    if(msg_arg.mid == 0) {
        printf("[%d] Customer exit!\n", msg_arg.mid);
        return EXIT_SUCCESS;  // 离开
    }

    msg_arg.mid = getpid();
    printf("[%d] Customer sit!\n", msg_arg.mid);
    msgrcv(respond_id, &msg_arg, sizeof(msg_arg), msg_arg.mid, 0);
    printf("[%d] Customer FINISHED cutting!\n", msg_arg.mid);

    // 付款
    msg_arg.mid = getpid();
    msg_arg.mtype = CASHQUEST;
    msgsnd(account_id, &msg_arg, sizeof(msg_arg), 0);
    msgrcv(respond_id, &msg_arg, sizeof(msg_arg), msg_arg.mid, 0);
    printf("[%d] Customer FINISHED!\n", msg_arg.mid);

    return EXIT_SUCCESS;
}

