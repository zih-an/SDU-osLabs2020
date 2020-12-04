/*
*
* Filename : control.c
* copyright : (C) 2006 by zhonghonglie
* Function : 建⽴并模拟控制者进程
*/
#include "ipc.h"


int main(int argc, char *argv[]) {
    int i; int rate;

    /* 神奇的事情之二：改成全局变量，在堆区声明就正常 */
    //int w_mid;

    int count = MAXVAL;
    Msg_buf msg_arg;
    struct msqid_ds msg_inf;
    //建⽴⼀个共享内存先写⼊⼀串 A 字符模拟要读写的内容
    buff_key = 101;
    buff_num = STRSIZ + 1;
    shm_flg = IPC_CREAT | 0644;
    buff_ptr = (char *)set_shm(buff_key, buff_num, shm_flg);
    for (i = 0; i < STRSIZ; i++) buff_ptr[i] = 'A';
    buff_ptr[i] = '\0';

    //建⽴⼀条请求消息队列
    quest_flg = IPC_CREAT | 0644;
    quest_key = 201;
    quest_id = set_msq(quest_key, quest_flg);

    //建⽴⼀条响应消息队列
    respond_flg = IPC_CREAT | 0644;
    respond_key = 202;
    respond_id = set_msq(respond_key, respond_flg);

    //控制进程准备接收和响应读写者的消息
    printf("Wait quest \n");
    while (1) {
        //当 count ⼤于 0 时说明没有新的读写请求，查询是否有任何新请求
        if (count > 0) {
            quest_flg = IPC_NOWAIT; //以⾮阻塞⽅式接收请求消息
            if (msgrcv(quest_id, &msg_arg, sizeof(msg_arg), FINISHED, quest_flg) >= 0)
            {
                //有读者完成
                count++;
                printf("%d reader finished\n", msg_arg.mid);
            } 
            else if (msgrcv(quest_id, &msg_arg, sizeof(msg_arg), READERQUEST, quest_flg) >= 0) 
            {
                //有读者请求，允许读者读
                count--;
                msg_arg.mtype = msg_arg.mid;
                msgsnd(respond_id, &msg_arg, sizeof(msg_arg), 0);
                printf("%d quest read\n", msg_arg.mid);
            } 
            else if (msgrcv(quest_id, &msg_arg, sizeof(msg_arg), WRITERQUEST, quest_flg) >= 0) 
            {
                //有写者请求
                w_mid = msg_arg.mid;
                count -= MAXVAL;

                //有读者正在读，则等待所有读者读完
                while(count < 0) {
                    //以阻塞⽅式接收读完成消息
                    msgrcv(quest_id, &msg_arg, sizeof(msg_arg), FINISHED, 0);
                    count++;
                    printf("%d reader finish\n", msg_arg.mid);
                }
                //允许写者写
                msg_arg.mtype = w_mid;
                printf("pid: %lld   w_mid: %d\n", msg_arg.mtype, w_mid);
                msgsnd(respond_id, &msg_arg, sizeof(msg_arg), 0);
                printf("%d quest write \n", msg_arg.mid);
            }
        }
        
        //当 count 等于 0 时说明写者正在写，等待写完成
        if (count == 0) {
            //以阻塞⽅式接收消息.
            msgrcv(quest_id, &msg_arg, sizeof(msg_arg), FINISHED, 0);
            count = MAXVAL;
            printf("%d write finished\n", msg_arg.mid);
            if (msgrcv(quest_id, &msg_arg, sizeof(msg_arg), READERQUEST, quest_flg) >= 0) {
                //有读者请求，允许读者读
                count--;
                msg_arg.mtype = msg_arg.mid;
                msgsnd(respond_id, &msg_arg, sizeof(msg_arg), 0);
                printf("%d quest read\n", msg_arg.mid);
            }
        }
    }
    return EXIT_SUCCESS;
}