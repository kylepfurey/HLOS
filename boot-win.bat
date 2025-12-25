:: .bat
:: OS Boot Batch Script
:: by Kyle Furey
:: Requires Windows, NASM, i686-elf-tools, and QEMU

@echo off

:: Set build and intermediate directories
set BUILD=bin\x86
set INTERMEDIATE=obj\x86

:: Create build and intermediate directories
if not exist "%BUILD%" mkdir "%BUILD%"
if not exist "%INTERMEDIATE%" mkdir "%INTERMEDIATE%"

:: Assemble boot.asm
"%APPDATA%\..\Local\bin\NASM\nasm.exe" -f bin "boot\boot.asm" -o "%BUILD%\boot.bin"

:: Compile kernel.c and other scripts
set COMPILE="%PROGRAMFILES%\i686-elf-tools-windows\bin\i686-elf-gcc.exe" -ffreestanding -m32 -c
set ASSEMBLE="%APPDATA%\..\Local\bin\NASM\nasm.exe" -f elf32
%COMPILE%  "kernel\kernel.c" -o "%INTERMEDIATE%\kernel.o"
%COMPILE%  "kernel\string.c" -o "%INTERMEDIATE%\string.o"
%COMPILE%  "kernel\lib.c" -o "%INTERMEDIATE%\lib.o"
%ASSEMBLE% "kernel\assembly.asm" -o "%INTERMEDIATE%\assembly.o"
%COMPILE%  "kernel\interrupt.c" -o "%INTERMEDIATE%\interrupt.o"
%COMPILE%  "kernel\init.c" -o "%INTERMEDIATE%\init.o"
%COMPILE%  "kernel\print.c" -o "%INTERMEDIATE%\print.o"
%COMPILE%  "kernel\read.c" -o "%INTERMEDIATE%\read.o"
%COMPILE%  "kernel\time.c" -o "%INTERMEDIATE%\time.o"
%COMPILE%  "kernel\rng.c" -o "%INTERMEDIATE%\rng.o"
%COMPILE%  "kernel\file.c" -o "%INTERMEDIATE%\file.o"
%COMPILE%  "kernel\malloc.c" -o "%INTERMEDIATE%\malloc.o"
%COMPILE%  "kernel\event.c" -o "%INTERMEDIATE%\event.o"
%COMPILE%  "kernel\beep.c" -o "%INTERMEDIATE%\beep.o"

:: Link compiled kernel with linker.ld
"%PROGRAMFILES%\i686-elf-tools-windows\bin\i686-elf-ld.exe" -T "boot\linker.ld" -o "%BUILD%\kernel.bin"^
	"%INTERMEDIATE%\kernel.o"^
	"%INTERMEDIATE%\string.o"^
	"%INTERMEDIATE%\lib.o"^
	"%INTERMEDIATE%\assembly.o"^
	"%INTERMEDIATE%\interrupt.o"^
	"%INTERMEDIATE%\init.o"^
	"%INTERMEDIATE%\print.o"^
	"%INTERMEDIATE%\read.o"^
	"%INTERMEDIATE%\time.o"^
	"%INTERMEDIATE%\rng.o"^
	"%INTERMEDIATE%\file.o"^
	"%INTERMEDIATE%\malloc.o"^
	"%INTERMEDIATE%\event.o"^
	"%INTERMEDIATE%\beep.o"

:: Log the size of boot and kernel binaries
echo.
echo -----------------------------------------------------------------
echo Boot Size (MUST EQUAL 512)
echo -----------------------------------------------------------------
echo.
dir "%BUILD%\boot.bin"
echo.
echo -----------------------------------------------------------------
echo Kernel Size
echo -----------------------------------------------------------------
echo.
dir "%BUILD%\kernel.bin"
echo.
echo -----------------------------------------------------------------
echo Divide by 512 and round up. That is the number of sectors loaded.
echo -----------------------------------------------------------------
echo.

:: Create binary blob for virtual hard drive space
if not exist "%BUILD%\hlos.img" qemu-img create -f raw "%BUILD%\raw.bin" 512000000 # 512 MB

:: Append kernel.bin into boot.bin
if not exist "%BUILD%\hlos.img" type "%BUILD%\boot.bin" "%BUILD%\kernel.bin" "%BUILD%\raw.bin" > "%BUILD%\hlos.img"

:: Boot hlos.img in QEMU
"%PROGRAMFILES%\qemu\qemu-system-i386.exe"^
    -fda "%BUILD%\hlos.img"^
    -drive file="%BUILD%\hlos.img",format=raw^
    -audiodev sdl,id=snd0^
    -device sb16,audiodev=snd0^
    -machine pcspk-audiodev=snd0
