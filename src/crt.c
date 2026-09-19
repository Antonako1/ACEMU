#include "crt.h"
#include <string.h>
#include <math.h>

#define CRT_PI 3.14159265358979323846f

static void CRT_SetColor(
    SDL_Renderer *renderer,
    U8 r,
    U8 g,
    U8 b,
    U8 a
)
{
    SDL_SetRenderDrawColor(
        renderer,
        r,
        g,
        b,
        a
    );
}

static bool CRT_InCircle(
    CRT *crt,
    float x,
    float y
)
{
    float dx =
        x - crt->x;

    float dy =
        y - crt->y;

    return
        (dx * dx + dy * dy) <=
        (crt->radius * crt->radius);
}


void CRT_Init(
    CRT *crt,
    SDL_Renderer *renderer,
    const U8 *memory
)
{
    crt->renderer = renderer;
    crt->memory = memory;

    crt->x = 360.0f;
    crt->y = 300.0f;
    crt->radius = 220.0f;

    crt->pixelsPerCycle =
        (
            (float)CRT_REFRESH_HZ *
            (float)CRT_FRAMEBUFFER_SIZE
            ) /
        (float)CRT_CPU_HZ;

    CRT_Reset( crt );
}


void CRT_Reset(
    CRT *crt
)
{
    crt->scanPosition = 0;
    crt->pixelAccumulator = 0.0f;
    crt->scanning = true;

    memset(
        crt->framebuffer,
        0,
        sizeof( crt->framebuffer )
    );
}


void CRT_SetMemory(
    CRT *crt,
    const U8 *memory
)
{
    crt->memory = memory;
}


void CRT_StepCycles(
    CRT *crt,
    U32 cycles
)
{
    if ( !crt->scanning )
        return;

    crt->pixelAccumulator +=
        (float)cycles *
        crt->pixelsPerCycle;

    U32 pixels =
        (U32)crt->pixelAccumulator;

    crt->pixelAccumulator -=
        (float)pixels;

    while ( pixels-- )
    {
        /*
         * The electron beam reaches this pixel now.
         *
         * Sample video memory NOW.
         */
        crt->framebuffer[
            crt->scanPosition
        ] =
            crt->memory[
                crt->scanPosition
            ];

            crt->scanPosition++;

            if (
                crt->scanPosition >=
                CRT_FRAMEBUFFER_SIZE
                )
            {
                crt->scanPosition = 0;
            }
    }
}

void CRT_Render(
    CRT *crt
)
{
    if ( !crt->renderer )
        return;

    if ( !crt->memory )
        return;

    SDL_Renderer *renderer =
        crt->renderer;

    const float radius =
        crt->radius;

    /*
     * -------------------------------------------------
     * CRT glow
     * -------------------------------------------------
     */

    SDL_SetRenderDrawBlendMode(
        renderer,
        SDL_BLENDMODE_BLEND
    );

    for ( int i = 8; i > 0; i-- )
    {
        CRT_SetColor(
            renderer,
            0,
            35,
            8,
            10
        );

        float glowRadius =
            radius + i * 4.0f;

        /*
         * Simple circular glow.
         */
        for (
            float y = -glowRadius;
            y <= glowRadius;
            y += 2.0f
            )
        {
            float width =
                sqrtf(
                    glowRadius * glowRadius -
                    y * y
                );

            SDL_RenderLine(
                renderer,
                crt->x - width,
                crt->y + y,
                crt->x + width,
                crt->y + y
            );
        }
    }

    /*
     * -------------------------------------------------
     * CRT glass
     * -------------------------------------------------
     */

    CRT_SetColor(
        renderer,
        0,
        5,
        1,
        250
    );

    for (
        float y = -radius;
        y <= radius;
        y += 1.0f
        )
    {
        float width =
            sqrtf(
                radius * radius -
                y * y
            );

        SDL_RenderLine(
            renderer,
            crt->x - width,
            crt->y + y,
            crt->x + width,
            crt->y + y
        );
    }

    /*
     * -------------------------------------------------
     * Draw framebuffer
     * -------------------------------------------------
     */

    const float pixelSize =
        (radius * 2.0f) / CRT_WIDTH;

    const float left =
        crt->x - radius;

    const float top =
        crt->y - radius;

    for ( int y = 0; y < CRT_HEIGHT; y++ )
    {
        for ( int x = 0; x < CRT_WIDTH; x++ )
        {
            U32 address =
                (U32)y * CRT_WIDTH +
                (U32)x;

            U8 value =
                crt->framebuffer[address];

            if ( value == 0 )
                continue;

            float px =
                left +
                x * pixelSize;

            float py =
                top +
                y * pixelSize;

            float pixelCenterX =
                px + pixelSize * 0.5f;

            float pixelCenterY =
                py + pixelSize * 0.5f;

            float half =
                pixelSize * 0.5f;

            if (
                !CRT_InCircle(
                    crt,
                    pixelCenterX - half,
                    pixelCenterY - half
                ) &&
                !CRT_InCircle(
                    crt,
                    pixelCenterX + half,
                    pixelCenterY - half
                ) &&
                !CRT_InCircle(
                    crt,
                    pixelCenterX - half,
                    pixelCenterY + half
                ) &&
                !CRT_InCircle(
                    crt,
                    pixelCenterX + half,
                    pixelCenterY + half
                )
                )
            {
                continue;
            }

            U8 green =
                (U8)(
                    40 +
                    ((U32)value * 215) / 255
                    );

            CRT_SetColor(
                renderer,
                20,
                green,
                35,
                255
            );

            SDL_FRect pixel =
            {
                px,
                py,
                pixelSize,
                pixelSize
            };

            SDL_RenderFillRect(
                renderer,
                &pixel
            );
        }
    }


/*
 * -------------------------------------------------
 * Warped scanlines
 * -------------------------------------------------
 */

    CRT_SetColor(
        renderer,
        0,
        50,
        10,
        35
    );

    for ( int row = 0; row < CRT_HEIGHT; row++ )
    {
        /*
         * Scanlines are spaced according to the actual
         * framebuffer, not the SDL coordinates.
         */
        float normalizedY =
            ((float)row + 0.5f) /
            (float)CRT_HEIGHT;

        /*
         * Convert framebuffer Y to -1 .. +1.
         */
        float v =
            normalizedY * 2.0f - 1.0f;

        /*
         * CRT barrel/pincushion warp.
         *
         * Positive values push the edges outward.
         */
        float warp =
            1.0f + 0.10f * v * v;

        float warpedY =
            v * radius * warp;

        float y =
            crt->y + warpedY;

        /*
         * Determine the visible width of this warped
         * scanline on the CRT surface.
         */
        float normalizedWarpedY =
            warpedY / radius;

        if ( fabsf( normalizedWarpedY ) > 1.0f )
            continue;

        float width =
            sqrtf(
                radius * radius -
                warpedY * warpedY
            );

        SDL_RenderLine(
            renderer,
            crt->x - width,
            y,
            crt->x + width,
            y
        );
    }



    /*
     * -------------------------------------------------
     * Raster beam
     * -------------------------------------------------
     */
    U32 scanPosition =
        crt->scanPosition;

    int scanX =
        (int)(scanPosition % CRT_WIDTH);

    int scanY =
        (int)(scanPosition / CRT_WIDTH);

    float beamX =
        left +
        ((float)scanX + 0.5f) * pixelSize;

    float beamY =
        top +
        ((float)scanY + 0.5f) * pixelSize;

    if ( CRT_InCircle(
        crt,
        beamX,
        beamY ) )
    {
        /*
         * Bright green electron beam.
         */
        CRT_SetColor(
            renderer,
            120,
            255,
            150,
            255
        );

        float beamSize =
            pixelSize * 0.9f;

        SDL_FRect beam =
        {
            beamX - beamSize * 0.5f,
            beamY - beamSize * 0.5f,
            beamSize,
            beamSize
        };

        SDL_RenderFillRect(
            renderer,
            &beam
        );
    }
    /*
     * -------------------------------------------------
     * CRT border
     * -------------------------------------------------
     */

    CRT_SetColor(
        renderer,
        25,
        130,
        55,
        220
    );

    for ( int i = 0; i < 3; i++ )
    {
        float r =
            radius - (float)i + 4;

        float previousX =
            crt->x + r;

        float previousY =
            crt->y;

        for (
            int angle = 1;
            angle <= 360;
            angle++
            )
        {
            float radians =
                angle * CRT_PI / 180.0f;

            float x =
                crt->x +
                cosf( radians ) * r;

            float y =
                crt->y +
                sinf( radians ) * r;

            SDL_RenderLine(
                renderer,
                previousX,
                previousY,
                x,
                y
            );

            previousX = x;
            previousY = y;
        }
    }

    SDL_SetRenderDrawBlendMode(
        renderer,
        SDL_BLENDMODE_NONE
    );
}