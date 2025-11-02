#include "arguments.h"
#include <unistd.h>
#include <string.h>

struct arguments_t lexicalize_args(int argc, char** argv)
{
    struct arguments_t args;
    args.hz = 300;
    args.window_size_multiplier = 15;
    
    for (int i = 1; i < argc; i++)
    {
        char* arg = argv[i];

        args.mode = CHIP;

        if (arg[0] == '-')
        {
            arg++;
            if (arg[0] == '-')
            {
                arg++;
            }

            if (strcmp(arg, "hz") == 0)
            {
                if (i + 1 != argc)
                {
                    i++;
                } else {
                    args.error = INCOMPLETE_ARG;
                    return args;
                }
                args.hz = atoi(argv[i]);
            } else if (strcmp(arg, "help") == 0 || strcmp(arg, "h") == 0)
            {
                args.mode = HELP;
            } else if (strcmp(arg, "scale") == 0)
            {
                if (i + 1 != argc)
                {
                    i++;
                } else {
                    args.error = INCOMPLETE_ARG;
                    return args;
                }
                args.window_size_multiplier = atoi(argv[i]);
            } else {
                args.error = UNKNOWN_ARG;
                return args;
            }
        } else {
            args.filepath = arg;
            if (access(args.filepath, F_OK) == 0) {
                // file exists
            } else {
                // file doesn't exist
                args.error = FILE_DOESNT_EXIST;
            }
        }
    }
    if (args.filepath == NULL && args.mode == CHIP) 
    {
        args.error = NO_FILE;
    }

    return args;
}