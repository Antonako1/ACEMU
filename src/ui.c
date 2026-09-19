#include "ui.h"
#include "app.h"

#include <stdio.h>
#include <string.h>

#define C_BG_R       7
#define C_BG_G       9
#define C_BG_B       14

#define C_PANEL_R    14
#define C_PANEL_G    18
#define C_PANEL_B    28

#define C_BORDER_R   35
#define C_BORDER_G   120
#define C_BORDER_B   75

#define C_GREEN_R    80
#define C_GREEN_G    255
#define C_GREEN_B    105

#define C_GREEN_DIM_R 50
#define C_GREEN_DIM_G 150
#define C_GREEN_DIM_B 75

#define C_WHITE_R    220
#define C_WHITE_G    225
#define C_WHITE_B    220

#define C_GREY_R     125
#define C_GREY_G     135
#define C_GREY_B     130

static void SetColor(
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

static void FillRect(
    SDL_Renderer *renderer,
    SDL_FRect rect,
    U8 r,
    U8 g,
    U8 b
)
{
    SetColor(
        renderer,
        r,
        g,
        b,
        255
    );

    SDL_RenderFillRect(
        renderer,
        &rect
    );
}

static void DrawRect(
    SDL_Renderer *renderer,
    SDL_FRect rect,
    U8 r,
    U8 g,
    U8 b
)
{
    SetColor(
        renderer,
        r,
        g,
        b,
        255
    );

    SDL_RenderRect(
        renderer,
        &rect
    );
}

static TTF_Font *OpenFont(
    float size
)
{
    const char *paths[] =
    {
        "C:\\Windows\\Fonts\\consola.ttf",
        "C:\\Windows\\Fonts\\lucon.ttf",
        "C:\\Windows\\Fonts\\cour.ttf",

        "/usr/share/fonts/truetype/dejavu/DejaVuSansMono.ttf",

        "/System/Library/Fonts/Menlo.ttc"
    };

    for ( size_t i = 0;
          i < sizeof( paths ) / sizeof( paths[0] );
          i++ )
    {
        TTF_Font *font =
            TTF_OpenFont(
                paths[i],
                size
            );

        if ( font )
            return font;
    }

    return NULL;
}

static void DrawText(
    UI *ui,
    TTF_Font *font,
    const char *text,
    float x,
    float y,
    U8 r,
    U8 g,
    U8 b
)
{
    if ( !font )
        return;

    SDL_Color color =
    {
        r, g, b, 255
    };

    SDL_Surface *surface =
        TTF_RenderText_Blended(
            font,
            text,
            0,
            color
        );

    if ( !surface )
        return;

    SDL_Texture *texture =
        SDL_CreateTextureFromSurface(
            ui->renderer,
            surface
        );

    if ( !texture )
    {
        SDL_DestroySurface( surface );
        return;
    }

    SDL_FRect dst =
    {
        x,
        y,
        (float)surface->w,
        (float)surface->h
    };

    SDL_RenderTexture(
        ui->renderer,
        texture,
        NULL,
        &dst
    );

    SDL_DestroyTexture( texture );
    SDL_DestroySurface( surface );
}

static void DrawPanel(
    UI *ui,
    SDL_FRect rect,
    const char *title
)
{
    FillRect(
        ui->renderer,
        rect,
        C_PANEL_R,
        C_PANEL_G,
        C_PANEL_B
    );

    DrawRect(
        ui->renderer,
        rect,
        C_BORDER_R,
        C_BORDER_G,
        C_BORDER_B
    );

    if ( title )
    {
        DrawText(
            ui,
            ui->fontHead,
            title,
            rect.x + 12.0f,
            rect.y + 10.0f,
            C_GREEN_DIM_R,
            C_GREEN_DIM_G,
            C_GREEN_DIM_B
        );
    }
}

static void DrawRegister(
    UI *ui,
    float x,
    float y,
    const char *name,
    const char *value
)
{
    DrawText(
        ui,
        ui->fontBody,
        name,
        x,
        y,
        C_GREY_R,
        C_GREY_G,
        C_GREY_B
    );

    DrawText(
        ui,
        ui->fontBody,
        value,
        x + 105.0f,
        y,
        C_WHITE_R,
        C_WHITE_G,
        C_WHITE_B
    );
}
static void DrawMemoryHexDump(
    UI *ui,
    const MEMORY *memory,
    U16 center,
    float x,
    float y
)
{
    /*
     * Display 8 rows x 16 bytes.
     *
     * The current CPU address is kept near the
     * middle of the dump.
     */
    const int rows = 8;
    const int columns = 16;

    const int centerRow = 3;

    U16 start =
        (U16)(
            center -
            (centerRow * columns)
            );


    DrawText(
        ui,
        ui->fontSmall,
        "HEX MEMORY",
        x,
        y - 25.0f,
        C_GREEN_DIM_R,
        C_GREEN_DIM_G,
        C_GREEN_DIM_B
    );

    for ( int row = 0; row < rows; row++ )
    {
        U16 address =
            (U16)(start + row * columns);

        char line[128];

        int offset = 0;

        offset += snprintf(
            line + offset,
            sizeof( line ) - offset,
            "%04X  ",
            address
        );

        for ( int column = 0;
              column < columns;
              column++ )
        {
            U16 current =
                (U16)(address + column);

            offset += snprintf(
                line + offset,
                sizeof( line ) - offset,
                "%02X ",
                memory->data[current]
            );
        }

        DrawText(
            ui,
            ui->fontSmall,
            line,
            x,
            y + row * 17.0f,
            row == centerRow
            ? C_GREEN_R
            : C_WHITE_R,
            row == centerRow
            ? C_GREEN_G
            : C_WHITE_G,
            row == centerRow
            ? C_GREEN_B
            : C_WHITE_B
        );
    }
}
static void DrawMemoryInfo(
    UI *ui,
    const MEMORY *memory,
    const CPU *cpu,
    SDL_FRect map
)
{
    U32 used = 0;
    U8 minimum = 255;
    U8 maximum = 0;

    for ( U32 i = 0; i < sizeof( memory->data ); i++ )
    {
        U8 value =
            memory->data[i];

        if ( value != 0 )
            used++;

        if ( value < minimum )
            minimum = value;

        if ( value > maximum )
            maximum = value;
    }

    float x =
        map.x + map.w + 25.0f;

    float y =
        map.y;

    char value[64];

    DrawText(
        ui,
        ui->fontSmall,
        "MEMORY STATISTICS",
        x,
        y,
        C_GREEN_DIM_R,
        C_GREEN_DIM_G,
        C_GREEN_DIM_B
    );

    y += 27.0f;

    snprintf(
        value,
        sizeof( value ),
        "%u / 65536 BYTES",
        used
    );

    DrawRegister(
        ui,
        x,
        y,
        "USED",
        value
    );

    y += 25.0f;

    snprintf(
        value,
        sizeof( value ),
        "%u / 65536",
        65536 - used
    );

    DrawRegister(
        ui,
        x,
        y,
        "ZERO",
        value
    );

    y += 25.0f;

    snprintf(
        value,
        sizeof( value ),
        "%02X",
        minimum
    );

    DrawRegister(
        ui,
        x,
        y,
        "MIN",
        value
    );

    y += 25.0f;

    snprintf(
        value,
        sizeof( value ),
        "%02X",
        maximum
    );

    DrawRegister(
        ui,
        x,
        y,
        "MAX",
        value
    );

    y += 25.0f;

    snprintf(
        value,
        sizeof( value ),
        "%04X",
        cpu->PC
    );

    DrawRegister(
        ui,
        x,
        y,
        "PC",
        value
    );

    y += 25.0f;

    snprintf(
        value,
        sizeof( value ),
        "%02X",
        memory->data[cpu->PC]
    );

    DrawRegister(
        ui,
        x,
        y,
        "PC BYTE",
        value
    );

    y += 25.0f;

    snprintf(
        value,
        sizeof( value ),
        "%04X",
        cpu->SP
    );

    DrawRegister(
        ui,
        x,
        y,
        "SP",
        value
    );

    y += 25.0f;

    DrawRegister(
        ui,
        x,
        y,
        "CRT",
        "0000-FFFF"
    );

    y += 25.0f;

    snprintf(
        value,
        sizeof( value ),
        "%u",
        (unsigned)sizeof( memory->data )
    );

    DrawRegister(
        ui,
        x,
        y,
        "CAPACITY",
        value
    );
}
static void DrawCPUInfo(
    UI *ui,
    const CPU *cpu,
    SDL_FRect panel
)
{
    float x = panel.x + 18.0f;
    float y = panel.y + 52.0f;

    DrawText(
        ui,
        ui->fontSmall,
        "8-BIT PROCESSOR",
        x,
        y,
        C_GREEN_R,
        C_GREEN_G,
        C_GREEN_B
    );

    y += 38.0f;

    char value[32];

    snprintf(
        value,
        sizeof( value ),
        "%02X",
        cpu->A
    );

    DrawRegister(
        ui,
        x,
        y,
        "A",
        value
    );

    y += 32.0f;

    snprintf(
        value,
        sizeof( value ),
        "%02X",
        cpu->X
    );

    DrawRegister(
        ui,
        x,
        y,
        "X",
        value
    );

    y += 32.0f;

    snprintf(
        value,
        sizeof( value ),
        "%02X",
        cpu->Y
    );

    DrawRegister(
        ui,
        x,
        y,
        "Y",
        value
    );

    y += 32.0f;

    snprintf(
        value,
        sizeof( value ),
        "%02X",
        cpu->SP
    );

    DrawRegister(
        ui,
        x,
        y,
        "SP",
        value
    );

    y += 32.0f;

    snprintf(
        value,
        sizeof( value ),
        "%04X",
        cpu->PC
    );

    DrawRegister(
        ui,
        x,
        y,
        "PC",
        value
    );

    y += 32.0f;

    snprintf(
        value,
        sizeof( value ),
        "%02X",
        cpu->P
    );

    DrawRegister(
        ui,
        x,
        y,
        "P",
        value
    );

    y += 50.0f;

    DrawText(
        ui,
        ui->fontSmall,
        "MEMORY",
        x,
        y,
        C_GREEN_DIM_R,
        C_GREEN_DIM_G,
        C_GREEN_DIM_B
    );

    y += 30.0f;

    DrawRegister(
        ui,
        x,
        y,
        "SIZE",
        "65536 BYTES"
    );

    y += 32.0f;

    DrawRegister(
        ui,
        x,
        y,
        "ADDRESS",
        "0000-FFFF"
    );

    y += 50.0f;

    DrawText(
        ui,
        ui->fontSmall,
        "CPU STATE",
        x,
        y,
        C_GREEN_DIM_R,
        C_GREEN_DIM_G,
        C_GREEN_DIM_B
    );

    y += 30.0f;

    DrawRegister(
        ui,
        x,
        y,
        "RUNNING",
        cpu->running ? "YES" : "NO"
    );

    y += 32.0f;

    snprintf(
        value,
        sizeof( value ),
        "%llu",
        (unsigned long long)cpu->cycles
    );

    DrawRegister(
        ui,
        x,
        y,
        "CYCLES",
        value
    );
}

bool UI_Init( UI *ui, void *app )
{
    memset(
        ui,
        0,
        sizeof( *ui )
    );

    ui->width = 1280;
    ui->height = 760;

    ui->window =
        SDL_CreateWindow(
            "ACE COMPUTING ENGINE",
            ui->width,
            ui->height,
            SDL_WINDOW_RESIZABLE
        );

    if ( !ui->window )
        return false;

    ui->renderer =
        SDL_CreateRenderer(
            ui->window,
            NULL
        );

    if ( !ui->renderer )
        return false;

    ui->fontTitle = OpenFont( 32.0f );
    ui->fontHead = OpenFont( 19.0f );
    ui->fontBody = OpenFont( 16.0f );
    ui->fontSmall = OpenFont( 13.0f );

    if ( !ui->fontTitle ||
         !ui->fontHead ||
         !ui->fontBody ||
         !ui->fontSmall )
    {
        return false;
    }

    CRT_Init(
        &ui->crt,
        ui->renderer,
        MEMORY_CRT( &((APP *)app)->memory )
    );

    /*
     * CRT position.
     *
     * Left side of the front panel.
     */
    ui->crt.x = 400.0f;
    ui->crt.y = 395.0f;
    ui->crt.radius = 300.0f;

    return true;
}

void UI_Shutdown( UI *ui )
{
    if ( ui->fontTitle )
        TTF_CloseFont( ui->fontTitle );

    if ( ui->fontHead )
        TTF_CloseFont( ui->fontHead );

    if ( ui->fontBody )
        TTF_CloseFont( ui->fontBody );

    if ( ui->fontSmall )
        TTF_CloseFont( ui->fontSmall );

    if ( ui->renderer )
        SDL_DestroyRenderer( ui->renderer );

    if ( ui->window )
        SDL_DestroyWindow( ui->window );
}

void UI_HandleEvents(
    UI *ui,
    CPU *cpu
)
{
    SDL_Event event;

    while ( SDL_PollEvent( &event ) )
    {
        if ( event.type == SDL_EVENT_QUIT )
        {
            ui->quit = true;
        }

        if ( event.type == SDL_EVENT_KEY_DOWN )
        {
            switch ( event.key.key )
            {
                case SDLK_F1:
                    CPU_Reset( cpu );
                    break;

                case SDLK_F5:
                    cpu->running = true;
                    break;

                case SDLK_F6:
                    cpu->running = false;
                    break;

                case SDLK_F10:
                    CPU_Step( cpu );
                    break;

                default:
                    break;
            }
        }
    }
}

void UI_Render(
    UI *ui,
    const CPU *cpu,
    const MEMORY *memory
)
{
    int windowWidth;
    int windowHeight;

    SDL_GetWindowSize(
        ui->window,
        &windowWidth,
        &windowHeight
    );

    SetColor(
        ui->renderer,
        C_BG_R,
        C_BG_G,
        C_BG_B,
        255
    );

    SDL_RenderClear(
        ui->renderer
    );

    /*
     * -------------------------------------------------------
     * HEADER
     * -------------------------------------------------------
     */

    DrawText(
        ui,
        ui->fontTitle,
        "ACE COMPUTING ENGINE",
        30.0f,
        22.0f,
        C_GREEN_R,
        C_GREEN_G,
        C_GREEN_B
    );

    SetColor(
        ui->renderer,
        C_BORDER_R,
        C_BORDER_G,
        C_BORDER_B,
        255
    );

    SDL_RenderLine(
        ui->renderer,
        30.0f,
        66.0f,
        (float)windowWidth - 30.0f,
        66.0f
    );

    /*
     * -------------------------------------------------------
     * MAIN AREA
     * -------------------------------------------------------
     */

    const float outerX = 30.0f;
    const float outerY = 85.0f;
    const float gap = 20.0f;

    const float availableWidth =
        (float)windowWidth -
        outerX * 2.0f -
        gap;

    const float cpuWidth =
        360.0f;

    float displayWidth =
        availableWidth -
        cpuWidth;

    if ( displayWidth < 400.0f )
        displayWidth = 400.0f;

    /*
     * Keep enough room for the memory panel.
     */
    const float memoryHeight = 235.0f;

    float mainHeight =
        (float)windowHeight -
        outerY -
        memoryHeight -
        30.0f;

    if ( mainHeight < 300.0f )
        mainHeight = 300.0f;

    /*
     * -------------------------------------------------------
     * DISPLAY / CRT
     * -------------------------------------------------------
     */

    SDL_FRect crtPanel =
    {
        outerX,
        outerY,
        displayWidth,
        mainHeight
    };

    DrawPanel(
        ui,
        crtPanel,
        "DISPLAY"
    );

    /*
     * Dynamically fit the circular CRT.
     */
    const float crtMarginX = 40.0f;
    const float crtMarginY = 50.0f;

    float maxRadiusX =
        crtPanel.w * 0.5f -
        crtMarginX;

    float maxRadiusY =
        crtPanel.h * 0.5f -
        crtMarginY;

    float radius =
        maxRadiusX < maxRadiusY
        ? maxRadiusX
        : maxRadiusY;

    if ( radius < 1.0f )
        radius = 1.0f;

    ui->crt.x =
        crtPanel.x +
        crtPanel.w * 0.5f;

    ui->crt.y =
        crtPanel.y +
        crtPanel.h * 0.5f +
        15.0f;

    ui->crt.radius =
        radius;

    CRT_Render(
        &ui->crt
    );

    /*
     * -------------------------------------------------------
     * PROCESSOR
     * -------------------------------------------------------
     */

    SDL_FRect cpuPanel =
    {
        outerX +
        displayWidth +
        gap,

        outerY,

        cpuWidth,
        mainHeight
    };

    DrawPanel(
        ui,
        cpuPanel,
        "PROCESSOR"
    );

    DrawCPUInfo(
        ui,
        cpu,
        cpuPanel
    );

    /*
     * -------------------------------------------------------
     * MEMORY
     * -------------------------------------------------------
     */

    float memoryY =
        outerY +
        mainHeight +
        20.0f;

    float memoryBottom =
        (float)windowHeight -
        15.0f;

    float actualMemoryHeight =
        memoryBottom -
        memoryY;

    if ( actualMemoryHeight < 100.0f )
        actualMemoryHeight = 100.0f;

    SDL_FRect memoryPanel =
    {
        outerX,
        memoryY,
        (float)windowWidth - outerX * 2.0f,
        actualMemoryHeight
    };

    DrawPanel(
        ui,
        memoryPanel,
        "MEMORY - 64 KiB"
    );

    /*
     * -------------------------------------------------------
     * MEMORY MAP
     * -------------------------------------------------------
     *
     * Fit the map to the available vertical space.
     */
    float mapSize =
        memoryPanel.h -
        55.0f;

    if ( mapSize > 190.0f )
        mapSize = 190.0f;

    if ( mapSize < 100.0f )
        mapSize = 100.0f;

    SDL_FRect memoryMap =
    {
        memoryPanel.x + 20.0f,
        memoryPanel.y + 35.0f,
        mapSize,
        mapSize
    };

    MEMORY_RenderMap(
        (MEMORY *)memory,
        ui->renderer,
        memoryMap
    );

    /*
     * Map address labels.
     */
    DrawText(
        ui,
        ui->fontSmall,
        "0000",
        memoryMap.x,
        memoryMap.y + memoryMap.h + 3.0f,
        C_GREY_R,
        C_GREY_G,
        C_GREY_B
    );

    DrawText(
        ui,
        ui->fontSmall,
        "FFFF",
        memoryMap.x +
        memoryMap.w -
        32.0f,
        memoryMap.y +
        memoryMap.h +
        3.0f,
        C_GREY_R,
        C_GREY_G,
        C_GREY_B
    );

    /*
     * -------------------------------------------------------
     * MEMORY INFORMATION
     * -------------------------------------------------------
     */

    DrawMemoryInfo(
        ui,
        memory,
        cpu,
        memoryMap
    );

    /*
     * -------------------------------------------------------
     * HEX DUMP
     * -------------------------------------------------------
     */

    DrawMemoryHexDump(
        ui,
        memory,
        cpu->PC,
        memoryPanel.x + 575.0f,
        memoryPanel.y + 60.0f
    );

    SDL_RenderPresent(
        ui->renderer
    );
}