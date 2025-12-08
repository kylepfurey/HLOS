; .asm
; OS Bootloader
; by Kyle Furey


; 16-bit directives
org 0x7C00							; Set origin to BIOS bootloader address
bits 16								; Set instructions to 16-bit real mode


; OS entry point
main:
	cli								; Disable interrupts
	mov [boot_drive], dl			; Store boot drive
	jmp init_stack16				; Initialize 16-bit stack registers


; Boot drive number
boot_drive:
	db 0x0


; Initializes the 16-bit stack registers
init_stack16:
	xor ax, ax						; Initialize 16-bit accumulator to 0
	mov ds, ax						; Initialize data segment
	mov es, ax						; Initialize extra segment
	mov ss, ax						; Initialize stack segment
	mov sp, 0x7C00					; Initialize stack pointer to origin
	jmp load_kernel					; Load kernel from disk


; Loads the kernel from disk
load_kernel:
	mov ah, 0x2						; Set BIOS read sectors function
	mov al, 45						; Set number of sectors (kernel size / 512 rounded up)
	mov ch, 0x0						; Set cylinder 0
	mov cl, 0x2						; Set sector after bootloader
	mov dh, 0x0						; Set head 0
	mov dl, [boot_drive]			; Set boot drive
	mov bx, 0x1000					; Set kernel address (address must match linker)
	int 0x13						; Load kernel with a BIOS interrupt
	jc disk_error					; Jump if loading fails
	xor ax, ax						; Clear accumulator
	mov ds, ax						; Clear data segment
	mov si, msg_boot				; Load string into source index
	call print16					; Print boot message
	jmp enable_a20					; Enable A20 line


; Called when disk read fails
disk_error:
	xor ax, ax						; Clear accumulator
	mov ds, ax						; Clear data segment
	mov si, msg_error				; Load string into source index
	call print16					; Print disk error message
	jmp pause						; Pause execution


; Boot message string
msg_boot:
	db 'Successfully booted HLOS.', 0xD, 0xA, 0x0


; Prints the string held in the source index register (16-bit)
print16:
	push si							; Push source index into the stack
	push ax							; Push accumulator into the stack
.loop:
	lodsb							; Load the next character into the lower byte of accumulator
	or al, al						; Check for null-terminator character
	jz .exit						; Exit if null-terminator character
	mov ah, 0xE						; Set BIOS teletype function
	int 0x10						; Print character with a BIOS interrupt
	jmp .loop						; Loop
.exit:
	pop ax							; Pop accumulator from the stack
	pop si							; Pop source index from the stack
	ret								; Exit function


; Enables the A20 line
enable_a20:
	in al, 0x92						; Read from control port A
	or al, 0x2						; Enable A20 gate
	out 0x92, al					; Write to control port A
	jmp protected_mode				; Enter 32-bit protected mode


; Disk error message string
msg_error:
	db 'Could not load kernel!', 0xD, 0xA, 0x0


; Pauses execution
pause:
	hlt								; Halt CPU
	jmp pause						; Infinitely loop


; Enters 32-bit protected mode
protected_mode:
	lgdt [gdt_descriptor]			; Load GDT
	mov eax, cr0					; Read from control register 0
	or eax, 0x1						; Enable protected mode
	mov cr0, eax					; Write to control register 0
	jmp 0x8:init_stack32			; Initialize 32-bit stack selectors (32-bit jump)


; GDT Descriptor structure
gdt_descriptor:
	dw gdt_end - gdt_start - 1		; Size of GDT
	dd gdt_start					; Address of GDT


; Global Descriptor Table
gdt_start:
	dq 0x0							; Null segment
	dq 0xCF9A000000FFFF				; Code segment
	dq 0xCF92000000FFFF				; Data segment
gdt_end:


; 32-bit directives
bits 32								; Set instructions to 32-bit protected mode


; Initializes the 32-bit stack selectors
init_stack32:
	mov ax, 0x10					; Store data segment selector
	mov ds, ax						; Initialize data segment selector
	mov es, ax						; Initialize extra segment selector
	mov fs, ax						; Initialize general purpose segment selector
	mov gs, ax						; Initialize general purpose segment selector
	mov ss, ax						; Initialize stack segment selector
	mov esp, 0x90000				; Initialize stack pointer to top
	mov ebp, esp					; Initialize base pointer
	cld								; Clear direction flag
	xor eax, eax					; Initialize 32-bit accumulator to 0
	xor ebx, ebx					; Initialize base register to 0
	xor ecx, ecx					; Initialize counter to 0
	xor edx, edx					; Initialize data register to 0
	jmp init_interrupts				; Initialize IDT


; Sets up the Interrupt Descriptor Table
init_interrupts:
	mov eax, ignore_interrupt		; Store address of callback in accumulator
	mov ebx, eax					; Copy callback address into base register
	mov ecx, 0x100					; Store number of IDT entries in counter
	mov edi, idt_start				; Store location of IDT in destination index
.loop:
	mov word [edi], ax				; Set address at low offset
	mov word [edi+2], 0x8			; Set code segment selector (from GDT)
	mov byte [edi+4], 0x0			; Set reserved byte to 0
	mov byte [edi+5], 0x8E			; Set interrupt flags
	shr ebx, 0x10					; Shift base register 16-bits right to to get the high offset
	mov word [edi+6], bx			; Set address at high offset
	mov ebx, eax					; Re-copy callback address into base register
	add edi, 0x8					; Increment destination index by 8 bytes
	loop .loop						; Loop until counter is 0
.exit:
	lidt [idt_descriptor]			; Load IDT
	jmp remap_pic					; Remap the PIC and enable interrupts


; Ignores an interrupt
ignore_interrupt:
	pushad							; Push 32-bit registers to the stack
	mov al, 0x20					; Store the end-of-interrupt command
	out 0xA0, al					; Send end-of-interrupt command to the slave PIC
	out 0x20, al					; Send end-of-interrupt command to the master PIC
	popad							; Pop 32-bit registers from the stack
	iret							; Exit interrupt


; IDT Descriptor structure
idt_descriptor:
	dw idt_end - idt_start - 1		; Size of IDT
	dd idt_start					; Address of IDT


; Interrupt Descriptor Table
idt_start equ 0x80000				; Set IDT at 0x80000
idt_end equ idt_start + 256*8		; Reserve 256 8-byte entries for callbacks


; Remaps the Programmable Interrupt Controller to the IDT and enables interrupts
remap_pic:
	in al, 0x21						; Read from the master PIC's mask
	push ax							; Push the master PIC's mask to the stack
	in al, 0xA1						; Read from the slave PIC's mask
	push ax							; Push the slave PIC's mask to the stack
	mov al, 0x11					; Store the ICW1 command
	out 0x20, al					; Send ICW1 command to the master PIC
	out 0xA0, al					; Send ICW1 command to the slave PIC
	mov al, 0x20					; Store the ICW2 command
	out 0x21, al					; Send ICW2 command to the master PIC
	mov al, 0x28					; Store the ICW2 command
	out 0xA1, al					; Send ICW2 command to the slave PIC
	mov al, 0x4						; Store the ICW3 command
	out 0x21, al					; Send ICW3 command to the master PIC
	mov al, 0x2						; Store the ICW3 command
	out 0xA1, al					; Send ICW3 command to the slave PIC
	mov al, 0x1						; Store the ICW4 command
	out 0x21, al					; Send ICW4 command to the master PIC
	out 0xA1, al					; Send ICW4 command to the slave PIC
	pop ax							; Pop the slave PIC's mask from the stack
	out 0xA1, al					; Write to the slave PIC's mask
	pop ax							; Push the master PIC's mask from the stack
	out 0x21, al					; Write to the master PIC's mask
	jmp init_floats					; Initialize floating-point operations


; Initializes floating-point operations
init_floats:
	mov eax, cr0					; Read from control register 0
	and eax, 0xFFFFFFFB				; Disable FPU emulation mode
	or eax, 0x22					; Handle FPU errors
	mov cr0, eax					; Write to control register 0
	mov eax, cr4					; Read from control register 4
	or eax, 0x600					; Enable SSE instructions
	mov cr4, eax					; Write to control register 4
	fninit							; Initializes registers for floating-point arithmetic
	jmp start_kernel				; Jump to kernel entry point


; Jumps to kernel entry point
start_kernel:
	jmp 0x1000						; Jump to kernel_main() (address must match linker)


; Boot sector
times 510-($-$$) db 0x0				; Fill boot sector to 512 bytes
dw 0xAA55							; Boot signature
