
# Note: This file is just for CLion syntax highlighting
# Build using build-win.sh or build-linux.sh

cmake_minimum_required(VERSION 4.0)
project(hlos)

set(CMAKE_C_STANDARD 11)

include_directories(kernel)

add_executable(hlos
        boot/boot.asm
        boot/linker.ld
        kernel/assembly.asm
        kernel/assembly.h
        kernel/beep.c
        kernel/beep.h
        kernel/event.c
        kernel/event.h
        kernel/exec.c
        kernel/exec.h
        kernel/extern.h
        kernel/file.c
        kernel/file.h
        kernel/hlos.h
        kernel/init.c
        kernel/init.h
        kernel/interrupt.c
        kernel/interrupt.h
        kernel/kernel.c
        kernel/lib.c
        kernel/lib.h
        kernel/malloc.c
        kernel/malloc.h
        kernel/print.c
        kernel/print.h
        kernel/read.c
        kernel/read.h
        kernel/rng.c
        kernel/rng.h
        kernel/socket.c
        kernel/socket.h
        kernel/string.c
        kernel/string.h
        kernel/thread.c
        kernel/thread.h
        kernel/time.c
        kernel/time.h
        kernel/types.h)
