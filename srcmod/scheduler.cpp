//
// Created by os on 5/14/24.
//
#include "../h/scheduler.hpp"
#include "../h/syscall_c.hpp"
#include "../h/riscv.hpp"
//List<TCB>Scheduler::readyCouroutineQueue;
TCB* Scheduler::idleThreadHandle= nullptr;

TCB* Scheduler::headReady= nullptr;
TCB* Scheduler::tailReady= nullptr;

TCB *Scheduler::get() {
    if(headReady== nullptr)
        return idleThreadHandle;
    TCB* newThread=headReady;
    headReady=newThread->nextReadyThread;
    if(headReady==newThread){ //ne ostaje nista u scheduleru
        headReady=tailReady= nullptr;
    }
    newThread->nextReadyThread= nullptr;
    if(tailReady)
        tailReady->nextReadyThread=headReady;
    return newThread;
}

void Scheduler::put(TCB *ccb) {
    if(!Scheduler::headReady)
        headReady=tailReady=ccb->nextReadyThread=ccb;
    else{
        tailReady->nextReadyThread=ccb;
        ccb->nextReadyThread=headReady;
        tailReady=ccb;
    }
    //readyCouroutineQueue.addLast(ccb);
}

void Scheduler::initScheduler() {
    thread_create(&idleThreadHandle, idleThread, nullptr);
}

void Scheduler::idleThread(void*) {
    while(!Riscv::kernelFin){
        thread_dispatch();
    }
}
