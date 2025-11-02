enum mode_t
{
    CHIP, HELP
};

enum error_t
{
    NONE, FILE_DOESNT_EXIST, NO_FILE, INCOMPLETE_ARG, UNKNOWN_ARG
};

struct arguments_t
{
    char* filepath;
    int hz;
    int sound_pitch;
    int window_size_multiplier;
    enum mode_t mode;
    enum error_t error;
};

struct arguments_t lexicalize_args(int argc, char** argv);