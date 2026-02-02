//
// Created by os on 5/10/24.
//
#include "../h/tcb.hpp"
#include "../h/riscv.hpp"
#include "../lib/console.h"
#include "../test/printing.hpp"
TCB* TCB::running= nullptr;
uint64 TCB::timeSliceCnt=0;
uint64 TCB::id=-1;
Sem* TCB::maxThreadsSem= nullptr;
bool TCB::maxThreadsSet=false;

TCB::TCB(Body body, void* arg, void* stack){
    this->body=body;
    this->arg=arg;
    this->stack=(uint64*)stack;
    this->context={(uint64)threadWrapper, (uint64)&this->stack[DEFAULT_STACK_SIZE]};
    //this->context={(uint64)threadWrapper, (uint64)&this->stack+DEFAULT_STACK_SIZE-1};
    this->finished=false;
    this->timeSlice=DEFAULT_TIME_SLICE;
    this->parent=TCB::running;
    this->childrenCnt=0;
    if(this->parent)
        this->parent->childrenCnt++;
    /*Sem::semOpen(&this->joinSem, 0);
    Sem::semOpen(&this->spaceAvail, 1);
    Sem::semOpen(&this->itemAvail, 0);*/
}

void TCB::dispatch() {
    TCB* old=running;
    old->context.sepc=Riscv::r_sepc();
    old->context.sstatus=Riscv::r_sstatus();
    old->context.scause=Riscv::r_scause();
    //ne ubacujemo blokiranu, uspavanu ili idle nit u Scheduler
    if(!old->isFinished() && old!=Scheduler::idleThreadHandle && !old->nextBlockedThread && !old->nextSleepThread) {
        Scheduler::put(old);
    }
    else if(old->isFinished()){
        MemoryAllocator::memFree(old->stack);
    }
    running=Scheduler::get();
    TCB::contextSwitch(&old->context, &running->context);

    Riscv::w_sepc(running->context.sepc);
    Riscv::w_sstatus(running->context.sstatus);
    Riscv::w_scause(running->context.scause);
}

int TCB::threadCreate(TCB** handle, Body body, void* arg, void* stack_space) {
    *handle=new TCB(body, arg, stack_space);
    if(*handle== nullptr)
        return -1;
    if(body && body!=Scheduler::idleThreadHandle->body) {
        (*handle)->pid=id++;
        if(maxThreadsSet)
            Sem::semWait(maxThreadsSem);
        if(handle)
            Scheduler::put(*handle);
    }
    return 0;
}

void TCB::threadWrapper() {
    //odavde smo dosli iz prekidne rutine (prekid od tajmera) kada je Scheduler za izvrsavanje izabrao
    //nit koja se do sada nije izvrsavala, posto i dalje vazi sve iz PR, treba da
    //pop iz statusnog reg - supervisor previous privilege (SPP)
    //supervisor previous interrupt enable (SPIE)
    Riscv::popSppSpie();
    running->body(running->arg); //ovo izvrsavanje je u korisnickom modu
    thread_exit();
}

int TCB::threadExit() {
    running->setFinished();
    if(TCB::maxThreadsSet) {
        printString("Zavrsila se nit ");
        printInt(running->pid);
        printString("\n");
        Sem::semSignal(maxThreadsSem);
    }
    running->parent->childrenCnt--;
    if(running->parent->isWaiting && running->parent->childrenCnt==0)
        Sem::semSignal(running->parent->joinAllSem);
    //ceka ali ceka bilo koje dete
    if(running->parent->isWaitingChild && running->parent->waitingFor==0){
        running->parent->isWaitingChild=false;
        Sem::semSignal(running->parent->waitSem);
    }
    else if(running->parent->isWaitingChild && running->parent->waitingFor==running->pid){
        running->parent->isWaitingChild=false;
        Sem::semSignal(running->parent->waitSem);
    }
    for(uint64 i=0;i<running->joinCounter;i++)
        Sem::semSignal(running->joinSem);
    running->joinCounter=0;
    //Sem::semClose(running->joinSem);
    TCB::dispatch(); //nit se zavrsila
    return 0;
}

void* TCB::operator new(size_t size){
    size_t newSize= size + sizeof(BlockHeader);
    newSize=newSize%MEM_BLOCK_SIZE?(newSize/MEM_BLOCK_SIZE+1):newSize/MEM_BLOCK_SIZE;
    void* volatile adr=MemoryAllocator::memAlloc(newSize);
    return adr;
}

void* TCB::operator new[](size_t size){
    size_t newSize= size + sizeof(BlockHeader);
    newSize=newSize%MEM_BLOCK_SIZE?(newSize/MEM_BLOCK_SIZE+1):newSize/MEM_BLOCK_SIZE;
    void* volatile adr=MemoryAllocator::memAlloc(newSize);
    return adr;
}

void TCB::operator delete(void* pointer){
    MemoryAllocator::memFree(pointer);
}

void TCB::operator delete[](void* pointer){
    MemoryAllocator::memFree(pointer);
}

int TCB::sleep(time_t time) {
    if(time==0)
        return 0;
    TCB* thr=TCB::running;
    if(!Riscv::sleepyThreadsHead) {
        Riscv::sleepyThreadsHead = Riscv::sleepyThreadsTail=thr;
        thr->relativeSleepTime=time;
        thr->nextSleepThread=thr;
    }
    else{
        //umece se u listu na odgovarajuce mesto
        TCB* curr=Riscv::sleepyThreadsHead, *prev= nullptr;
        if(time < curr->relativeSleepTime){ //ako ova nit treba prva da se probudi
            //dodajemo je na pocetak liste i racunamo vreme spavanja sledecoj niti
            thr->nextSleepThread=curr;
            curr->relativeSleepTime=curr->relativeSleepTime-time;
            thr->relativeSleepTime=time;
            Riscv::sleepyThreadsHead=thr;
            Riscv::sleepyThreadsTail->nextSleepThread=Riscv::sleepyThreadsHead;
        }
        else{
            bool firstPass=true;
            while(firstPass || (curr!=Riscv::sleepyThreadsHead && curr->relativeSleepTime<=time)){
                firstPass=false;
                prev=curr;
                time-=curr->relativeSleepTime;
                curr=curr->nextSleepThread;
            }
            prev->nextSleepThread = thr;
            thr->nextSleepThread = curr;

            thr->relativeSleepTime = time;
            if(thr->nextSleepThread==Riscv::sleepyThreadsHead)
                Riscv::sleepyThreadsTail=thr;
        }
    }
    //nit se ne vraca u spremne, samo se radi dispatch
    TCB::dispatch();
    return 0;
}

void TCB::outputThread(void *) {
    while(!Riscv::kernelFin){
        while((*((char*)(CONSOLE_STATUS)) & CONSOLE_TX_STATUS_BIT) && !Riscv::kernelFin){
            char c = Riscv::putcBuff->getc();
            *((char*)CONSOLE_TX_DATA) = c;
        }
        if(Riscv::kernelFin)
            running->finished=true;
        TCB::dispatch();
    }
}

void TCB::threadJoin(TCB *pTcb) {
    Sem::semOpen(&pTcb->joinSem, 0);
    if(!pTcb->isFinished()){
        ++pTcb->joinCounter;
        Sem::semWait(pTcb->joinSem);
    }
    Sem::semClose(pTcb->joinSem);
}

void TCB::timeJoin(TCB *pTcb, time_t i) {
    Sem::semOpen(&pTcb->joinSem, 0);
    if(i==0 && !pTcb->isFinished()){ //onda obicno cekanje na semaforu
        ++pTcb->joinCounter;
        Sem::semWait(pTcb->joinSem);
    }
    else
        Sem::semTimeWait(pTcb->joinSem, i);
    Sem::semClose(pTcb->joinSem);
}

void TCB::joinAll() {
    Sem::semOpen(&running->joinAllSem, 0);
    running->isWaiting=true;
    Sem::semWait(running->joinAllSem);
    Sem::semClose(running->joinAllSem);
}

void TCB::joinAllTimed(time_t i) {
    Sem::semOpen(&running->joinAllSem, 0);
    if(i==0){
        running->isWaiting=true;
        Sem::semWait(running->joinAllSem);
    }
    else
        Sem::semTimeWait(running->joinAllSem, i);
    Sem::semClose(running->joinAllSem);
}

void TCB::setPid(TCB *pTcb, uint64 i) {
    pTcb->pid=i;
}

void TCB::wait(uint64 pid) {
    running->waitingFor=pid;
    running->isWaitingChild=true;
    Sem::semOpen(&running->waitSem, 0);
    Sem::semWait(running->waitSem);
    Sem::semClose(running->waitSem);
}

void TCB::send(TCB *pTcb, const char *string) {
    Sem::semWait(pTcb->spaceAvail);
    pTcb->mess=string;
    Sem::semSignal(pTcb->itemAvail);
}

const char* TCB::receive() {
    Sem::semWait(running->itemAvail);
    const char* mess=running->mess;
    Sem::semSignal(running->spaceAvail);
    return mess;
}

uint64 TCB::getThreadID() {
    return running->pid;
}

uint64 TCB::getThreadIDDisp() {
    uint64 id=running->pid;
    TCB::dispatch();
    return id;
}

void TCB::setMaxThreads(uint64 num) {
    maxThreadsSet=true;
    Sem::semOpen(&maxThreadsSem, num);
}


