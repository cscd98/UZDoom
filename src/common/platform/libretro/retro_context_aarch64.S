#if defined(__aarch64__)
/* Cooperative switch at a C call boundary. Preserve all AAPCS64 callee saves. */
.text
.balign 16
#ifdef __APPLE__
.global _SwitchContext
.private_extern _SwitchContext
_SwitchContext:
#else
.global SwitchContext
.hidden SwitchContext
.type SwitchContext, %function
SwitchContext:
#endif
    stp x19, x20, [x0, #0]
    stp x21, x22, [x0, #16]
    stp x23, x24, [x0, #32]
    stp x25, x26, [x0, #48]
    stp x27, x28, [x0, #64]
    stp x29, x30, [x0, #80]
    stp d8, d9, [x0, #96]
    stp d10, d11, [x0, #112]
    stp d12, d13, [x0, #128]
    stp d14, d15, [x0, #144]
    mov x2, sp
    str x2, [x0, #160]
    ldp x19, x20, [x1, #0]
    ldp x21, x22, [x1, #16]
    ldp x23, x24, [x1, #32]
    ldp x25, x26, [x1, #48]
    ldp x27, x28, [x1, #64]
    ldp x29, x30, [x1, #80]
    ldp d8, d9, [x1, #96]
    ldp d10, d11, [x1, #112]
    ldp d12, d13, [x1, #128]
    ldp d14, d15, [x1, #144]
    ldr x2, [x1, #160]
    mov sp, x2
    ret
#ifndef __APPLE__
.size SwitchContext, .-SwitchContext
.section .note.GNU-stack,"",%progbits
#endif
#endif
