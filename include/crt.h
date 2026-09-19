#ifndef ACEMU_CRT_H
#define ACEMU_CRT_H

#include "acemu.h"
#include "memory.h"

#include <SDL3/SDL.h>

#define CRT_CPU_HZ             1000000U
#define CRT_REFRESH_HZ         60U

#define CRT_FRAMEBUFFER_SIZE \
    ((U32)CRT_WIDTH * (U32)CRT_HEIGHT)

/*
 * Length of the glowing trail that follows the raster beam,
 * in framebuffer pixels. Sized to span several scan rows so
 * the trail flows from row to row (vertically) as the beam
 * sweeps down the screen.
 */
#define CRT_BEAM_TRAIL_LENGTH (CRT_WIDTH / 2)

typedef struct CRT
{
    SDL_Renderer *renderer;

    float x;
    float y;
    float radius;

    const U8 *memory;

    U8 framebuffer[CRT_FRAMEBUFFER_SIZE];

    U32 scanPosition;

    float pixelAccumulator;
    float pixelsPerCycle;

    bool scanning;

    /*
     * Physical CRT cover.
     */
    bool coverEnabled;
    bool coverPeek;

    /*
     * Cover appearance.
     */
    float coverDepth;
    float coverHighlight;

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
void CRT_SetCover(
    CRT *crt,
    bool enabled
);

void CRT_SetCoverPeek(
    CRT *crt,
    bool peek
);
#endif