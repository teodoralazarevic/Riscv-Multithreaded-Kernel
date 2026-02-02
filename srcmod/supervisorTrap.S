.extern _ZN5Riscv20handleSupervisorTrapEv

.align 4 //jer imamo jednu prekidnu rutinu za sve, pa zadnja dva bita registra stvec (mode) moraju da budu 00
//mode=00 - oznaka da se sve obradjuje u jednoj prekidnoj rutini
.global _ZN5Riscv14supervisorTrapEv
.type _ZN5Riscv14supervisorTrapEv, @function
_ZN5Riscv14supervisorTrapEv:
    j ecallHandler
    sret

//odvajamo handler-a jer se za ecall ne treba restaurirati registar a0(x10) jer se tu smesta
//povratna vrednost sistemskog poziva

ecallHandler:
    //cuvanje registara x0-x31 bez x10 (jer je x10 u stvari a0, tu se smesta povratna
    //vrednost funkcije! -> za ecall
    addi sp, sp, -256
    .irp index, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31
    sd x\index, \index*8(sp)
    .endr

    /*csrr t0, sepc //da se ne bismo vracali na adresu ecall
    addi t0, t0, 4
    csrw sepc, t0*/

    call _ZN5Riscv20handleSupervisorTrapEv //poziva se handleSupervisorTrap

    csrr t0, scause
    xor t0, t0, 9 //ako je scause=9 (ecall), onda ovaj xor daje 0
    beqz t0, ecallInterrupt

    csrr t0, scause
    xor t0, t0, 8
    beqz t0, ecallInterrupt

    //inace restauriramo vrednosti svih registara
    .irp index, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31
    ld x\index, \index*8(sp)
    .endr
    addi sp, sp, 256
    sret
ecallInterrupt:
    //restauracija registara sa steka x0-x31 (sve sem r0, tu je rezultat)
    .irp index, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31
    ld x\index, \index*8(sp)
    .endr
    addi sp, sp, 256

    //automatski upisuje staru vrednost pc-a u pc (iz registra sepc)
    //prelazi se u rezim rada definisan sa SPP
    //bit SIE dobija vrednost SPIE
    sret