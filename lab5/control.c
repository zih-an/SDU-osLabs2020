#include "ipc.h"


int main(int argc, char* argv[]) {
    int i; int rate;
    Msg_buf msg_arg;

    /* 账簿、椅子、沙发、等候室、请求和响应 */
    // 联系一个椅子消息队列
    chair_flg = IPC_CREAT | 0644;
    chair_key = 202;
    chair_id = set_msq(chair_key, chair_flg);
    
    // 联系一个椅子消息队列
    chair_flg = IPC_CREAT | 0644;
    chair_key = 202;
    chair_id = set_msq(chair_key, chair_flg);

    // 联系一个沙发消息队列
    sofa_flg = IPC_CREAT | 0644;
    sofa_key = 203;
    sofa_id = set_msq(sofa_key, sofa_flg);

    // 联系一个等候室消息队列
    wroom_flg = IPC_CREAT | 0644;
    wroom_key = 204;
    wroom_id = set_msq(wroom_key, wroom_flg);

    //联系⼀个请求消息队列
    quest_flg = IPC_CREAT | 0644;
    quest_key = 205;
    quest_id = set_msq(quest_key, quest_flg);

    //联系⼀个响应消息队列
    respond_flg = IPC_CREAT | 0644;
    respond_key = 206;
    respond_id = set_msq(respond_key, respond_flg);


    // 主循环
    printf("Wait qest!\n");
    int c_mid = 0;
    while(1) {
        // 响应顾客的入店请求
        if(msgrcv(quest_id, &msg_arg, sizeof(msg_arg), ENTERQUEST, IPC_NOWAIT) >= 0) {
            if(chair_cnt + sofa_cnt + wroom_cnt < 20) {
                c_mid = msg_arg.mid;
                /* 先挪位置 */
                // 沙发到椅子
                while(chair_cnt<3 && sofa_cnt!=0) {
                    msgrcv(sofa_id, &msg_arg, sizeof(msg_arg), CUSTOMER, 0);
                    msgsnd(chair_id, &msg_arg, sizeof(msg_arg), 0);
                    chair_cnt++;
                    sofa_cnt--;
                }
                // 等候室到沙发
                while(sofa_cnt<4 && wroom_cnt!=0) {
                    msgrcv(wroom_id, &msg_arg, sizeof(msg_arg), CUSTOMER, 0);
                    msgsnd(sofa_id, &msg_arg, sizeof(msg_arg), 0);
                    sofa_cnt++;
                    wroom_cnt--;
                }

                // 放在合适的队列
                msg_arg.mtype = CUSTOMER;
                msg_arg.mid = c_mid;
                if(chair_cnt < 3) {
                    msgsnd(chair_id, &msg_arg, sizeof(msg_arg), 0);
                    chair_cnt++;
                }
                else if(sofa_cnt < 4) {
                    msgsnd(sofa_id, &msg_arg, sizeof(msg_arg), 0);
                    sofa_cnt++;
                }
                else if(wroom_cnt < 13) {
                    msgsnd(wroom_id, &msg_arg, sizeof(msg_arg), 0);
                    wroom_cnt++;
                }
                printf("[%d] Customer come in~~\n", c_mid);
                // 提示入店
                msg_arg.mtype = c_mid;
                msg_arg.mid = 1;
                msgsnd(respond_id, &msg_arg, sizeof(msg_arg), 0);
            }
            else {
                // 离店
                printf("[%d] Customer leaves...\n", msg_arg.mid);
                msg_arg.mtype = msg_arg.mid;
                msg_arg.mid = 0;
                msgsnd(respond_id, &msg_arg, sizeof(msg_arg), 0);
            }
        }
        // 响应理发师的结账请求
        else if(msgrcv(quest_id, &msg_arg, sizeof(msg_arg), CASHQUEST, IPC_NOWAIT) >= 0) {
            sleep(3); // 结账
            printf("[%d] Pay!\n", msg_arg.mid);
            chair_cnt--;
            msg_arg.mtype = msg_arg.mid;
            msgsnd(respond_id, &msg_arg, sizeof(msg_arg), 0);
        }
        // 响应理发师的请下一个顾客的请求（挪位置）
        else if(msgrcv(quest_id, &msg_arg, sizeof(msg_arg), CUSTOMER, IPC_NOWAIT) >= 0) {
            printf("[%d] Move the next customer!\n", msg_arg.mid);
            // 沙发到椅子
            while(chair_cnt<3 && sofa_cnt!=0) {
                msgrcv(sofa_id, &msg_arg, sizeof(msg_arg), CUSTOMER, 0);
                msgsnd(chair_id, &msg_arg, sizeof(msg_arg), 0);
                chair_cnt++;
                sofa_cnt--;
            }
            // 等候室到沙发
            while(sofa_cnt<4 && wroom_cnt!=0) {
                msgrcv(wroom_id, &msg_arg, sizeof(msg_arg), CUSTOMER, 0);
                msgsnd(sofa_id, &msg_arg, sizeof(msg_arg), 0);
                sofa_cnt++;
                wroom_cnt--;
            }
            printf("[CNTS] c: %d  s: %d  w: %d\n", chair_cnt, sofa_cnt, wroom_cnt);
        }
        
    }


    return EXIT_SUCCESS;
}