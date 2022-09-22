#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include "SDL2/SDL.h"
#include "disassembler.h"
#include "cpu.h"

#define HEIGHT 256
#define WIDTH 224


SDL_Surface *surf;
SDL_Window *win;
SDL_Surface *winsurf;
int resizef;
State8080 state;

int main(int argc, char* argv[])
{
    FILE *rom = fopen(argv[1], "rb");
    if (rom == NULL)
    {
        printf("error: Couldn't open %s\n", argv[1]);
        exit(1);
    }

    fseek(rom, 0L, SEEK_END);
    int fsize = ftell(rom);
    fseek(rom, 0L, SEEK_SET);
    state.memory = malloc(64000);

    fread(state.memory, fsize, 1, rom);
    fclose(rom);



    if (SDL_Init(SDL_INIT_VIDEO)) {
        printf("%s\n", SDL_GetError());
        exit(1);
    }

    win = SDL_CreateWindow("SDL_CreateTexture",
                    SDL_WINDOWPOS_UNDEFINED,
                    SDL_WINDOWPOS_UNDEFINED,
                    2*WIDTH, 2*HEIGHT,
                    SDL_WINDOW_RESIZABLE);
    if (!win)
    {
        printf("Failed to get window surface!");
    }

    winsurf = SDL_GetWindowSurface(win);
    if (!winsurf)
    {
        printf("Failed to get window surface!");
    }

    surf = SDL_CreateRGBSurface(0, WIDTH, HEIGHT, 32, 0, 0, 0, 0);
    // SDL_Renderer *renderer = SDL_CreateRenderer(window, -1, 0);
    // SDL_Texture *sdlTexture = SDL_CreateTexture(renderer,
    //                            SDL_PIXELFORMAT_ARGB8888,
    //                            SDL_TEXTUREACCESS_STREAMING,
    //                            256, 224);

    state.pc = 0;
    printf("%d", state.pc);
    while (state.pc < fsize)
    {
     Emulate8080Op(&state);
     draw_video_ram();
    }

    return 0;
}
