//
// Created by os on 5/3/24.
//

#ifndef PROJECT_BASE_SYSCALL_CPP_HPP
#define PROJECT_BASE_SYSCALL_CPP_HPP
#include "syscall_c.hpp"

void* operator new(size_t);
void operator delete(void*);

class Thread{
public:
    Thread(void(*body)(void*), void* arg);
    virtual ~Thread();
    int start();
    static void dispatch();
    static int sleep(time_t);
    void join();
    void timedJoin(time_t);
    static void joinAll();
    static void joinAllTimed(time_t);
    void setPID(uint64 pid);
    static void wait(uint64 pid=0);
    void send(const char* message);
    static char* receive();
    static uint64 getThreadID();
    static uint64 getThreadIDDisp();
    static void setMaxThreads(uint64 num=5);
    bool isFinished() const{return myHandle->isFinished();}
protected:
    Thread();
    virtual void run(){}
private:
    thread_t myHandle;
    static void threadWrapper(void* arg);
    void (*body)(void*);
    void* arg;
};

class Semaphore{
public:
    Semaphore(uint32 init=1);
    virtual ~Semaphore();
    int wait();
    int signal();
    int timedWait(time_t);
    int tryWait();
    void addOwner();
    void removeOwner(uint64 id);
private:
    sem_t myHandle;
};

class PeriodicThread:public Thread{
public:
    void terminate();

protected:
    PeriodicThread(time_t period);
    virtual void periodicActivation(){}
    void run() final;
private:
    time_t period;
};

class Console{
public:
    static char getc();
    static void putc(char);
};

#endif //PROJECT_BASE_SYSCALL_CPP_HPP
