#include "memory.h"
#include <SDL3/SDL.h>
#include <SDL3_ttf/SDL_ttf.h>
#include <string.h>

void MEMORY_Init(
    MEMORY *memory
)
{
    MEMORY_Reset( memory );
}

void MEMORY_Reset(
    MEMORY *memory
)
{
    memset(
        memory->data,
        0,
        sizeof( memory->data )
    );
}

U8 MEMORY_Read(
    MEMORY *memory,
    U16 address
)
{
    return memory->data[address];
}

void MEMORY_Write(
    MEMORY *memory,
    U16 address,
    U8 value
)
{
    memory->data[address] = value;
}

const U8 *MEMORY_CRT(
    MEMORY *memory
)
{
    return &memory->data[CRT_MEMORY_START];
}

void MEMORY_RenderMap(
    MEMORY *memory,
    SDL_Renderer *renderer,
    SDL_FRect area
)
{
    const int width = 256;
    const int height = 256;

    const float pixelWidth =
        area.w / (float)width;

    const float pixelHeight =
        area.h / (float)height;

    for ( int y = 0; y < height; y++ )
    {
        for ( int x = 0; x < width; x++ )
        {
            const U16 address =
                (U16)((y * width) + x);

            const U8 value =
                memory->data[address];

            if ( value == 0 )
            {
                SDL_SetRenderDrawColor(
                    renderer,
                    0,
                    0,
                    0,
                    255
                );
            }
            else
            {
                /*
                 * Use the actual byte value as brightness.
                 * This makes the map considerably more useful
                 * than simply zero/non-zero.
                 */
                const U8 brightness =
                    value;

                SDL_SetRenderDrawColor(
                    renderer,
                    brightness,
                    brightness,
                    brightness,
                    255
                );
            }

            SDL_FRect pixel =
            {
                area.x + x * pixelWidth,
                area.y + y * pixelHeight,
                pixelWidth,
                pixelHeight
            };

            SDL_RenderFillRect(
                renderer,
                &pixel
            );
        }
    }
}