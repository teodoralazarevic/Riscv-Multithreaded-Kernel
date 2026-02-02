//
// Created by os on 5/4/24.
//

#include "../h/riscv.hpp"
#include "../h/syscall_c.hpp"
#include "../h/tcb.hpp"
#include "../h/sem.hpp"
#include "../test/printing.hpp"
#include "../lib/console.h"

Sem* Riscv::semaphoresHead= nullptr;
Sem* Riscv::semaphoresTail= nullptr;
TCB* Riscv::sleepyThreadsHead= nullptr;
TCB* Riscv::sleepyThreadsTail= nullptr;
TCB* Riscv::mainThread= nullptr;
Bufferc* Riscv::getcBuff= nullptr;
Bufferc* Riscv::putcBuff= nullptr;
bool Riscv::kernelFin=false;

//ecall prekid iz sistemskog rezima
void Riscv::handleSupervisorTrap() {
    uint64 volatile scause=r_scause(); //citanje scause registra
    uint64 volatile code, arg1, arg2, arg3, arg4;
    __asm__ volatile("mv %0, a0" : "=r"(code)); //citamo a0 reg u prom code
    __asm__ volatile("mv %0, a1" : "=r"(arg1)); //citamo argumente
    __asm__ volatile("mv %0, a2" : "=r"(arg2));
    __asm__ volatile("mv %0, a3" : "=r"(arg3));
    __asm__ volatile("mv %0, a4" : "=r"(arg4));
    if(scause==0x8000000000000001UL) { //softverski prekid (prekid od tajmera
        TCB::timeSliceCnt++;

        //budimo niti koje cekaju na semaforu
        bool firstPass=true;
        Sem* currSem=semaphoresHead;
        while(currSem && (currSem!=semaphoresHead || firstPass)){
            firstPass=false;
            TCB* blockedThreadHead=currSem->getBlockedHead(); //blokirane niti na tom semaforu
            TCB* currTCB=blockedThreadHead;
            //umanjujemo svakoj vreme cekanja i ako je 0 odblokiramo je
            bool firstPass2=true;
            while(currTCB && (currTCB!=blockedThreadHead || firstPass2)){
                firstPass2=false;
                currTCB->blockTime--;
                if(currTCB->blockTime==0)
                    currSem->unblock(currTCB);
                currTCB=currTCB->nextBlockedThread;
            }
            currSem=currSem->nextSem;
        }

        //ako se dostiglo predvidjeno vreme izvrsavanja na procesoru
        if(TCB::timeSliceCnt>=TCB::running->getTimeSlice()) {
            TCB::timeSliceCnt=0;
            TCB::dispatch(); //promena konteksta
        }

        //budimo uspavane niti
        TCB* sleepThreads=sleepyThreadsHead;
        if(sleepThreads){
            sleepThreads->relativeSleepTime--;
            while(sleepThreads && sleepThreads->relativeSleepTime==0){
                //sleepThreads=sleepThreads->nextSleepThread;
                TCB* nextThread=sleepThreads->nextSleepThread;
                sleepThreads->nextSleepThread= nullptr;
                Scheduler::put(sleepThreads);
                if(sleepThreads==nextThread){
                    sleepThreads = nullptr;
                    sleepyThreadsHead=sleepyThreadsTail= nullptr; //DODATO
                }
                else {
                    sleepThreads = nextThread;
                    sleepyThreadsHead = sleepThreads; //DODATO
                    sleepyThreadsTail->nextSleepThread = sleepyThreadsHead; //DODATO
                }
            }
            if(!sleepyThreadsHead)
                sleepyThreadsTail= nullptr;
        }

        clear_sip(SIP_SSIP); //obradili smo zahtev za softverski prekid, vise nije pending
    }
    else if(scause==0x8000000000000009UL){ //spoljasnji hardverski prekid (prekid od konzole)
        /*console_handler();
        clear_sip(SIP_SEIP);*/

        int numOfReq=plic_claim();
        while(*((char*)(CONSOLE_STATUS))&CONSOLE_RX_STATUS_BIT){
            char ch=(*(char*)CONSOLE_RX_DATA);
            if(ch=='\r')
                getcBuff->putc('\n');
            else
                getcBuff->putc(ch);
        }
        plic_complete(numOfReq);
        clear_sip(SIP_SEIP);
    }
    else if(scause==0x0000000000000009UL || scause==0x0000000000000008UL){ //ecall iz sistemskog rezima
        uint64 volatile sepc=r_sepc();
        sepc+=4; //da se ne vratimo na ecall opet nego na sledecu instrukciju
        w_sepc(sepc);
        switch (code) {
            case MEM_ALLOC: {
                MemoryAllocator::memAlloc(arg1);
                break;
            }
            case MEM_FREE: {
                MemoryAllocator::memFree((void *) arg1);
                break;
            }
            case THREAD_CREATE: {
                TCB::threadCreate((TCB **) arg1, (TCB::Body) arg2, (void *) arg3, (void *) arg4);
                break;
            }
            case THREAD_EXIT: {
                TCB::threadExit();
                break;
            }
            case THREAD_DISPATCH: {//dispatch
                TCB::timeSliceCnt = 0;
                TCB::dispatch(); //promena konteksta
                break;
            }
            case JOIN:{
                TCB::threadJoin((TCB*) arg1);
                break;
            }
            case JOIN_TIME:{
                TCB::timeJoin((TCB*)arg1, (time_t)arg2);
                break;
            }
            case JOIN_ALL:{
                TCB::joinAll();
                break;
            }
            case JOIN_ALL_TIMED:{
                TCB::joinAllTimed((time_t)arg1);
                break;
            }
            case SET_PID:{
                TCB::setPid((TCB*)arg1, (uint64)arg2);
                break;
            }
            case WAIT:{
                TCB::wait((uint64)arg1);
                break;
            }
            case SEND:{
                TCB::send((TCB*)arg1, (char*)arg2);
                break;
            }
            case RECEIVE:{
                TCB::receive();
                break;
            }
            case GET_THREAD_ID:{
                TCB::getThreadID();
                break;
            }
            case GET_THREAD_ID_DISP:{
                TCB::getThreadIDDisp();
                break;
            }
            case SET_MAX_THREADS:{
                TCB::setMaxThreads((uint64)arg1);
                break;
            }
            case ADD_OWNER:{
                Sem::addOwner((Sem*)arg1);
                break;
            }
            case REMOVE_OWNER:{
                Sem::removeOwner((Sem*)arg1, (uint64)arg2);
                break;
            }
            case SEM_OPEN: {
                Sem::semOpen((Sem **) arg1, (uint32) arg2);
                break;
            }
            case SEM_CLOSE: {
                Sem::semClose((Sem *) arg1);
                break;
            }
            case SEM_WAIT: {
                Sem::semWait((Sem *) arg1);
                break;
            }
            case SEM_TIMEDWAIT:{
                Sem::semTimeWait((Sem*) arg1, (time_t)arg2);
                break;
            }
            case SEM_TRYWAIT:{
                Sem::semTryWait((Sem*) arg1);
                break;
            }
            case SEM_SIGNAL: {
                Sem::semSignal((Sem *) arg1);
                break;
            }
            case TIME_SLEEP:{
                TCB::sleep((uint64)arg1);
                break;
            }
            case GETC:{
                char* ret= nullptr;
                __asm__ volatile("mv t2, %0" : : "r"(arg1));
                __asm__ volatile("mv %0, t2" : "=r"(ret));
                *ret=getcBuff->getc();

                /*char* ret= nullptr;
                __asm__ volatile("mv t2, %0" : : "r"(arg1));
                __asm__ volatile("mv %0, t2" : "=r"(ret));
                uint64 sepc=r_sepc();
                *ret=__getc();
                w_sepc(sepc);*/
                break;
            }
            case PUTC:{
                putcBuff->putc((char)arg1);
                //__putc((char)arg1);
                break;
            }
        }
    }
    else{ //ako nije nista odozgo, isprintovati sve potrebno
        printInt(scause);
        printString("\n");
    }
}

void Riscv::popSppSpie() {
    if(TCB::running->body!=TCB::outputThread){
        Riscv::clear_sstatus(SSTATUS_SPP); //prelazak u korisnicke
        Riscv::set_sstatus(SSTATUS_SPIE);
    }
    //Riscv::clear_sstatus(SSTATUS_SPP);
    __asm__ volatile("csrw sepc, ra"); //vracamo se tamo odakle se pozvalo popSppSpie
    __asm__ volatile("sret"); //izlazak iz privilegovanog rezima
}

void Riscv::kernelFinished() {
    Riscv::kernelFin=true;
    while(putcBuff->getSize()>0){
        char c = Riscv::putcBuff->getc();
        if(putcBuff->getSize()>0)
            *((char*)CONSOLE_TX_DATA) = c;
    }
}