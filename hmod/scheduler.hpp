//
// Created by os on 5/14/24.
//

#ifndef PROJECT_BASE_SCHEDULER_HPP
#define PROJECT_BASE_SCHEDULER_HPP
class TCB;

class Scheduler{
private:
    Scheduler(){}
    static void idleThread(void*);

    //spremne niti (njihovi TCB-ovi) su kruzno uvezani medjusobno
    static TCB* headReady, *tailReady;
public:
    static TCB* get();
    static void put(TCB* ccb);

    static void initScheduler();
    static TCB* idleThreadHandle;
};
#endif //PROJECT_BASE_SCHEDULER_HPP
