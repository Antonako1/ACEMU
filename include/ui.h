#ifndef ACEMU_UI_H
#define ACEMU_UI_H

#include "acemu.h"
#include "cpu.h"
#include "memory.h"
#include "crt.h"

#include <SDL3/SDL.h>
#include <SDL3_ttf/SDL_ttf.h>

typedef struct UI
{
    SDL_Window *window;
    SDL_Renderer *renderer;

    TTF_Font *fontTitle;
    TTF_Font *fontHead;
    TTF_Font *fontBody;
    TTF_Font *fontSmall;

    int width;
    int height;

    bool quit;

    CRT crt;

    void *app;
} UI;

bool UI_Init( UI *ui, void *app );

void UI_Shutdown( UI *ui );

void UI_HandleEvents(
    UI *ui,
    CPU *cpu
);

void UI_Render(
    UI *ui,
    const CPU *cpu,
    const MEMORY *memory
);

#endif