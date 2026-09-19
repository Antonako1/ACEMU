#include "crt.h"

#include <string.h>
#include <math.h>

#define CRT_PI 3.14159265358979323846f


/*
 * --------------------------------------------------------------------------
 * Helpers
 * --------------------------------------------------------------------------
 */

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


/*
 * Draw a circular outline.
 */
static void CRT_DrawCircle(
    SDL_Renderer *renderer,
    float cx,
    float cy,
    float radius
)
{
    const int segments = 128;

    float previousX =
        cx + cosf( 0.0f ) * radius;

    float previousY =
        cy + sinf( 0.0f ) * radius;

    for ( int i = 1; i <= segments; i++ )
    {
        float angle =
            ((float)i / (float)segments) *
            CRT_PI * 2.0f;

        float x =
            cx + cosf( angle ) * radius;

        float y =
            cy + sinf( angle ) * radius;

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


/*
 * Soft glow around the raster beam.
 */
static void CRT_DrawGlow(
    SDL_Renderer *renderer,
    float cx,
    float cy,
    float radius,
    U8 r,
    U8 g,
    U8 b
)
{
    for ( int i = 4; i >= 1; i-- )
    {
        float t =
            (float)i / 4.0f;

        float size =
            radius * 2.0f * t;

        U8 alpha =
            (U8)(
                8.0f +
                22.0f * (1.0f - t)
                );

        CRT_SetColor(
            renderer,
            r,
            g,
            b,
            alpha
        );

        SDL_FRect rect =
        {
            cx - size * 0.5f,
            cy - size * 0.5f,
            size,
            size
        };

        SDL_RenderFillRect(
            renderer,
            &rect
        );
    }
}


/*
 * Draw the opaque/translucent portions of the circular cover.
 *
 * The rectangle underneath is deliberately NOT clipped.
 *
 * When peek is disabled:
 *
 *     [ CIRCLE ]
 *
 * hides the four corners.
 *
 * When peek is enabled:
 *
 *     [ translucent CIRCLE ]
 *
 * allows the rectangular screen to be seen through the corners.
 */
static void CRT_DrawCoverMask(
    CRT *crt
)
{
    SDL_Renderer *renderer =
        crt->renderer;

    float radius =
        crt->radius;

    float cx =
        crt->x;

    float cy =
        crt->y;

    U8 alpha =
        crt->coverPeek
        ? 55
        : 255;

/*
 * Cover colour.
 *
 * The area outside the circle is drawn here.
 */
    CRT_SetColor(
        renderer,
        7,
        17,
        11,
        alpha
    );

    /*
     * Draw the four rectangular corner regions.
     *
     * These are outside the circular glass.
     */
    for ( int y = -(int)radius;
          y <= (int)radius;
          y++ )
    {
        float fy =
            (float)y;

        float inside =
            radius * radius -
            fy * fy;

        if ( inside < 0.0f )
            continue;

        float width =
            sqrtf( inside );

        float left =
            cx - radius;

        float right =
            cx + radius;

        float circleLeft =
            cx - width;

        float circleRight =
            cx + width;

        float yy =
            cy + fy;

        /*
         * Left side.
         */
        if ( circleLeft >= left )
        {
            SDL_RenderLine(
                renderer,
                left,
                yy,
                circleLeft,
                yy
            );
        }

        /*
         * Right side.
         */
        if ( circleRight <= right )
        {
            SDL_RenderLine(
                renderer,
                circleRight,
                yy,
                right,
                yy
            );
        }
    }
}


/*
 * Draw the glass dome itself.
 *
 * This does not clip the screen.
 * It only overlays subtle translucent layers.
 */
static void CRT_DrawCoverGlass(
    CRT *crt
)
{
    SDL_Renderer *renderer =
        crt->renderer;

    float cx =
        crt->x;

    float cy =
        crt->y;

    float radius =
        crt->radius;

    /*
     * A very subtle dark glass tint.
     *
     * Only draw this when peeking so that the user can
     * actually see the rectangular screen through it.
     */
    if ( crt->coverPeek )
    {
        CRT_SetColor(
            renderer,
            20,
            45,
            30,
            20
        );

        for ( int i = 0; i < 6; i++ )
        {
            float r =
                radius -
                (float)i * 2.0f;

            if ( r <= 0.0f )
                break;

            CRT_DrawCircle(
                renderer,
                cx,
                cy,
                r
            );
        }
    }

    /*
     * Inner glass edge.
     */
    CRT_SetColor(
        renderer,
        70,
        115,
        85,
        45
    );

    CRT_DrawCircle(
        renderer,
        cx,
        cy,
        radius - 2.0f
    );

    /*
     * Dark inner edge.
     */
    CRT_SetColor(
        renderer,
        0,
        5,
        2,
        100
    );

    CRT_DrawCircle(
        renderer,
        cx,
        cy,
        radius - 5.0f
    );


    /*
     * ----------------------------------------------------------------------
     * Specular reflection
     * ----------------------------------------------------------------------
     *
     * A curved highlight on the upper-left portion of the glass.
     */
    for ( int i = 0; i < 3; i++ )
    {
        float reflectionRadius =
            radius -
            8.0f -
            (float)i * 3.0f;

        float startAngle =
            205.0f * CRT_PI / 180.0f;

        float endAngle =
            315.0f * CRT_PI / 180.0f;

        float previousX =
            cx +
            cosf( startAngle ) *
            reflectionRadius;

        float previousY =
            cy +
            sinf( startAngle ) *
            reflectionRadius;

        for ( int j = 1; j <= 40; j++ )
        {
            float t =
                (float)j / 40.0f;

            float angle =
                startAngle +
                (endAngle - startAngle) * t;

            float x =
                cx +
                cosf( angle ) *
                reflectionRadius;

            float y =
                cy +
                sinf( angle ) *
                reflectionRadius;

            U8 alpha =
                (U8)(
                    22.0f -
                    (float)i * 6.0f
                    );

            CRT_SetColor(
                renderer,
                180,
                230,
                195,
                alpha
            );

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


    /*
     * Small bright reflection.
     */
    CRT_SetColor(
        renderer,
        190,
        240,
        205,
        18
    );

    SDL_FRect highlight =
    {
        cx - radius * 0.55f,
        cy - radius * 0.65f,
        radius * 0.45f,
        radius * 0.08f
    };

    SDL_RenderFillRect(
        renderer,
        &highlight
    );
}


/*
 * --------------------------------------------------------------------------
 * Initialisation
 * --------------------------------------------------------------------------
 */

void CRT_Init(
    CRT *crt,
    SDL_Renderer *renderer,
    const U8 *memory
)
{
    memset(
        crt,
        0,
        sizeof( *crt )
    );

    crt->renderer =
        renderer;

    crt->memory =
        memory;

    crt->x =
        360.0f;

    crt->y =
        300.0f;

    crt->radius =
        220.0f;


    /*
     * One complete 128x128 frame should be scanned
     * CRT_REFRESH_HZ times per second.
     */
    crt->pixelsPerCycle =
        (
            (float)CRT_REFRESH_HZ *
            (float)CRT_FRAMEBUFFER_SIZE
            ) /
        (float)CRT_CPU_HZ;


    crt->coverEnabled =
        true;

    crt->coverPeek =
        false;


    /*
     * Start with an empty phosphor.
     */
    CRT_Reset(
        crt
    );
}


/*
 * --------------------------------------------------------------------------
 * Reset
 * --------------------------------------------------------------------------
 */

void CRT_Reset(
    CRT *crt
)
{
    crt->scanPosition =
        0;

    crt->pixelAccumulator =
        0.0f;

    crt->scanning =
        true;

    memset(
        crt->framebuffer,
        0,
        sizeof( crt->framebuffer )
    );
}


/*
 * --------------------------------------------------------------------------
 * Memory
 * --------------------------------------------------------------------------
 */

void CRT_SetMemory(
    CRT *crt,
    const U8 *memory
)
{
    crt->memory =
        memory;
}


/*
 * --------------------------------------------------------------------------
 * Cover controls
 * --------------------------------------------------------------------------
 */

void CRT_SetCover(
    CRT *crt,
    bool enabled
)
{
    crt->coverEnabled =
        enabled;
}


void CRT_SetCoverPeek(
    CRT *crt,
    bool peek
)
{
    crt->coverPeek =
        peek;
}


/*
 * --------------------------------------------------------------------------
 * Raster
 * --------------------------------------------------------------------------
 */

void CRT_StepCycles(
    CRT *crt,
    U32 cycles
)
{
    if ( !crt->scanning )
        return;

    if ( !crt->memory )
        return;


    /*
     * Convert CPU cycles into raster pixels.
     *
     * Fractional pixels are retained between calls so that
     * the average raster frequency remains accurate.
     */
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
         * The electron beam reaches this pixel NOW.
         *
         * Sample CPU-visible VRAM at this exact moment.
         *
         * This is important:
         *
         *     memory[]       = live VRAM
         *     framebuffer[]  = CRT phosphor
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
                crt->scanPosition =
                    0;
            }
    }
}


/*
 * --------------------------------------------------------------------------
 * Rendering
 * --------------------------------------------------------------------------
 */

void CRT_Render(
    CRT *crt
)
{
    if ( !crt )
        return;

    if ( !crt->renderer )
        return;

    if ( !crt->memory )
        return;


    SDL_Renderer *renderer =
        crt->renderer;


    SDL_SetRenderDrawBlendMode(
        renderer,
        SDL_BLENDMODE_BLEND
    );


    const float radius =
        crt->radius;


    /*
     * The framebuffer is ALWAYS rectangular.
     *
     * The circle is merely a physical/visual cover.
     */
    const float screenWidth =
        radius * 2.0f;

    const float screenHeight =
        radius * 2.0f;


    const float pixelWidth =
        screenWidth /
        (float)CRT_WIDTH;

    const float pixelHeight =
        screenHeight /
        (float)CRT_HEIGHT;


    const float left =
        crt->x - radius;

    const float top =
        crt->y - radius;


    /*
     * ----------------------------------------------------------------------
     * CRT outer glow
     * ----------------------------------------------------------------------
     */

    CRT_SetColor(
        renderer,
        20,
        120,
        50,
        12
    );

    SDL_FRect outerGlow =
    {
        crt->x - radius - 30.0f,
        crt->y - radius - 30.0f,
        radius * 2.0f + 60.0f,
        radius * 2.0f + 60.0f
    };

    SDL_RenderFillRect(
        renderer,
        &outerGlow
    );


    /*
     * ----------------------------------------------------------------------
     * Physical rectangular screen
     * ----------------------------------------------------------------------
     */

    CRT_SetColor(
        renderer,
        0,
        5,
        1,
        255
    );

    SDL_FRect screen =
    {
        left,
        top,
        screenWidth,
        screenHeight
    };

    SDL_RenderFillRect(
        renderer,
        &screen
    );


    /*
     * ----------------------------------------------------------------------
     * Phosphor framebuffer
     * ----------------------------------------------------------------------
     *
     * IMPORTANT:
     *
     * Do NOT read crt->memory here.
     *
     * The framebuffer represents what the CRT has already scanned.
     */
    for ( int y = 0; y < CRT_HEIGHT; y++ )
    {
        for ( int x = 0; x < CRT_WIDTH; x++ )
        {
            U32 address =
                (U32)y *
                CRT_WIDTH +
                (U32)x;


            U8 value =
                crt->framebuffer[address];


            if ( value == 0 )
                continue;


            float px =
                left +
                (float)x *
                pixelWidth;

            float py =
                top +
                (float)y *
                pixelHeight;


            U8 green =
                (U8)(
                    40 +
                    (
                        (U32)value *
                        215
                        ) /
                    255
                    );


            /*
                * Phosphor bloom.
                */
            /*CRT_SetColor(
                renderer,
                20,
                green,
                35,
                40
            );

            SDL_FRect glow =
            {
                px - pixelWidth,
                py - pixelHeight,
                pixelWidth,
                pixelHeight
            };

            SDL_RenderFillRect(
                renderer,
                &glow
            );*/


            /*
             * Actual phosphor pixel.
             */
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
                pixelWidth,
                pixelHeight
            };

            SDL_RenderFillRect(
                renderer,
                &pixel
            );
        }
    }


    /*
     * ----------------------------------------------------------------------
     * Scanlines
     * ----------------------------------------------------------------------
     */

    CRT_SetColor(
        renderer,
        0,
        50,
        10,
        35
    );

    for ( int row = 0;
          row < CRT_HEIGHT;
          row++ )
    {
        float y =
            top +
            (
                (float)row +
                0.5f
                ) *
            pixelHeight;

        SDL_RenderLine(
            renderer,
            left,
            y,
            left + screenWidth,
            y
        );
    }


    /*
     * ----------------------------------------------------------------------
     * Raster beam
     * ----------------------------------------------------------------------
     */

    U32 scanPosition =
        crt->scanPosition;


    int scanX =
        (int)(
            scanPosition %
            CRT_WIDTH
            );

    int scanY =
        (int)(
            scanPosition /
            CRT_WIDTH
            );


    float beamX =
        left +
        (
            (float)scanX +
            0.5f
            ) *
        pixelWidth;


    float beamY =
        top +
        (
            (float)scanY +
            0.5f
            ) *
        pixelHeight;


    /*
     * ----------------------------------------------------------------------
     * Beam trail
     * ----------------------------------------------------------------------
     */

    for (
        U32 i = 1;
        i <= CRT_BEAM_TRAIL_LENGTH;
        i++
        )
    {
        U32 trailPosition =
            (
                scanPosition +
                CRT_FRAMEBUFFER_SIZE -
                i
                ) %
            CRT_FRAMEBUFFER_SIZE;


        int trailX =
            (int)(
                trailPosition %
                CRT_WIDTH
                );

        int trailY =
            (int)(
                trailPosition /
                CRT_WIDTH
                );


        float trailBeamX =
            left +
            (
                (float)trailX +
                0.5f
                ) *
            pixelWidth;


        float trailBeamY =
            top +
            (
                (float)trailY +
                0.5f
                ) *
            pixelHeight;


        float fade =
            1.0f -
            (float)i /
            (
                float)(
                    CRT_BEAM_TRAIL_LENGTH +
                    1
                    );


        CRT_SetColor(
            renderer,
            70,
            210,
            120,
            (U8)(
                70.0f *
                fade
                )
        );


        float trailSize =
            pixelWidth * 2.0f;


        SDL_FRect trail =
        {
            trailBeamX -
                trailSize * 0.5f,

            trailBeamY -
                trailSize * 0.5f,

            trailSize,
            trailSize
        };


        SDL_RenderFillRect(
            renderer,
            &trail
        );
    }


    /*
     * Beam glow.
     */
    CRT_DrawGlow(
        renderer,
        beamX,
        beamY,
        pixelWidth * 2.0f,
        70,
        210,
        120
    );


    /*
     * Actual beam.
     */
    CRT_SetColor(
        renderer,
        220,
        255,
        220,
        255
    );


    float beamSize =
        pixelWidth * 2.0f;


    SDL_FRect beam =
    {
        beamX -
            beamSize * 0.5f,

        beamY -
            beamSize * 0.5f,

        beamSize,
        beamSize
    };


    SDL_RenderFillRect(
        renderer,
        &beam
    );


    /*
     * ----------------------------------------------------------------------
     * Circular physical cover
     * ----------------------------------------------------------------------
     *
     * This is intentionally AFTER the framebuffer.
     *
     * The framebuffer remains a complete 128x128 rectangle.
     *
     * The cover simply sits in front of it.
     */
    if ( crt->coverEnabled )
    {
        CRT_DrawCoverMask(
            crt
        );

        CRT_DrawCoverGlass(
            crt
        );
    }


    /*
     * ----------------------------------------------------------------------
     * Outer CRT bezel
     * ----------------------------------------------------------------------
     */

    /*
     * Outer shadow.
     */
    CRT_SetColor(
        renderer,
        0,
        0,
        0,
        100
    );

    CRT_DrawCircle(
        renderer,
        crt->x,
        crt->y,
        radius + 7.0f
    );


    /*
     * Main bezel.
     */
    CRT_SetColor(
        renderer,
        20,
        45,
        28,
        180
    );

    CRT_DrawCircle(
        renderer,
        crt->x,
        crt->y,
        radius + 4.0f
    );


    /*
     * Bright bezel edge.
     */
    CRT_SetColor(
        renderer,
        70,
        110,
        78,
        100
    );

    CRT_DrawCircle(
        renderer,
        crt->x,
        crt->y,
        radius + 2.0f
    );


    /*
     * Inner dark bezel edge.
     */
    CRT_SetColor(
        renderer,
        0,
        10,
        4,
        220
    );

    CRT_DrawCircle(
        renderer,
        crt->x,
        crt->y,
        radius
    );


    SDL_SetRenderDrawBlendMode(
        renderer,
        SDL_BLENDMODE_NONE
    );
}