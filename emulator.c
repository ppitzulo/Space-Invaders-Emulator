#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <sys/time.h>


#include "SDL2/SDL.h"
#include "cpu.h"

#define HEIGHT 256
#define WIDTH 224
#define DEBUG 0

SDL_Surface *surface;
SDL_Window *window;
State8080 state;



void getInput(State8080 *state)
{
    SDL_Event event;

    while (SDL_PollEvent(&event))
    {
        if (event.type == SDL_KEYDOWN)
        {
            switch (event.key.keysym.sym)
            {
                case SDLK_c:
                {
                    // Insert coin
                    state->port1 |= 1;
                    break;
                }
                case SDLK_d:
                {
                    // P2 start
                    state->port1 |= 1 << 1;
                    break;
                }
                case SDLK_s:
                {
                    // P1 start
                    state->port1 |= 1 << 2;
                    break;
                }
                case SDLK_f:
                {
                    // Fire
                    state->port1 |= 1 << 4;
                    state->port2 |= 1 << 4;
                    break;
                }
                case SDLK_LEFT:
                {
                    // P1 left
                    state->port1 |= 1 << 5;
                    state->port2 |= 1 << 5;
                    break;
                }
                case SDLK_RIGHT:
                {
                    // P2 right
                    state->port1 |= 1 << 6;
                    state->port2 |= 1 << 6;
                    break;
                }
            }
        }
        else if (event.type == SDL_KEYUP)
        {
            switch (event.key.keysym.sym)
            {
                case SDLK_c:
                {
                    // Insert coin
                    state->port1 &= ~1;
                    break;
                }
                case SDLK_d:
                {
                    // P2 start
                    state->port1 &= ~(1 << 1);
                    break;
                }
                case SDLK_s:
                {
                    // P1 start
                    state->port1 &= ~(1 << 2);
                    break;
                }
                case SDLK_f:
                {
                    // Fire
                    state->port1 &= ~(1 << 4);
                    state->port2 &= ~(1 << 4);
                    break;
                }
                case SDLK_LEFT:
                {
                    // P1 left
                    state->port1 &= ~(1 << 5);
                    state->port2 &= ~(1 << 5);
                    break;
                }
                case SDLK_RIGHT:
                {
                    // P2 right
                    state->port1 &= ~(1 << 6);
                    state->port2 &= ~(1 << 6);
                    break;
                }
            }
        }
        else if (event.window.event == SDL_WINDOWEVENT_CLOSE)
        {
            exit(1);
        }
    }
}

long long time()
{
    struct timeval te;
    gettimeofday(&te, NULL);
    long long milliseconds = te.tv_sec * 1000LL + te.tv_usec / 1000;

    return milliseconds;
}

void draw_video_ram()
{
    
    uint32_t *pix = surface->pixels;


    int i = 0x2400; // Start of Video RAM

    for (int col = 0; col < WIDTH; col++)
    {
        for (int row = HEIGHT-1; row > 0; row -= 8)
        {
            for (int j = 0; j < 8; j++)
            {
                int idx = (row - j) * WIDTH + col;

                if (state.memory[i] & 1 << j)
                {
                    pix[idx] = 0xFFFFFF;
                }
                else
                {
                    pix[idx] = 0x000000;
                }
            }

            i++;
        }
    }


    if (SDL_UpdateWindowSurface(window))
    {
        puts(SDL_GetError());
    }
}

void GenerateInterrupt(State8080 *state, int interrupt_num)
{
    state->sp = state->sp - 2;
    state->memory[state->sp + 1] = (state->pc >> 8) & 0xff;
    state->memory[state->sp] = (state->pc & 0xFF);

    state->pc = interrupt_num;
    state->cc.interrupt_enabled = 0;
}

double getTimeUsec() {
    struct timeval time;
    gettimeofday(&time, NULL);
    return ((double)time.tv_sec *1E6) + ((double)time.tv_usec);
}

int machineOUT(uint16_t port) {
    #if DEBUG
        if (port == 0) {
            return 1;
        }
        else if (port == 1) {
            uint8_t operation = state.c;
            
            if (operation == 2) {
                printf("%c", state.e);
            }
            else if (operation == 9) {
                uint16_t addr = (state.d << 8) | state.e;
                do {
                    printf("%c", state.memory[addr++]);
                } while (state.memory[addr] != '$');
                printf("\n");
            }
        }
    #endif
    switch(port) {
        case 2:
            state.shift_offset = state.a & 0x7;
            break;
        case 3:
            state.outport3 = state.a;
            break;
        case 4:
            state.shift0 = state.shift1;
            state.shift1 = state.a;
            break;
        case 5:
            state.outport5 = state.a;
            break;
        case 6:
            break;
    }
    return 0;
}


int main(int argc, char *argv[])
{
    int lastInterrupt = 1;

    FILE *rom = fopen(argv[1], "rb");

    if (rom == NULL)
    {
        printf("error: Couldn't open %s\n", argv[1]);
        exit(1);
    }
    fseek(rom, 0, SEEK_END);
    int fsize = ftell(rom);
    fseek(rom, 0L, SEEK_SET);
    state.memory = malloc(0x16384);

    if (DEBUG) {
        fread(&state.memory[0x100], sizeof(uint8_t), fsize, rom);
    }
    else {
        fread(state.memory, fsize, 1, rom);
    }
    fclose(rom);
    
    if (DEBUG) {
        state.pc = 0x100;
        state.sp = 0;
        state.memory[0x0] = 0xD3;
        state.memory[0x01] = 0x0;
        state.memory[0x05] = 0xD3;
        state.memory[0x06] = 0x1;
        state.memory[0x07] = 0xC9;
    }

    if (!DEBUG) {
        if (SDL_Init(SDL_INIT_VIDEO))
        {
            printf("%s\n", SDL_GetError());
            exit(1);
        }
        window = SDL_CreateWindow("Space Invaders Emulator",
                                SDL_WINDOWPOS_CENTERED,
                                SDL_WINDOWPOS_CENTERED,
                                WIDTH, HEIGHT,
                                SDL_WINDOW_SHOWN);

        if (!window)
        {
            printf("Failed to create window!");
        }

        surface = SDL_GetWindowSurface(window);
        if (!surface)
        {
            printf("Failed to get window surface!");
        }
    }
    int cycles = 0;

    while(1)
    {
        uint8_t f = 0;

        while (cycles < 17066)
        {   
            f = 0;
            f |= state.cc.s << 7;
            f |= state.cc.z << 6;
            f |= state.cc.ac << 4;
            f |= state.cc.p << 2;
            f |= 1 << 1;
            f |= state.cc.cy << 0;

        //    printf("PC: %04X, A: %04X, AF: %04X, BC: %04X, DE: %04X, HL: %04X, SP: %04X, CYC: %lu",
        //    state.pc, state.a, state.a << 8 | f, (state.b << 8) | state.c, (state.d << 8) | state.e, (state.h << 8) | state.l,
        //    state.sp, cycles);
        //    printf("\t(%02X %02X %02X %02X)", state.memory[state.pc], state.memory[state.pc + 1], state.memory[state.pc + 2], state.memory[state.pc + 3]);
        //    printf("\tsign: %X zero: %X Aux carry: %d parity: %X carry: %X\n", state.cc.s, state.cc.z, state.cc.ac, state.cc.p, state.cc.cy);
            if (state.memory[state.pc] == 0xd3) {
                //OUT
                unsigned char *opcode = &state.memory[state.pc];
                uint8_t port = opcode[1];
                if (machineOUT(port)) {
                    return 0;
                }
                state.pc += 2;
                cycles += 10;
            }
            else {
                cycles += Emulate8080Op(&state);
            }
            
        }
            if (state.cc.interrupt_enabled) {
                if (lastInterrupt) {
                    GenerateInterrupt(&state, 8);
                    lastInterrupt = 0;
                }
                else {
                    GenerateInterrupt(&state, 16);
                    lastInterrupt = 1;
                }
            }
            cycles = 0;
            draw_video_ram();
            getInput(&state);
            SDL_Delay(8);

        cycles = 0;
    }

    return 0;
}
