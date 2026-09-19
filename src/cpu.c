#include "cpu.h"
#include "memory.h"
#include "app.h"

#include <string.h>

void CPU_Init( CPU *cpu )
{
    memset( cpu, 0, sizeof( *cpu ) );

    CPU_Reset( cpu );

    cpu->running = true;
    cpu->A = 0xFB;
}

void CPU_Reset( CPU *cpu )
{
    cpu->A = 0;
    cpu->X = 0;
    cpu->Y = 0;

    cpu->SP = 0xFF;
    cpu->P = 0x00;

    cpu->PC = 0x0000;

    cpu->running = false;
    cpu->cycles = 0;
}

U32 CPU_Step( CPU *cpu )
{
    U32 cycles_consumed = 0;
    U32 previous_cycles = cpu->cycles;
    /*
     * CPU implementation goes here.
     *
     * Eventually:
     *
     * 1. Fetch instruction from memory.
     * 2. Decode instruction.
     * 3. Execute instruction.
     * 4. Update registers.
     * 5. Update flags.
     * 6. Advance PC.
     * 7. Advance cycle counter.
     */

    cpu->cycles++;
    cpu->PC++;
    if ( cpu->PC >= 0xFFFF ) cpu->PC = 0;

    cycles_consumed = cpu->cycles - previous_cycles;
    return cycles_consumed + 1024;
}