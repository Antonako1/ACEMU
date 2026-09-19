#include "app.h"
#include <stdio.h>

bool APP_Init( APP *app )
{
    CPU_Init( &app->cpu );

    MEMORY_Init( &app->memory );

    if ( !UI_Init( &app->ui, app ) )
        return false;

    CRT_SetMemory(
        &app->ui.crt,
        MEMORY_CRT( &app->memory )
    );

    return true;
}

void APP_Shutdown( APP *app )
{
    UI_Shutdown( &app->ui );
}

void APP_Run( APP *app )
{
    while ( !app->ui.quit )
    {
        static int inc = 1;

        UI_HandleEvents(
            &app->ui,
            &app->cpu
        );

        /*
         * CPU execution.
         *
         * This is deliberately primitive for now.
         * The actual CPU will eventually run at its
         * configured clock rate.
         */

        MEMORY_Write(
            &app->memory,
            (U16)(0xC000 + inc),
            0xFF
        );

        inc = SDL_rand(CRT_FRAMEBUFFER_SIZE + 128);

        if ( inc >= CRT_FRAMEBUFFER_SIZE )
            inc = 0;

        if ( app->cpu.running )
        {
            U32 cycles =
                CPU_Step(
                    &app->cpu
                );

            CRT_StepCycles(
                &app->ui.crt,
                cycles
            );
        }


        UI_Render(
            &app->ui,
            &app->cpu,
            &app->memory
        );

        SDL_Delay( 4 );
    }
}