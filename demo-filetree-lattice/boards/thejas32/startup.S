/*
 * Minimal RISC-V reset vector for the THEJAS32 reference board.
 *
 * TODO: verify the trap vector, interrupt controller and FPU setup against the
 * actual port. This file only establishes a stack and the C runtime; it does
 * not enable caches, PMP, or the FPU.
 */

    .section .text.init, "ax", @progbits
    .globl _start
    .type _start, @function

_start:
    /* Stack grows down from the top of RAM. */
    la      sp, _estack

    /* Copy .data from flash to RAM. */
    la      t0, __data_load_start
    la      t1, __data_start
    la      t2, __data_end

1:
    bge     t1, t2, 2f
    lw      t3, 0(t0)
    sw      t3, 0(t1)
    addi    t0, t0, 4
    addi    t1, t1, 4
    j       1b

2:
    /* Zero .bss. */
    la      t0, __bss_start
    la      t1, __bss_end

3:
    bge     t0, t1, 4f
    sw      zero, 0(t0)
    addi    t0, t0, 4
    j       3b

4:
    /* Disable interrupts until the port and board are ready. */
    csrw    mie, zero
    call    main

5:
    j       5b

    .size _start, . - _start
