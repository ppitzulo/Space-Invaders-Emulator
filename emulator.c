#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include "SDL2/SDL.h"
#include "cpu.h"

#define HEIGHT 256
#define WIDTH 224

SDL_Surface *surface;
SDL_Window *window;
State8080 state;


void draw_video_ram() {
    uint32_t *pix = surface->pixels;

    int i = 0x2400;  // Start of Video RAM
    for (int col = 0; col < WIDTH; col++) {
        for (int row = HEIGHT; row > 0; row -= 8) {
            for (int j = 0; j < 8; j++) {
                int idx = (row - j) * WIDTH + col;

                if (state.memory[i] & 1 << j) {
                    pix[idx] = 0xFFFFFF;
                } else {
                    pix[idx] = 0x000000;
                }
            }

            i++;
        }
    }

    // Update window
    if (SDL_UpdateWindowSurface(window)) {
        puts(SDL_GetError());
    }
}


int main(int argc, char* argv[])
{
    int lastInterrupt = 0;
    FILE *rom = fopen(argv[1], "rb");
    state.pc = 0;
    SDL_Event event;


    if (rom == NULL)
    {
        printf("error: Couldn't open %s\n", argv[1]);
        exit(1);
    }
    fseek(rom, 0L, SEEK_END);
    int fsize = ftell(rom);
    fseek(rom, 0L, SEEK_SET);
    state.memory = malloc(16384);
    fread(state.memory, fsize, 1, rom);
    fclose(rom);

    if (SDL_Init(SDL_INIT_VIDEO)) {
        printf("%s\n", SDL_GetError());
        exit(1);
    }
    window = SDL_CreateWindow("SDL_CreateTexture",
                    SDL_WINDOWPOS_UNDEFINED,
                    SDL_WINDOWPOS_UNDEFINED,
                    WIDTH, HEIGHT,
                    SDL_WINDOW_RESIZABLE);
    if (!window)
    {
        printf("Failed to create window!");
    }

    surface = SDL_GetWindowSurface(window);
    if(!surface)
    {
        printf("Failed to get window surface!");
    }


    while (state.pc < fsize)
    {
        Emulate8080Op(&state);
        draw_video_ram();
        SDL_PollEvent(&event);
        if (event.window.event == SDL_WINDOWEVENT_CLOSE)
        {
            break;
        }
    }

    return 0;
}
