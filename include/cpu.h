#ifndef ACEMU_CPU_H
#define ACEMU_CPU_H

#include "acemu.h"
#include "crt.h"

#define CPU_CYCLES_PER_FRAME \
    (CRT_CPU_HZ / CRT_REFRESH_HZ)

typedef struct CPU
{
    /* 8-bit registers */
    U8 A;
    U8 X;
    U8 Y;

    U8 SP;
    U8 P;

    /* 16-bit program counter */
    U16 PC;

    /* CPU state */
    bool running;

    U64 cycles;

} CPU;

void CPU_Init( CPU *cpu );
void CPU_Reset( CPU *cpu );

U32 CPU_Step( CPU *cpu );

#endif