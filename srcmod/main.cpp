#include "../h/memoryAllocator.hpp"
#include "../h/riscv.hpp"
#include "../h/syscall_cpp.hpp"
#include "../lib/console.h"
#include "../test/printing.hpp"

//inicijalizacija potrebna za pocetak izvrsavanja
void initialization(){
    //Riscv::set_sie(Riscv::SIE_SEIE);
    Riscv::set_sstatus(Riscv::SSTATUS_SIE);//dozvoljavamo prekide
    Riscv::w_stvec((uint64) &Riscv::supervisorTrap);//adresa prekidne rutine

    MemoryAllocator::initMem();
    Scheduler::initScheduler();

    Riscv::putcBuff=new Bufferc();
    Riscv::getcBuff=new Bufferc();

    TCB* outputThread;
    thread_create(&outputThread, TCB::outputThread, nullptr);

    //pokrecemo main nit
    TCB* mainThread;
    thread_create(&mainThread, nullptr, nullptr);
    TCB::running=mainThread;
    Riscv::mainThread=TCB::running;
}


extern void userMain();

void userWrapper(void*){
    userMain();
}

void main(){
    initialization();
    //userMain();
    TCB* userThread;
    thread_create(&userThread, &userWrapper, nullptr);

    while(!userThread->isFinished()){
        thread_dispatch();
    };

    Riscv::kernelFinished();
    /*int* end=(int*)0x100000;
    *end=0x5555;*/
}
