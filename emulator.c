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

SDL_Window *win;
SDL_Renderer *renderer;
SDL_Texture *screentex;
uint8_t screen_buf[256*224*4];
        SDL_Rect dest_rect = {0, 0, 256, 224};


void spaceInvaders_vblank ()
{
	uint16_t vram_base = 0x2400;
	uint32_t *screenPtr = screen_buf;
	int i;

	while (vram_base < 0x4000) {
		uint8_t b = state.memory[vram_base];

		*screenPtr++ = ((b >> 0)&1) ?  0xFFFFFFFF : 0xFF000000;
		*screenPtr++ = ((b >> 1)&1) ?  0xFFFFFFFF : 0xFF000000;
		*screenPtr++ = ((b >> 2)&1) ?  0xFFFFFFFF : 0xFF000000;
		*screenPtr++ = ((b >> 3)&1) ?  0xFFFFFFFF : 0xFF000000;
		*screenPtr++ = ((b >> 4)&1) ?  0xFFFFFFFF : 0xFF000000;
		*screenPtr++ = ((b >> 5)&1) ?  0xFFFFFFFF : 0xFF000000;
		*screenPtr++ = ((b >> 6)&1) ?  0xFFFFFFFF : 0xFF000000;
		*screenPtr++ = ((b >> 7)&1) ?  0xFFFFFFFF : 0xFF000000;

		vram_base += 1;
	}

	SDL_UpdateTexture(screentex, NULL, screen_buf, 256 * 4);
}

void getInput(State8080 *state)
{
    SDL_Event event;

    while (SDL_PollEvent(&event))
    {
        // printf("POLL EVENT\n");
        // if (event.type == SDL_KEYDOWN) {
        //     printf("before setting key\n");
        //     SDL_Scancode key = event.key.keysym.scancode;
        //     if (key == SDL_SCANCODE_C) {
        //         printf("INPUT C");
        //         state->port1 |= 1;
        //     }
        // }
        // else if (event.type == SDL_KEYUP) {
        //     SDL_Scancode key = event.key.keysym.scancode;
        //     if (key == SDL_SCANCODE_C) {
        //         state->port1 &= ~1;
        //     }
        // }
        // printf("Here1\n");
        if (event.type == SDL_KEYDOWN)
        {
            // printf("Here2\n");
            switch (event.key.keysym.sym)
            {
                // printf("Here3\n");
                case SDLK_LEFT:
                {
                    state->port1 |= 0x20;
                    break;
                }
                case SDLK_c:
                {
                    // insert coin
                    state->port1 |= 1;
                    // state->memory[0x20c0] = 0;
                    printf("DEBUG: KEY c %d\n", state->port1);
                    break;
                }
                case SDLK_s:
                {
                    state->port1 |= 1 << 2;
                    printf("DEBUG KEY val of state->port1 %d\n", state->port1);
                    break;
                }
                // case SDLK_c:
                // {
                //     state->port1 |= 1 << 2;
                //     break;
                // }
                case SDLK_d:
                {
                    state->port1 |= 1 << 3;
                    break;
                }
            }
            // break;
        }
        else if (event.type == SDL_KEYUP)
        {
            switch (event.key.keysym.sym)
            {
                case SDLK_LEFT:
                {
                    state->port1 &= 0xDF;
                    break;
                }
                case SDLK_s:
                {
                    state->port1 &= ~(1 << 2);
                    // printf("clearing bit");
                    break;
                }
                case SDLK_c:
                {
                    state->port1 &= ~1;
                    printf("clearing bit c %d\n", state->port1); 
                    break;
                }
                // case SDLK_c:
                // {
                //     state->port1 &= ~(1 << 2);
                //     break;
                // }
                case SDLK_d:
                {
                    state->port1 &= ~(1 << 3);
                    break;
                }
            }
            // break;
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
    // for (int col = 0; col < WIDTH; col++) {
    //     for (int row = 0; row < HEIGHT; row += 8) {
    //         int p;
    //         pix[((row * (256/8)) + col/8)] =;

            // int offgest = (255 - j) * (224 * 4) + (i * 4);
            // unsigned int *p1 = (unsigned int *)(&b[offset]);
            // for (p = 0; p < 8; p++) {
            //     if (0 != (pix & (1 << p))) {
            //         *p1 = 0xffffff;
            //     }
            //     else {
            //         *p1 = 0x000000;
            //     }
            //     p1 -= 224;
    //         }
    //     }
    // }

    for (int col = 0; col < WIDTH; col++)
    {
        for (int row = HEIGHT; row > 0; row -= 8)
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
                    pix[idx] = 0x000000; // Crashes here sometimes yay
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
    // Push PC
    state->sp = state->sp - 2;
    state->memory[state->sp + 1] = (state->pc >> 8) & 0xff;
    state->memory[state->sp] = (state->pc & 0xFF);
    // might need to add this back in
    // cycles += 11;
    // state->pc = interrupt_num;
    state->pc = 8 * interrupt_num;
    state->int_enable = 0;
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
            // printf("DONE\n");
        }
        else if (port == 1) {
            uint8_t operation = state.c;
            
            if (operation == 2) {
                printf("%c", state.e);
            }
            else if (operation == 0) {
                uint16_t addr = (state.d << 8) | state.e;
                do {
                    printf("%c", state.memory[addr++]);
                } while (state.memory[addr] != '$');
            }
        }
    #endif
    switch(port) {
        case 1:
            {
                uint8_t test = state.c;
                if (test == 2) {
                    printf("%c", &state.e);
                    // printf(state.e);
                }
                else if (test == 9) {
                    uint16_t addr = (state.d << 8) | state.e;
                    do {
                        printf("%c", state.memory[addr++]);
                    }
                    while(state.memory[addr] != '$');
                }
                break;
            }
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
    }
    return 0;
}


int main(int argc, char *argv[])
{
    int lastInterrupt = 0;
    int currentInterrupt = 1;
    int nextInterrupt = 0xcf;

    state.pc = 0;

    SDL_Event event;
    FILE *rom = fopen(argv[1], "rb");

    if (rom == NULL)
    {
        printf("error: Couldn't open %s\n", argv[1]);
        exit(1);
    }
    fseek(rom, 0, SEEK_END);
    int fsize = ftell(rom);
    // rewind(rom);
    fseek(rom, 0L, SEEK_SET);
    state.memory = malloc(16384);
    // state.memory = malloc(0x100000);//16384);

    if (DEBUG) {
        fread(&state.memory[0x100], sizeof(uint8_t), fsize, rom);//fsize, 1, rom);
        // fread(&state.memory[0x99], fsize, 1, rom);// + 255, fsize, 1, rom);
    }
    else {
        fread(state.memory, fsize, 1, rom);
    }
    fclose(rom);

    if (DEBUG) {
        state.pc = 0x100;
        state.sp = 0;
        // state.memory[0] = 0xD3;
        // state.memory[1] = 0x0;
        
        // state.memory[0x0] = 0xc3;               
        // state.memory[0x1] = 0x0; 
        // state.memory[0x2] = 0x1;

        // state.memory[0x05] = 0xD3;
        // state.memory[0x06] = 0x01;
        state.memory[0x0] = 0xD3;
        state.memory[0x01] = 0x0;
        state.memory[0x05] = 0xD3;
        state.memory[0x06] = 0x1;
        state.memory[0x07] = 0xC9;
        // state.memory[2] = 0x01;
        // state.memory[0x7] = 0xc9;
        // state.memory[368] = 0x7;

        // state.memory[0x59c] = 0xc3;    
        // state.memory[0x59d] = 0xc2;
        // state.memory[0x59e] = 0x05;
    }

    if (!DEBUG) {
        if (SDL_Init(SDL_INIT_VIDEO))
        {
            printf("%s\n", SDL_GetError());
            exit(1);
        }
        window = SDL_CreateWindow("SDL_CreateTexture",
                                SDL_WINDOWPOS_UNDEFINED,
                                SDL_WINDOWPOS_UNDEFINED,
                                WIDTH, HEIGHT,
                                SDL_WINDOW_RESIZABLE);
        // SDL_CreateWindowAndRenderer(256, 256, SDL_WINDOW_OPENGL, &win, &renderer);
        // screentex = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_STREAMING, 256, 224);

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
    lastInterrupt = time();
    int cycles = 0;
    int counter = 0;
    uint8_t opcode;
    u_int32_t timer = 0;
    double now = getTimeUsec();
    // double nextInterrupt = 0;
    double whichInterrupt = 0;
    double lastTimer = 0.0;

    // while (state.pc < fsize) {
    //     Emulate8080Op(&state);
    //     draw_video_ram();
    // }
    while(1)
    {

    //     if (lastTimer == 0.0) {
    //         lastTimer = now;
    //         nextInterrupt = lastTimer + 16000.0;
    //         whichInterrupt = 1;
    //     }

    //     //graphics interrupts
    // if (state.cc.interrupt_enabled && (now > nextInterrupt))
    // {
    //     if (whichInterrupt == 1)
    //     {
    //         GenerateInterrupt(&state, 1);
    //         whichInterrupt = 2;
    //         //updateWindow();
    //     } 
    //     else
    //     {
    //         GenerateInterrupt(&state, 2);
    //         // cpu.genInterrupt(2);
    //         whichInterrupt = 1;
    //         //updateWindow();
    //     }
    //     nextInterrupt = now + 8000.0;
    // }

    // //ensure that CPU performs ~2M cycles/sec (2MHz)
    // double sinceLast = now - lastTimer;
    // int catchUpCycles = 2 * sinceLast;
    // int cycleCount = 0;

    // while (catchUpCycles > cycleCount)
    // {
    //     cycleCount += Emulate8080Op(&state);
    //          printf("Carry=%d, Parity=%d, Sign=%d, Zero=%d, Cycles=%d, Opcode=%X\n", state.cc.cy, state.cc.p,
    //         state.cc.s, state.cc.z, cycles, opcode);
    //     printf("A $%02x B $%02x C $%02x D $%02x E $%02x H $%02x L $%02x PC %05x SP %04x\n\n",
    //         state.a, state.b, state.c, state.d,
    //         state.e, state.h, state.l, state.pc, state.pc);
    //     // cpu.debugPrint();
    // }
    // draw_video_ram();
    // spaceInvaders_vblank();
    // lastTimer = now;



        // if ((SDL_GetTicks() - timer) > (1000/60)) {
        //     timer = SDL_GetTicks();

        //     getInput(&state);
        //     // handleInput(system.input);

        //     while(cycles < 2000000/60/2) {
        //         cycles += Emulate8080Op(&state);
        //     // printf("Carry=%d, Parity=%d, Sign=%d, Zero=%d, Cycles=%d, Opcode=%X\n", state.cc.cy, state.cc.p,
        //     // state.cc.s, state.cc.z, cycles, opcode);
        //     // printf("A $%02x B $%02x C $%02x D $%02x E $%02x H $%02x L $%02x PC %05x SP %04x\n\n",
        //     // state.a, state.b, state.c, state.d,
        //     // state.e, state.h, state.l, state.pc, state.pc);
        //     }
        //                 // draw_video_ram();
        //                 spaceInvaders_vblank();

        //     GenerateInterrupt(&state, 1);
        //                 // draw_video_ram();
        //                 spaceInvaders_vblank();

        //     while (cycles < 2000000/60) {
        //         cycles += Emulate8080Op(&state);
        // //              printf("Carry=%d, Parity=%d, Sign=%d, Zero=%d, Cycles=%d, Opcode=%X\n", state.cc.cy, state.cc.p,
        // //     state.cc.s, state.cc.z, cycles, opcode);
        // // printf("A $%02x B $%02x C $%02x D $%02x E $%02x H $%02x L $%02x PC %05x SP %04x\n\n",
        // //     state.a, state.b, state.c, state.d,
        // //     state.e, state.h, state.l, state.pc, state.pc);
        //     }

        //     GenerateInterrupt(&state, 2);

        //     cycles = 2000000/60 - cycles;

        //     // draw_video_ram();
        //     spaceInvaders_vblank();
        // }

        uint8_t f = 0;
        // if (time() - lastInterrupt > 1.0 / 60.0)
        //     {
        //         if (state.cc.interrupt_enabled)
        //         {
        //             // printf("INTE");
        //             if (currentInterrupt == 1)
        //             {
        //                 GenerateInterrupt(&state, currentInterrupt);
        //     spaceInvaders_vblank();
        //                 printf("interupt 1");
        //                 currentInterrupt = 2;
        //             }
        //             else
        //             {
        //                 GenerateInterrupt(&state, currentInterrupt);
        //                 printf("interupt 2");
        //                 currentInterrupt = 1;
        //             }
        //             state.cc.interrupt_enabled = 0;
        //             // GenerateInterrupt(&state, currentInterrupt);
        //             lastInterrupt = time(); // check if this is the proper time function
        //             // currentInterrupt = 2;
        //         }
        //         SDL_RenderClear(renderer);
        //         SDL_RenderCopyEx(renderer, screentex, NULL, &dest_rect, 270, NULL, 0);
        //         SDL_RenderPresent(renderer);
        //     }

        // Third option to try
        // while (cycles < 17066)
        while (cycles < 16667)
        // while (cycles < 800000)
        // while (cycles < 16667)//17066)//16667)
        // while (cycles < 4915)
        // while (cycles < 12000)
        {   
            // f = 0;
            // f |= state.cc.s << 7;
            // f |= state.cc.z << 6;
            // f |= state.cc.ac << 4;
            // f |= state.cc.p << 2;
            // f |= 1 << 1;
            // f |= state.cc.cy << 0;
            // uint8_t opcode = state.memory[state.pc];

            // printf("PC: %04X, A: %04X, AF: %04X, BC: %04X, DE: %04X, HL: %04X, SP: %04X, CYC: %lu",
            // state.pc, state.a, state.a << 8 | f, (state.b << 8) | state.c, (state.d << 8) | state.e, (state.h << 8) | state.l,
            // state.sp, cycles);
            // printf("\t(%02X %02X %02X %02X)", state.memory[state.pc], state.memory[state.pc + 1], state.memory[state.pc + 2], state.memory[state.pc + 3]);
            // printf("\tsign: %X zero: %X Aux carry: %d parity: %X carry: %X\n", state.cc.s, state.cc.z, state.cc.ac, state.cc.p, state.cc.cy);

            if (state.memory[state.pc] == 0xd3) {
                //OUT
                unsigned char *opcode = &state.memory[state.pc];
                // uint8_t port = state->memory[state->pc + 1];
                // uint16_t port = state->memory[state->pc];
                uint8_t port = opcode[1];
                if (machineOUT(port)) {
                    return 0;
                }
                state.pc += 2;
                cycles += 10;
            }
            // else if (state.cc.interrupt_enabled) {
            //     uint16_t tmpPC = state.pc;
            //     state.pc = nextInterrupt;
            //     cycles += Emulate8080Op(&state);
            //     state.pc = tmpPC;
            //     state.cc.interrupt_enabled = 0;
            //     if (nextInterrupt == 0xcf) {
            //         nextInterrupt = 0xd7;
            //     }
            //     else {
            //         nextInterrupt = 0xcf;
            //     }
            // }
            else {
                cycles += Emulate8080Op(&state);
            }
            
            }
            cycles = 0;
            GenerateInterrupt(&state, 1);
            // spaceInvaders_vblank()
            draw_video_ram();
            SDL_PollEvent(&event);
            while (cycles < 16667) {
                cycles += Emulate8080Op(&state);
            }

            GenerateInterrupt(&state, 2);
            getInput(&state);
            SDL_Delay(16);


            // SDL_RenderClear(renderer);
            // SDL_RenderCopyEx(renderer, screentex, NULL, &dest_rect, 270, NULL, 0);
            // SDL_RenderPresent(renderer);

        cycles = 0;
    }
    // #endif

    return 0;
}
