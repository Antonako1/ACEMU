#ifndef ACEMU_CRT_H
#define ACEMU_CRT_H

#include "acemu.h"
#include "memory.h"

#include <SDL3/SDL.h>

#define CRT_CPU_HZ             1000000U
#define CRT_REFRESH_HZ         60U

#define CRT_FRAMEBUFFER_SIZE \
    ((U32)CRT_WIDTH * (U32)CRT_HEIGHT)

typedef struct CRT
{
    SDL_Renderer *renderer;

    float x;
    float y;
    float radius;

    const U8 *memory;

    /*
     * Actual visible CRT phosphor.
     */
    U8 framebuffer[CRT_FRAMEBUFFER_SIZE];

    U32 scanPosition;

    float pixelAccumulator;
    float pixelsPerCycle;

    bool scanning;

} CRT;


void CRT_Init(
    CRT *crt,
    SDL_Renderer *renderer,
    const U8 *memory
);

void CRT_Reset(
    CRT *crt
);

void CRT_SetMemory(
    CRT *crt,
    const U8 *memory
);

void CRT_StepCycles(
    CRT *crt,
    U32 cycles
);

void CRT_Render(
    CRT *crt
);

#endif