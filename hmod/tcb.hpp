//
// Created by os on 5/10/24.
//

#ifndef PROJECT_BASE_TCB_HPP
#define PROJECT_BASE_TCB_HPP
#include "../lib/hw.h"
#include "syscall_c.hpp"
#include "scheduler.hpp"
#include "../h/memoryAllocator.hpp"

class Sem;
class TCB{
public:
    friend class Riscv;
    bool isFinished()const{return finished;}
    void setFinished(){finished=true;}

    static TCB* running;
    TCB* nextReadyThread= nullptr;

    TCB* nextBlockedThread= nullptr;
    time_t blockTime=-1; //one koje nisu blokirane vremenski ce imati vreme -1

    //nit ce biti uspavana ako ima nextSleepThread i ako je relativeSleepTime>0
    TCB* nextSleepThread= nullptr;
    time_t relativeSleepTime=-1;

    ~TCB(){delete[] stack;}

    using Body = void(*)(void*); //funkcija nad kojom se izvrsava nit

    static int threadCreate(TCB** handle, Body body, void* arg, void* stack_space);
    static int threadExit();
    static void dispatch();
    static int sleep(time_t time);
    static void outputThread(void*);

    uint64 getTimeSlice() const{return timeSlice;}

    void* operator new(size_t size);
    void* operator new[](size_t size);
    void operator delete(void* pointer);
    void operator delete[](void* pointer);

    uint64 joinCounter=0;
    Sem* joinSem;

    TCB* parent; //u cijem kontekstu je napravljena nit
    Sem* joinAllSem;
    uint64 childrenCnt;
    bool isWaiting=false;

    uint64 pid=0;
    bool isWaitingChild=false;
    uint64 waitingFor;
    Sem* waitSem;

    const char* mess= nullptr;
    Sem* spaceAvail;
    Sem* itemAvail;

    static uint64 id;

    static Sem* maxThreadsSem;
    static bool maxThreadsSet;
private:
    TCB(Body body, void* arg, void* stack);
    //cuvamo sp i ra (gde se vracamo nakon sto predamo procesor)
    //registre cuvamo na pocebnom steku, svaka nit ima svoj stek
    struct Context{
        uint64 ra; //x1
        uint64 sp; //x2
        uint64 sepc;
        uint64 sstatus;
        uint64 scause;
    };
    Body body;
    void* arg; //argumenti funkcije nad kojom se nit izvrsava
    uint64* stack; //gde pocinje stek
    Context context;
    bool finished=false; //da li je nit zavrsila
    uint64 timeSlice;

    static uint64 timeSliceCnt; //koliko se tekuca nit izvrsavala na procesoru

    static void contextSwitch(Context* oldContext, Context* runningContext);
    static void threadWrapper();

    static void threadJoin(TCB *pTcb);
    static void timeJoin(TCB *pTcb, time_t i);
    static void joinAll();
    static void joinAllTimed(time_t i);
    static void setPid(TCB *pTcb, uint64 i);
    static void wait(uint64 i);
    static void send(TCB *pTcb, const char *string);
    static const char* receive();

    static uint64 getThreadID();

    static uint64 getThreadIDDisp();

    static void setMaxThreads(uint64 num);
};

#endif //PROJECT_BASE_TCB_HPP