cmake_minimum_required(VERSION 3.0.0)
project(chip8 VERSION 0.1.0)


add_executable(chip8 src/main.c src/chip8.c src/arguments.c)

include_directories(include)

target_link_libraries(chip8 raylib)
target_link_libraries(chip8 m)

install(TARGETS chip8 DESTINATION bin)
