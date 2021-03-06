# 铁路单行线问题
## 题意
- 两个城市南北方向之间有一条铁路，多列火车分别从两个城市的车站排队等待进入车道进入对面
- 铁路在同一时间只允许在同一方向上行车
- 构造一个管程，模拟实现两个方向行车，满足以下要求：
    + 可以设置铁路同方向最多行车数
    + 不会撞车
    + 不会出现某一方向火车长时间等待

## 解题思路
- 条件变量
    > 条件变量是管程内的等待机制，每个条件变量表示一种等待原因，对应一个等待队列（即一个信号量）
    + 因为C/C++没有内置“管程、条件变量”的机制，需要靠信号量机制模拟实现
    + 本实验中，将2个条件变量（2个等待队列）合并在一个Condition中实现
        * N_sema：N方向的等待队列
        * S_sema：S方向的等待队列
    + Wait操作：
        * 仅提供阻塞所需要的内容，其余条件在外部判断
            ```c++
            开锁
            //此处因为是用信号量模拟，所以使用 <0（负数）阻塞
            sema->down(); //用if判方向，加入相应等待队列（信号量）
            上锁
            ```
    + Signal操作：
        * 仅提供升高信号量所需的内容，其余在外部判断
            ```c++
            //此处因为是用信号量模拟，所以使用 <=0（非正数） 释放
            sema->up(); //用if判方向，释放相应队列（信号量）
            ```
- 管程类的设计
    + 驶入 drivein(方向)
        ```python
        开锁
        while [条件]:
            正在等待的方向计数++
            Wait()
            正在等待的方向计数--
        路面车计数++
        [该行驶方向路过计数（路面上+已到达）++]
        记录当前路面的行驶方向
        上锁
        sleep(rate) #正在行驶中
        ```
    + 驶出 arrive(方向)
        ```python
        开锁
        路面车计数--
        if 路面无车: #重置
            [过车数 = 0]
            路面状态为空闲
        if 对面有等待车: #对面优先释放
            for i in range([可释放的车数]):  #释放多辆车
                Signal()
        elif 同向有等待车: #考虑同向释放
            for i in range([可释放的车数]):  #释放多辆车
                Signal()
        上锁
        ```
    + 对以上伪代码方框的说明
        * [条件]：不满足阻塞- 
            - 该方向过车数总值<约束 and
            - (路面为空 or
            - (路面车流向相同 and 路面正在行驶的车辆数<约束) )
        * [过车数] == [该行驶方向路过计数（路面上+已到达）]：引入充当红绿灯效果，防止某一方向长时间等待，以便在[条件]中进行判断是否阻塞
        * [可释放的车数]：为保证可并行释放多辆同向等待车（同方向一定数量可并发）而设置
            - 其值 = `min(欲释放方向上等待的车辆数，最高单向并行数)`
    + 根据上述几项操作，设计一个管程类：
        * int rate; //行车速度
        * MAX_1d;  //数量约束，共用
        * *on_cnt; //路面上正在行驶的车辆计数
        * *pass_cnt; //某方向的过车数(到达+正在行驶)
        * *S_waiting; //S方向等待计数
        * *N_waiting; //N方向等待计数
        * *way_status[1]; //当前公路的行驶状态，enum WayStatus
        * block(int d); //阻塞的逻辑表达式
        * Condition *go; //通过的条件变量 -以公路为主角，只有1个
        * Lock *lock; //控制互斥进⼊管程的锁
        > 使用指针形式表达的`*on_cnt,*pass_cnt,*S_waiting,*N_waiting,*way_status[1]`，保证在进程间共享，需要使用set_shm分配为可共享的内存

## 小总结
[关于信号量+管程的参考课程链接：学堂在线-清华大学操作系统](https://www.xuetangx.com/learn/THU08091000267/THU08091000267/4231154/video/6287625)
- 条件变量：每个条件表示一种等待原因，对应一个等待队列
- 管程：是一种类，管理多个进程之间的并发操作，比PV更高级
