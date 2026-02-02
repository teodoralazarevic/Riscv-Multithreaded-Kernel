//a0-&old->context, a1-&running->context
.global _ZN3TCB13contextSwitchEPNS_7ContextES1_
.type _ZN3TCB13contextSwitchEPNS_7ContextES1_, @function
_ZN3TCB13contextSwitchEPNS_7ContextES1_:
    sd ra, 0*8(a0)
    sd sp, 1*8(a0)
    /*sd sepc, 2*8(a0)
    sd sstatus, 3*8(a0)*/

    ld ra, 0*8(a1)
    ld sp, 1*8(a1)
    /*ld sepc, 2*8(a1)
    ld sstatus, 3*8(a1)*/
    ret
