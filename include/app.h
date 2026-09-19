#ifndef ACEMU_APP_H
#define ACEMU_APP_H

#include "cpu.h"
#include "memory.h"
#include "ui.h"

typedef struct _APP
{
    SDL_Renderer *ren;

    TTF_Font *fontTitle;
    TTF_Font *fontHead;
    TTF_Font *fontBody;
    TTF_Font *fontSmall;

    bool powered;
    bool running;

    CPU cpu;
    MEMORY memory;
    UI ui;

} APP;

bool APP_Init( APP *app );

void APP_Shutdown( APP *app );

void APP_Run( APP *app );

APP *APP_Get();

#endif