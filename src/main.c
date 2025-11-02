#include <stdio.h>
#include <raylib.h>
#include <pthread.h>
#include "arguments.h"
#include "chip8.h"

int main(int argc, char** argv) 
{
    struct arguments_t args;
    args = lexicalize_args(argc, argv);

    switch (args.error)
    {
    case FILE_DOESNT_EXIST:
        printf("chip8emu: File specified does not exist.\n");
        return -1;
        
    case NO_FILE:
        printf("chip8emu: No file specified\nUse --help\n");
        return -1;

    case INCOMPLETE_ARG:
        printf("chip8emu: Incomplete argument\n");
        return -1;
    
    case UNKNOWN_ARG:
        printf("chip8emu: Unknown Argument\n");
        return -1;
    }

    switch (args.mode)
    {
    case CHIP:
        set_args(args);
    
        pthread_t window, cpu, timer;

        pthread_create(&window, NULL, window_main, NULL);
        pthread_create(&cpu, NULL, cpu_main, NULL);
        pthread_create(&timer, NULL, timer_main, NULL);

        pthread_join(window, NULL);
        return 0;
    case HELP:
        printf("chip8emu is emulator for Chip 8\n<file>           The file opened and interpreted\n-hz n            The cpu clockspeed (default = 300)\n-help            Shows this menu\n-scale n        Sets the window size multiplier\n");
        return 0;
    }
}
