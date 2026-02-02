//
// Created by os on 5/14/24.
//

#ifndef PROJECT_BASE_SEM_HPP
#define PROJECT_BASE_SEM_HPP
#include "list.hpp"
#include "tcb.hpp"
#include "riscv.hpp"

class Sem{
public:
    friend class Riscv;

    static int semOpen(Sem** handle, uint32 init);
    static int semClose(Sem* handle);
    static int semWait(Sem* id);
    static int semSignal(Sem* id);
    static int semTimeWait(Sem* id, time_t timeout);
    static int semTryWait(Sem* id);
    uint64 getVal(){return val;}

    TCB* getBlockedHead();

    void* operator new(size_t size);
    void* operator new[](size_t size);
    void operator delete(void* pointer);
    void operator delete[](void* pointer);

private:
    bool allocated=false;
    uint32 val;

    TCB* blockedHead, *blockedTail; //blokirane niti
    Sem* nextSem;

    void block(time_t blockTime); //blokira tekucu nit
    void unblock(TCB* thread= nullptr);  //odblokira bilo koju nit ili neku odredjenu

    bool hasTimeBlockedThreads(); //da li ima niti koje vremenski cekaju na ovom semaforu

    //medjusobno ulancavamo semafore na kojima imamo vremensko cekanje kako bismo proveravali
    //tu listu semafora u prekindoj rutini i odblokirali niti
    static void linkSemaphore(Sem* semaphore);
    static void unlinkSemaphore(Sem* semaphore); //kada vise nema vremenski blokiranih niti na tom semaforu
    static void addOwner(Sem *pSem);

    bool hasOwner=false;
    List<uint64 > owners;

    static void removeOwner(Sem *pSem, uint64 i);
};


#endif //PROJECT_BASE_SEM_HPP
