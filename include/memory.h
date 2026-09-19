#ifndef ACEMU_MEMORY_H
#define ACEMU_MEMORY_H

#include "acemu.h"

#include <SDL3/SDL.h>

#define MEMORY_SIZE             65536

/*
 * Memory mapped devices.
 */
#define CRT_MEMORY_START       0xC000
#define CRT_MEMORY_END         0xFFFF
#define CRT_MEMORY_SIZE        (CRT_MEMORY_END - CRT_MEMORY_START + 1)

/*
 * CRT framebuffer.
 */
#define CRT_WIDTH              128
#define CRT_HEIGHT             128
#define CRT_PIXELS             (CRT_WIDTH * CRT_HEIGHT)

typedef struct MEMORY
{
    U8 data[MEMORY_SIZE];

} MEMORY;

void MEMORY_Init(
    MEMORY *memory
);

void MEMORY_Reset(
    MEMORY *memory
);

U8 MEMORY_Read(
    MEMORY *memory,
    U16 address
);

void MEMORY_Write(
    MEMORY *memory,
    U16 address,
    U8 value
);

const U8 *MEMORY_CRT(
    MEMORY *memory
);

void MEMORY_RenderMap(
    MEMORY *memory,
    SDL_Renderer *renderer,
    SDL_FRect area
);

#endif