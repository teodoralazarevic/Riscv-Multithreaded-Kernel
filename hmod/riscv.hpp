//
// Created by os on 5/4/24.
//

#ifndef PROJECT_BASE_RISCV_HPP
#define PROJECT_BASE_RISCV_HPP

#include "tcb.hpp"
#include "bufferc.hpp"

//klasa za upis/citanje iz sistemskih registara
//r_ - citanje iz odgovarajuceg registra
//w_ - upis u odgovarajuci registar
class Sem;
class Bufferc;

class Riscv {
public:
    static Sem* semaphoresHead;
    static Sem* semaphoresTail;
    static TCB* sleepyThreadsHead;
    static TCB* sleepyThreadsTail;
    static TCB* mainThread;

    static Bufferc* putcBuff;
    static Bufferc* getcBuff;

    static bool kernelFin;

    static void pushRegisters();
    static void popRegisters();
    static void supervisorTrap(); //prekidna rutina
    //pop sstatus.spp i sstatus.spie, ne sme da bude inline funkcija
    static void popSppSpie();

    //sepc - tu se cuva vrednost registra PC (adresa instrukcije ecall ili adresa prekinute instrukcije)
    static uint64 r_sepc();
    static void w_sepc(uint64 sepc);

    //sstatus
    //SPP (8.bit)-iz kog rezima se dogodio skok (0-korisnicki, 1-sistemski)
    //SIE (1.bit)-nula ako su maskirani spoljasnji prekidi
    //SPIE (5.bit) - prethodna vrednost registra SIE
    static uint64 r_sstatus();
    static void w_sstatus(uint64 sstatus);
    enum BitMaskSStatus{
        SSTATUS_SPP=(1<<8),
        SSTATUS_SIE=(1<<1),
        SSTATUS_SPIE=(1<<5)
    };
    //postavljanje i brisanje odgovarajucih bitova se radi pomocu maske
    static void set_sstatus(uint64 mask);
    static void clear_sstatus(uint64 mask);

    //scause - da li se dogodio spoljasnji prekid (najvisi bit) i razlog (ostali biti_
    static uint64 r_scause();
    static void w_scause(uint64 scause);

    //stvec - adresa prekidne rutine
    static uint64 r_stvec();
    static void w_stvec(uint64 stvec);

    //sip
    //SSIP (1. bit)-da li postoji zahtev za softverski prekid
    //SEIP (9. bit)-da li postoji zahtev za spoljasnji hardverski prekid
    static uint64 r_sip();
    static void w_sip(uint64 sip);
    enum BitMaskSip{
        SIP_SSIP=(1<<1),
        SIP_SEIP=(1<<9)
    };
    //postavljanje i brisanje odgovarajucih bitova se radi pomocu maske
    static void set_sip(uint64 mask);
    static void clear_sip(uint64 mask);

    //sie - reg za maskiranje prekida
    //SSIE (1. bit) - da li su dozvoljeni softverski prekidi
    //SEIE (9. bit) - da li su dozvoljeni spoljasnji hardverski prekidi
    static uint64 r_sie();
    static void w_sie(uint64 sie);
    enum BitMaskSie{
        SIE_SSIE=(1<<1),
        SIE_SEIE=(1<<9)
    };
    //postavljanje i brisanje odgovarajucih bitova se radi pomocu maske
    static void set_sie(uint64 mask);
    static void clear_sie(uint64 mask);

    static void kernelFinished();
private:
    static void handleSupervisorTrap();
    //static void handleConsoleTrap();
};

inline uint64 Riscv::r_sepc(){
    uint64 volatile sepc;
    __asm__ volatile("csrr %0, sepc" : "=r"(sepc));
    return sepc;
}

inline void Riscv::w_sepc(uint64 sepc) {
    __asm__ volatile("csrw sepc, %0" : : "r"(sepc));
}

inline uint64 Riscv::r_sstatus(){
    uint64 volatile sstatus;
    __asm__ volatile("csrr %0, sstatus" : "=r"(sstatus));
    return sstatus;
}

inline void Riscv::w_sstatus(uint64 sstatus) {
    __asm__ volatile("csrw sstatus, %0" : : "r"(sstatus));
}

inline uint64 Riscv::r_scause(){
    uint64 volatile scause;
    __asm__ volatile("csrr %0, scause" : "=r"(scause));
    return scause;
}

inline void Riscv::w_scause(uint64 scause) {
    __asm__ volatile("csrw scause, %0" : : "r"(scause));
}

inline uint64 Riscv::r_stvec() {
    uint64 volatile stvec;
    __asm__ volatile("csrr %0, stvec" : "=r"(stvec));
    return stvec;
}

inline void Riscv::w_stvec(uint64 stvec) {
    __asm__ volatile("csrw stvec, %0" : : "r"(stvec));
}

inline uint64 Riscv::r_sip() {
    uint64 volatile sip;
    __asm__ volatile("csrr %0, sip" : "=r"(sip));
    return sip;
}

inline void Riscv::w_sip(uint64 sip) {
    __asm__ volatile("csrw sip, %0" : : "r"(sip));
}

inline uint64 Riscv::r_sie() {
    uint64 volatile sie;
    __asm__ volatile("csrr %0, sie" : "=r"(sie));
    return sie;
}

inline void Riscv::w_sie(uint64 sie) {
    __asm__ volatile("csrw sie, %0" : : "r"(sie));
}

inline void Riscv::set_sstatus(uint64 mask) {
    __asm__ volatile("csrs sstatus, %0" : : "r"(mask));
}

inline void Riscv::clear_sstatus(uint64 mask) {
    __asm__ volatile("csrc sstatus, %0" : : "r"(mask));
}

inline void Riscv::set_sip(uint64 mask) {
    __asm__ volatile("csrs sip, %0" : : "r"(mask));
}

inline void Riscv::clear_sip(uint64 mask) {
    __asm__ volatile("csrc sip, %0" : : "r"(mask));
}

inline void Riscv::set_sie(uint64 mask) {
    __asm__ volatile("csrs sie, %0" : : "r"(mask));
}

inline void Riscv::clear_sie(uint64 mask) {
    __asm__ volatile("csrc sie, %0" : : "r"(mask));
}

#endif //PROJECT_BASE_RISCV_HPP
