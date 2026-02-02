//
// Created by os on 5/14/24.
//
#include "../h/sem.hpp"
#include "../lib/console.h"

int Sem::semOpen(Sem **handle, uint32 init) {
    Sem* semaphore=new Sem();
    semaphore->allocated=true;
    if(!semaphore) return -1;
    semaphore->val=init;
    semaphore->blockedHead=semaphore->blockedTail=nullptr;
    semaphore->nextSem= nullptr;
    *handle=semaphore;
    return 0;
}

void Sem::block(time_t blockTime) {
    //da se ne zablokira nit kad treba da ispise sve iz bafera (metoda kernelFinished)
    if(TCB::running==Riscv::mainThread && Riscv::kernelFin)
        return;
    //nit se dodaje u blokirane na tom semaforu
    TCB* thread=TCB::running;
    thread->blockTime=blockTime;
    if(blockTime!=(time_t)-1){ //nit je vremenski blokirana, ovaj semafor se dodaje u listu vremenski blok sem
        linkSemaphore(this);
    }
    if (!blockedHead)
        blockedHead = blockedTail = thread->nextBlockedThread = thread;
    else {
        blockedTail->nextBlockedThread = thread;
        thread->nextBlockedThread = blockedHead;
        blockedTail = thread;
    }
}

//wait na semaforu sa datom ruckom
int Sem::semWait(Sem *id) {
    if(!id) return -1;
    /*if(id->hasOwner && !id->owners.elemInList(&TCB::running->pid))
        return -1;*/
    if(id->val == 0) { //nit se blokira na semaforu, bira se nova za izvrsavanje
        id->block(-1); //vreme je -1
        //bira se druga nit za izvrsavanje
        TCB::dispatch();
        if(!id->allocated) //semafor je dealociran u toku cekanja
            return -1;
    }
    else
        id->val--;
    return 0;
}

int Sem::semTimeWait(Sem* id, time_t timeout) {
    if(!id) return -1;
    if(id->val == 0) { //nit se blokira na semaforu, bira se nova za izvrsavanje
        id->block(timeout);
        //bira se druga nit za izvrsavanje
        TCB::dispatch();
        if(!id->allocated) //semafor je dealociran u toku cekanja
            return -1;
        if(TCB::running->blockTime!=0) { //tajmer ju je odblokirao
            TCB::running->blockTime=-1;
            return -2;
        }
    }
    else
        id->val--;
    return 0;
}


void Sem::unblock(TCB* thread) {
    TCB *newThread;
    if(!thread) { //ako se odblokira bilo koja nit
        if (blockedHead == nullptr)
            return;
        newThread = blockedHead;
        //newThread->blockTime = -1;
        blockedHead = newThread->nextBlockedThread;
        if (blockedHead == newThread) { //nijedna vise nije blokirana
            blockedHead = blockedTail = nullptr;
        }
        if (blockedTail)
            blockedTail->nextBlockedThread = blockedHead;
    }
    else{ //ako zelimo da odblokiramo odredjenu nit
        if (blockedHead == nullptr)
            return;
        newThread = blockedHead;
        TCB* prev= nullptr;
        while(newThread!=thread){ //trazimo nit koju zelimo da odblokiramo
            prev=newThread;
            newThread=newThread->nextBlockedThread;
        }

        if(prev){
            prev->nextBlockedThread=newThread->nextBlockedThread;
            if(newThread==blockedTail)
                blockedTail=prev;
        }
        else{
            blockedTail->nextBlockedThread=newThread->nextBlockedThread;
            blockedHead=newThread->nextBlockedThread;
            if(blockedHead==newThread)
                blockedHead=blockedTail= nullptr;
        }

        if (!hasTimeBlockedThreads()) //ako nema niti koje vremenski cekaju
            unlinkSemaphore(this);
    }
    newThread->nextBlockedThread = nullptr;
    //newThread->blockTime = -1;
    Scheduler::put(newThread);
}


int Sem::semSignal(Sem *id) {
    if(!id) return -1;
    /*if(id->hasOwner && !id->owners.elemInList(&TCB::running->pid))
        return -1;*/
    if(id->blockedHead){ //odblokiramo jednu nit, ako ima blokiranih
        id->unblock();
        TCB::running->blockTime=-1;
        //TCB::dispatch();
    }
    else
        id->val++;
    return 0;
}

int Sem::semClose(Sem *handle) {
    //oslobadja semafor sa datom ruckom, sve niti koje su cekale na semaforu se deblokiraju,
    //njihov wait vraca gresku
    if(!handle) return -1;
    while(handle->blockedHead){
        handle->unblock();
    }
    handle->allocated=false; //semafor je dealociran
    //delete handle;
    return 0;
}

void* Sem::operator new(size_t size){
    size_t newSize= size + sizeof(BlockHeader);
    newSize=newSize%MEM_BLOCK_SIZE?(newSize/MEM_BLOCK_SIZE+1):newSize/MEM_BLOCK_SIZE;
    void* volatile adr=MemoryAllocator::memAlloc(newSize);
    return adr;
}

void* Sem::operator new[](size_t size){
    size_t newSize= size + sizeof(BlockHeader);
    newSize=newSize%MEM_BLOCK_SIZE?(newSize/MEM_BLOCK_SIZE+1):newSize/MEM_BLOCK_SIZE;
    void* volatile adr=MemoryAllocator::memAlloc(newSize);
    return adr;
}

void Sem::operator delete(void* pointer){
    MemoryAllocator::memFree(pointer);
}

void Sem::operator delete[](void* pointer){
    MemoryAllocator::memFree(pointer);
}

void Sem::linkSemaphore(Sem *semaphore) {
    if(Riscv::semaphoresHead== nullptr){ //kruzno ulancavanje
        Riscv::semaphoresHead=Riscv::semaphoresTail=semaphore;
        semaphore->nextSem=semaphore;
    }
    else{
        //ako je ovaj semafor vec ulancan, necemo da ga ulancavamo
        Sem* semFirst=Riscv::semaphoresHead;
        bool firstPass=true;
        while(semFirst!=Riscv::semaphoresTail || firstPass){
            if(semFirst==semaphore)
                return;
            semFirst=semFirst->nextSem;
            firstPass=false;
        }

        //ovaj semafor nije ulancan
        Riscv::semaphoresTail->nextSem=semaphore;
        semaphore->nextSem=Riscv::semaphoresHead;
        Riscv::semaphoresTail=semaphore;
    }
}

void Sem::unlinkSemaphore(Sem *semaphore) {
    if(!Riscv::semaphoresHead)
        return;
    Sem* prev= nullptr, *curr= Riscv::semaphoresHead;
    bool found=false; //da li je pronadjen semafor koji treba izbaciti
    while(!found && prev!=Riscv::semaphoresTail){
        if(curr==semaphore)
            found=true;
        else{
            prev=curr;
            curr=curr->nextSem;
        }
    }
    if(!found) return;
    if(prev){
        prev->nextSem=curr->nextSem;
        if(curr==Riscv::semaphoresTail){
            Riscv::semaphoresTail=prev;
        }
    }
    else{
        Riscv::semaphoresTail->nextSem=curr->nextSem;
        Riscv::semaphoresHead=curr->nextSem;
        if(Riscv::semaphoresHead==semaphore){
            Riscv::semaphoresHead=Riscv::semaphoresTail= nullptr;
        }
    }
    curr->nextSem= nullptr;
}

bool Sem::hasTimeBlockedThreads() {
    TCB* curr=blockedHead;
    bool firstPass=true;
    while(curr && (curr!=blockedHead || firstPass)){
        firstPass=false;
        if(curr->blockTime!=(time_t)-1)
            return true;
    }
    return false;
}

TCB *Sem::getBlockedHead() {
    return blockedHead;
}

int Sem::semTryWait(Sem *id) {
    bool result=(id->val)>0; //ako je val>0 semafor nije zakljucan i vraca se 1
    if((id->val)>0)
        --(id->val);
    return result;
}

void Sem::addOwner(Sem *pSem) {
    pSem->hasOwner=true;
    pSem->owners.addLast(&TCB::running->pid);
}

void Sem::removeOwner(Sem *pSem, uint64 i) {
    pSem->owners.remove(&i);
}
