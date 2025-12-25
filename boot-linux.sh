# .sh
# OS Boot Batch Script
# by Kyle Furey
# Requires Linux, NASM, i686-elf-tools, and QEMU

# Set build and intermediate directories
BUILD=bin/x86
INTERMEDIATE=obj/x86

# Create build and intermediate directories
mkdir -p "$BUILD"
mkdir -p "$INTERMEDIATE"

# Assemble boot.asm
nasm -f bin "boot/boot.asm" -o "$BUILD/boot.bin"

# Compile kernel.c and other scripts
COMPILE="/opt/i686-elf-tools-linux/bin/i686-elf-gcc -ffreestanding -m32 -c"
ASSEMBLE="nasm -f elf32"
$COMPILE  "kernel/kernel.c" -o "$INTERMEDIATE/kernel.o"
$COMPILE  "kernel/string.c" -o "$INTERMEDIATE/string.o"
$COMPILE  "kernel/lib.c" -o "$INTERMEDIATE/lib.o"
$ASSEMBLE "kernel/assembly.asm" -o "$INTERMEDIATE/assembly.o"
$COMPILE  "kernel/interrupt.c" -o "$INTERMEDIATE/interrupt.o"
$COMPILE  "kernel/init.c" -o "$INTERMEDIATE/init.o"
$COMPILE  "kernel/print.c" -o "$INTERMEDIATE/print.o"
$COMPILE  "kernel/read.c" -o "$INTERMEDIATE/read.o"
$COMPILE  "kernel/time.c" -o "$INTERMEDIATE/time.o"
$COMPILE  "kernel/rng.c" -o "$INTERMEDIATE/rng.o"
$COMPILE  "kernel/file.c" -o "$INTERMEDIATE/file.o"
$COMPILE  "kernel/malloc.c" -o "$INTERMEDIATE/malloc.o"
$COMPILE  "kernel/event.c" -o "$INTERMEDIATE/event.o"
$COMPILE  "kernel/beep.c" -o "$INTERMEDIATE/beep.o"

# Link compiled kernel with linker.ld
/opt/i686-elf-tools-linux/bin/i686-elf-ld -T "boot/linker.ld" -o "$BUILD/kernel.bin"\
    "$INTERMEDIATE/kernel.o"\
    "$INTERMEDIATE/string.o"\
    "$INTERMEDIATE/lib.o"\
    "$INTERMEDIATE/assembly.o"\
    "$INTERMEDIATE/interrupt.o"\
    "$INTERMEDIATE/init.o"\
    "$INTERMEDIATE/print.o"\
    "$INTERMEDIATE/read.o"\
    "$INTERMEDIATE/time.o"\
    "$INTERMEDIATE/rng.o"\
    "$INTERMEDIATE/file.o"\
    "$INTERMEDIATE/malloc.o"\
    "$INTERMEDIATE/event.o"\
    "$INTERMEDIATE/beep.o"

# Log the size of boot and kernel binaries
echo
echo -----------------------------------------------------------------
echo Boot Size \(MUST EQUAL 512\)
echo -----------------------------------------------------------------
echo
ls -l "$BUILD/boot.bin"
echo
echo -----------------------------------------------------------------
echo Kernel Size
echo -----------------------------------------------------------------
echo
ls -l "$BUILD/kernel.bin"
echo
echo -----------------------------------------------------------------
echo Divide by 512 and round up. That is the number of sectors loaded.
echo -----------------------------------------------------------------
echo

if [ ! -f "$BUILD/hlos.img" ]; then
   # Create binary blob for virtual hard drive space
    qemu-img create -f raw "$BUILD/raw.bin" 512000000 # 512 MB

    # Append kernel.bin and raw.bin into boot.bin
    cat "$BUILD/boot.bin" "$BUILD/kernel.bin" "$BUILD/raw.bin" > "$BUILD/hlos.img"
fi

# Boot hlos.img in QEMU
qemu-system-i386\
    -drive file="$BUILD/hlos.img",format=raw\
    -audiodev sdl,id=snd0\
    -device sb16,audiodev=snd0\
    -machine pcspk-audiodev=snd0
