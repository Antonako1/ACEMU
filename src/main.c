#include "app.h"

#include <SDL3/SDL.h>
#include <SDL3_ttf/SDL_ttf.h>

#include <stdio.h>
#include <string.h>

static APP app;

APP *APP_Get()
{
    return &app;
}

int main( void )
{
    if ( !SDL_Init( SDL_INIT_VIDEO ) )
    {
        fprintf(
            stderr,
            "SDL_Init failed: %s\n",
            SDL_GetError()
        );

        return 1;
    }

    if ( !TTF_Init() )
    {
        fprintf(
            stderr,
            "TTF_Init failed: %s\n",
            SDL_GetError()
        );

        SDL_Quit();

        return 1;
    }

    memset( &app, 0, sizeof( APP ) );

    if ( !APP_Init( &app ) )
    {
        fprintf(
            stderr,
            "Failed to initialize ACEMU.\n"
        );

        TTF_Quit();
        SDL_Quit();

        return 1;
    }

    APP_Run( &app );

    APP_Shutdown( &app );

    TTF_Quit();
    SDL_Quit();

    return 0;
}
