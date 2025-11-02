#include <raylib.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint-gcc.h>
#include <time.h>
#include <unistd.h>
#include <sys/stat.h>
#include "arguments.h"
#include "raylib.h"


uint8_t delay = 0x00;
uint8_t sound = 0x00;
uint8_t display[32][64];
uint8_t ram[4096];
uint8_t v[16];
uint16_t I;
uint16_t stack[17]; // Actual stack is 16, but we have length of 17 so we can point to very bottom when nothing is on stack.
uint16_t sp = 0x00;
uint16_t pc = 0x200;
int exit_loop = 0;
int hz = 300;
int keys[16] = {KEY_X, KEY_ONE, KEY_TWO, KEY_THREE, KEY_Q, KEY_W, KEY_E, KEY_A, KEY_S, KEY_D, KEY_Z, KEY_C, KEY_FOUR, KEY_R, KEY_F, KEY_V};
int size_multiplier;

char font[16][5] = 
{
    {0xF0, 0x90, 0x90, 0x90, 0xF0},
    {0x20, 0x60, 0x20, 0x20, 0x70},
    {0xF0, 0x10, 0xF0, 0x80, 0xF0},
    {0xF0, 0x10, 0xF0, 0x10, 0xF0},
    {0x90, 0x90, 0xF0, 0x10, 0x10},
    {0xF0, 0x80, 0xF0, 0x10, 0xF0},
    {0xF0, 0x80, 0xF0, 0x90, 0xF0},
    {0xF0, 0x10, 0x20, 0x40, 0x40},
    {0xF0, 0x90, 0xF0, 0x90, 0xF0},
    {0xF0, 0x90, 0xF0, 0x10, 0xF0},
    {0xF0, 0x90, 0xF0, 0x90, 0x90},
    {0xE0, 0x90, 0xE0, 0x90, 0xE0},
    {0xF0, 0x80, 0x80, 0x80, 0xF0},
    {0xE0, 0x90, 0x90, 0x90, 0xE0},
    {0xF0, 0x80, 0xF0, 0x80, 0xF0},
    {0xF0, 0x80, 0xF0, 0x80, 0x80}
};

void call_instruction()
{
    
    uint16_t op = (ram[pc] << 8) + ram[pc + 1];

    // First, we do the opcodes that always have the same opcode. 
    switch (op)
    {
        // CLS
        // Clears Screen
        case 0x00E0:
            for (int i = 0; i < 64; i++)
            {
                for (int ii = 0; ii < 32; ii++)
                {
                    display[ii][i] = 0;
                }
            }
            pc += 2;
            return;

        // RET
        // Return from a subroutine
        case 0x00EE:
            if (sp == 0)
            {
                printf("chip8emu: Returning from subroutine when SP is 0\n");
                exit(1);
            } else {
                pc = stack[sp];
                sp--;
            }
            pc += 2;
            return;
    }

    if (op >> 12 == 0x0)
    {
        pc += 2;
        return;
    }

    // Now, the opcodes with Xnnn
    switch (op >> 12)
    {
        // JP nnn
        // Jump to location nnn.
        case 0x1:
            pc = op & 0x0FFF;
            return;

        // CALL nnn
        // Call subroutine at nnn
        case 0x2:
            sp++;
            stack[sp] = pc;
            pc = op & 0x0FFF;
            return;

        // SE Vx, byte
        // Skip next instruction if Vx == byte
        case 0x3:
            if ((v[(op >> 8) & 0xF]) == (op & 0xFF))
            {
                pc += 2;
            }

            pc += 2;
            return;

        // SNE Vx, byte
        // Skip next instruction if Vx != byte
        case 0x4:
            if ((v[(op >> 8) & 0xF]) != (op & 0xFF))
            {
                pc += 2;
            }

            pc += 2;
            return;

        // SE Vx, Vy
        // Skip next instruction if Vx == Vy
        case 0x5:
            if (v[(op & 0xF00) >> 8] == v[(op & 0xF0) >> 4])
            {
                pc += 2;
            }

            pc += 2;
            return;

        // LD Vx, byte
        // Load byte into Vx    
        case 0x6:
            v[(op & 0x0F00) >> 8] = op & 0xFF;
            pc += 2;
            return;

        // ADD Vx, byte
        // Add byte into Vx    
        case 0x7:
            v[(op & 0x0F00) >> 8] += op & 0xFF;
            pc += 2;
            return;

         
        case 0x8:
            switch (op & 0xF)
            {
                // LD Vx, Vy
                // Load value of Vy into Vx   
                case 0x0:
                    v[(op & 0x0F00) >> 8] = v[(op & 0x00F0) >> 4];
                    pc += 2;
                    break;

                // OR Vx, Vy
                // Binary ors Vx with Vy
                case 0x1:
                    v[(op & 0x0F00) >> 8] = v[(op & 0x0F00) >> 8] | v[(op & 0x00F0) >> 4];
                    pc += 2;
                    break;

                // AND Vx, Vy
                // Binary ands Vx with Vy
                case 0x2:
                    v[(op & 0x0F00) >> 8] = v[(op & 0x0F00) >> 8] & v[(op & 0x00F0) >> 4];
                    pc += 2;
                    break;

                // XOR Vx, Vy
                // Binary xors Vx with Vy
                case 0x3:
                    v[(op & 0x0F00) >> 8] = v[(op & 0x0F00) >> 8] ^ v[(op & 0x00F0) >> 4];
                    pc += 2;
                    break;

                // ADD Vx, Vy
                // Adds Vy to Vx, also set VF to 1 if Vx + Vy > 255
                case 0x4:
                    if (v[(op & 0x0F00) >> 8] + v[(op & 0x00F0) >> 4] > 255)
                    {
                        v[0xF] = 1;
                    } else {
                        v[0xF] = 0;
                    }

                    v[(op & 0x0F00) >> 8] = (v[(op & 0x0F00) >> 8] + v[(op & 0x00F0) >> 4]) & 0xFF;
                    pc += 2;
                    break;

                // SUB Vx, Vy
                // Subtracts Vy from Vx, also set VF to 1 if Vx > Vy
                case 0x5:
                    if (v[(op & 0x0F00) >> 8] > v[(op & 0x00F0) >> 4])
                    {
                        v[0xF] = 1;
                    } else {
                        v[0xF] = 0;
                    }

                    v[(op & 0x0F00) >> 8] = v[(op & 0x0F00) >> 8] - v[(op & 0x00F0) >> 4];
                    pc += 2;
                    break;

                // SHR Vx{, Vy}
                // Shifts Vx right by one, and takes the least signifigant bit and moves it to the Vf register
                case 0x6:
                    
                    v[0xF] = v[(op & 0x0F00) >> 8] & 0x1;
                    

                    v[(op & 0x0F00) >> 8] = v[(op & 0x0F00) >> 8] >> 1;
                    pc += 2;
                    break;

                // SUB Vx, Vy
                // Subtracts Vx from Vy, also set VF to 1 if Vx > Vy
                case 0x7:
                    if (v[(op & 0x00F0) >> 4] > v[(op & 0x0F00) >> 8])
                    {
                        v[0xF] = 1;
                    } else {
                        v[0xF] = 0;
                    }

                    v[(op & 0x0F00) >> 8] = v[(op & 0x00F0) >> 4] - v[(op & 0x0F00) >> 8];
                    pc += 2;
                    break;

                // SHL Vx{, Vy}
                // Shifts Vx left by one, and takes the most signifigant bit and moves it to the Vf register
                case 0xE:
                    v[0xF] = (v[(op & 0x0F00) >> 8] & 0b10000000) >> 7 ;
                    v[(op & 0x0F00) >> 8] = v[(op & 0x0F00) >> 8] << 1;
                    pc += 2;
                    break;
            }
            return;
            
        // SNE Vx, Vy
        // Skip next instruction if Vx != Vy
        case 0x9:
            if (v[(op & 0xF00) >> 8] != v[(op & 0xF0) >> 4])
            {
                pc += 2;
            }
            pc += 2;
            return;

        // LD I, addr
        // Set I = nnn
        case 0xA:
            I = op & 0x0FFF;
            pc += 2;
            return;

        // JP V0
        case 0xB:
            pc = (op & 0xFFF) + v[0];
            return;

        // RND Vx, byte
        // Set Vx to Random byte AND byte
        case 0xC:
            srand(clock());
            v[(op & 0xF00) >> 8] = rand() & (op & 0xFF);
            pc += 2;
            return;
        
        // DRW Vx, Vy, nibble
        // Display n-byte sprite starting at memory location I at (Vx, Vy), set VF = Collision
        case 0xD:
            int x, y, n;
            x = v[(op >> 8) & 0xF];
            y = v[(op >> 4) & 0xF];
            n = op & 0xF;

            v[0xF] = 0;

            for (int i = y; i < y + n; i++)
            {
                for (int ii = x; ii < x + 8; ii++)
                {
                    if (display[i][ii] == 1 && ((ram[I + (i - y)] >> (7 - (ii - x))) & 0b1) == 1)
                    {
                        v[0xF] = 1;
                    }

                    // uint8_t byte = ram[I + (i - y)];
                    // int shift = (7-(ii-x));

                    // byte = (byte >> shift
                    display[i][ii] = display[i][ii] ^ ((ram[I + (i - y)] >> (7 - (ii - x))) & 0b1);

                }
            }

            pc += 2;
            return;

        
        case 0xE:
            switch (op & 0xFF)
            {
                // SKP Vx
                // Skip next instruction if key with the value of Vx is pressed
                case 0x9E:
                    if (IsKeyPressed(keys[v[(op & 0xF00) >> 8]]))
                    {
                        pc += 2;
                        
                    }   
                    pc += 2; 
                    break;
                // SKNP Vx
                // Skip next instruction if key with the value of Vx is NOT pressed
                case 0xA1:
                    if (!IsKeyPressed(keys[v[(op & 0xF00) >> 8]]))
                    {
                        pc += 2;
                    }   
                    pc += 2;
                    break;
            }
            return;

        case 0xF:
            switch (op & 0xFF)
            {
                // LD Vx, DT
                // Loads delay timer value into Vx
                case 0x07:
                    v[(op & 0xF00) >> 8] = delay;
                    pc += 2;
                    break;
                
                // LD Vx, K
                // Waits for key press, stores the value of key in Vx
                // All execution stops until key is pressed
                case 0x0A:
                    bool key_pressed = false;
                    while (!key_pressed)
                    {
                        for (int i = 0; i < 16; i++)
                        {
                            if (IsKeyPressed(keys[i]))
                            {
                                key_pressed = true;
                                v[(op & 0xF00) >> 8] = i; 
                                i = 16;
                            }
                        }
                    }
                    pc += 2;
                    break;

                // LD DT, Vx
                // Loads value of Vx into Delay Timer
                case 0x15:
                    delay = v[(op & 0xF00) >> 8];
                    pc += 2;
                    break;

                // LD ST, Vx
                // Loads value of Vx into Sound Timer
                case 0x18:
                    sound = v[(op & 0xF00) >> 8];
                    pc += 2;
                    break;

                // Add I, Vx
                // Set I = I + Vx
                case 0x1E:
                    I = I + v[(op & 0xF00) >> 8];
                    pc += 2;
                    break;

                // LD F, Vx
                // Set I = Location of sprite for digit Vx (text store in 0x0000)
                case 0x29:
                    I = (v[(op & 0xF00) >> 8] * 5) + 0x50;
                    pc += 2;
                    break;

                // LD B, Vx
                // Store BCD representation of Vx in memory locations I, I+1, I+2
                case 0x33:
                    ram[I] = v[(op & 0xF00) >> 8] / 100;
                    ram[I + 1] = (v[(op & 0xF00) >> 8]- ram[I] * 100) / 10;
                    ram[I + 2] = (v[(op & 0xF00) >> 8] - (ram[I] * 100 ) - (ram[I + 1] * 10));
                    pc += 2;
                    break;

                // LD [I], Vx
                // Store values of registers V0 through Vx in memory location starting with I
                case 0x55:
                    for (int i = 0; i <= (op & 0xF00) >> 8; i++)
                    {
                        ram[I + i] = v[i];
                    }
                    pc += 2;
                    break;

                // LD Vx, [I]
                // Reads values of registers V0 through Vx in memory location starting with I
                case 0x65:
                    for (int i = 0; i <= (op & 0xF00) >> 8; i++)
                    {
                        v[i] = ram[I + i];
                    }
                    pc += 2;
                    break;
            }
            return;


    }
}

void open_rom(char* filepath)
{

    FILE* in_file = fopen(filepath, "rb");
    if (!in_file) {
        printf("chip8emu: Error opening file\n");
        fclose(in_file);
        exit(0);
    }

    struct stat sb;
    if (stat(filepath, &sb) == -1) {
        printf("chip8emu: Error opening stat\n");
        fclose(in_file);
        exit(0);
    }

    char* file_contents = malloc(sb.st_size);
    fread(file_contents, sb.st_size, 1, in_file);

    // Load Font
    for (int i = 0; i < 16; i++)
    {
        for (int ii = 0; ii < 5; ii++)
        {
            ram[0x50 + (i * 5 + ii)] = font[i][ii];
        }
    }

    if (sb.st_size + 0x200 > 0xFFF)
    {
        printf("chip8emu: Input file too big\n");
        fclose(in_file);
        exit(0);
    }

    for (int i = 0x200; i < 0x200 + sb.st_size; i++)
    {
        ram[i] = file_contents[i - 0x200];
    }
    fclose(in_file);

}   

void customLog(int msgType, const char *text, va_list args)
{

}

int *cpu_main()
{
    
    while (!exit_loop)
    {
        clock_t tic, toc;
        tic = clock();
        call_instruction();

        toc = clock();

        double time = (double)(toc - tic) / CLOCKS_PER_SEC;

        
        if (time < 1.0 / (double)(hz))
        {
            double sleeptime = (double)(1.0 / hz) - time;
            int microsec = sleeptime * 1000000;
            usleep(microsec);
            
        }
    }
}

int *window_main()
{
    SetTraceLogCallback(customLog);
    InitWindow(64 * size_multiplier, 32 * size_multiplier, "Chip8");
    InitAudioDevice();
    SetTargetFPS(60);
    

    while (!WindowShouldClose())
    {
        // Draw Display
        BeginDrawing();
        ClearBackground(BLACK);
        for (int i = 0; i < 64; i++)
        {
            for (int ii = 0; ii < 32; ii++)
            {
                if (display[ii][i] == 0)
                {
                    DrawRectangle(i * size_multiplier, ii * size_multiplier, size_multiplier, size_multiplier, BLACK);
                } else {
                    DrawRectangle(i * size_multiplier, ii * size_multiplier, size_multiplier, size_multiplier, WHITE);
                }
            }
        }
        EndDrawing();
    }
}

void *timer_main()
{
    while (1)
    {
        clock_t tic, toc;
        tic = clock();
        if (delay != 0)
        {
            delay--;
        }
    
        if (sound != 0)
        {
            sound--;
        }
        toc = clock();
        double time = (double)(toc - tic) / CLOCKS_PER_SEC;
        
        if (time < 1.0 / (double)(60.0))
        {
            double sleeptime = (double)(1.0 / 60.0) - time;
            int microsec = sleeptime * 1000000;
            usleep(microsec);
        }
    }
}

void set_args(struct arguments_t args)
{
    open_rom(args.filepath);
    hz = args.hz;
    size_multiplier = args.window_size_multiplier;
}