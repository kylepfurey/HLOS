; .asm
; OS External Assembly Functions
; by Kyle Furey


; 32-bit directives
bits 32								; Set instructions to 32-bit protected mode


; Declare exported functions
global hlt							; void hlt()
global cli							; void cli()
global sti							; void sti()
global in							; byte_t in(ushort_t port)
global out							; void out(ushort_t port, byte_t num)
global in2							; ushort_t in2(ushort_t port)
global out2							; void out2(ushort_t port, ushort_t num)
global call							; void call(const void *addr)
global timer_interrupt				; void timer_interrupt()
global keyboard_interrupt			; void keyboard_interrupt()
global page_fault_interrupt		 	; void page_fault_interrupt()
global enable_paging				; void enable_paging()
global invlpg						; void invlpg(const void *addr)


; Declare external functions
extern tick							; void tick()
extern key							; void key()
extern crash						; void crash(uint_t err)


; Pauses the CPU
hlt:
	sti								; Enable interrupts
	hlt								; Halt CPU
	ret								; Exit function


; Disables external interrupts
cli:
	cli								; Disable interrupts
	ret								; Exit function


; Enables external interrupts
sti:
	sti								; Enable interrupts
	ret								; Exit function


; Reads a byte from the given IO port
in:
	push ebp						; Push base pointer to the stack
	mov ebp, esp					; Store the stack pointer
	mov dx, [ebp+8]					; Store <port>
	in al, dx						; Read from <port>
	pop ebp							; Pop base pointer from the stack
	ret								; Exit function


; Writes a byte to the given IO port
out:
	push ebp						; Push base pointer to the stack
	mov ebp, esp					; Store the stack pointer
	mov dx, [ebp+8]					; Store <port>
	mov al, [ebp+12]				; Store <num>
	out dx, al						; Write <num> to <port>
	pop ebp							; Pop base pointer from the stack
	ret								; Exit function


; Reads a short from the given IO port
in2:
	push ebp						; Push base pointer to the stack
	mov ebp, esp					; Store the stack pointer
	mov dx, [ebp+8]					; Store <port>
	in ax, dx						; Read from <port>
	pop ebp							; Pop base pointer from the stack
	ret								; Exit function


; Writes a short to the given IO port
out2:
	push ebp						; Push base pointer to the stack
	mov ebp, esp					; Store the stack pointer
	mov dx, [ebp+8]					; Store <port>
	mov ax, [ebp+12]				; Store <num>
	out dx, ax						; Write <num> to <port>
	pop ebp							; Pop base pointer from the stack
	ret								; Exit function


; Jumps to and begins executing the given memory address until it returns
call:
	push ebp						; Push base pointer to the stack
	mov ebp, esp					; Store the stack pointer
	mov eax, [ebp+8] 				; Store <addr>
	call eax						; Execute code at <addr>
	pop ebp							; Pop base pointer from the stack
	ret								; Exit function


; INTERRUPTS


; The callback for the timer interrupt
timer_interrupt:
	pushad							; Push 32-bit registers to the stack
	push ds							; Push data segment selector to the stack
	push es							; Push extra segment selector to the stack
	push fs							; Push general purpose segment selector to the stack
	push gs							; Push general purpose segment selector to the stack
	call tick						; Call tick() in C
	mov al, 0x20					; Store the end-of-interrupt command
	out 0x20, al					; Send end-of-interrupt command to the master PIC
	pop gs							; Pop general purpose segment selector from the stack
	pop fs							; Pop general purpose segment selector from the stack
	pop es							; Pop extra segment selector from the stack
	pop ds							; Pop data segment from the stack
	popad							; Pop 32-bit registers from the stack
	iret							; Exit interrupt


; The callback for the keyboard interrupt
keyboard_interrupt:
	pushad							; Push 32-bit registers to the stack
	push ds							; Push data segment selector to the stack
	push es							; Push extra segment selector to the stack
	push fs							; Push general purpose segment selector to the stack
	push gs							; Push general purpose segment selector to the stack
	call key						; Call key() in C
	mov al, 0x20					; Store the end-of-interrupt command
	out 0x20, al					; Send end-of-interrupt command to the master PIC
	pop gs							; Pop general purpose segment selector from the stack
	pop fs							; Pop general purpose segment selector from the stack
	pop es							; Pop extra segment selector from the stack
	pop ds							; Pop data segment from the stack
	popad							; Pop 32-bit registers from the stack
	iret							; Exit interrupt


; The callback for a page fault interrupt
page_fault_interrupt:
	pushad							; Push 32-bit registers to the stack
	push ds							; Push data segment selector to the stack
	push es							; Push extra segment selector to the stack
	push fs							; Push general purpose segment selector to the stack
	push gs							; Push general purpose segment selector to the stack
	push dword [esp + 20]			; Push <err> to the stack
	call crash						; Call crash() in C
	add esp, 4						; Pop <err> from the stack
	pop gs							; Pop general purpose segment selector from the stack
	pop fs							; Pop general purpose segment selector from the stack
	pop es							; Pop extra segment selector from the stack
	pop ds							; Pop data segment from the stack
	popad							; Pop 32-bit registers from the stack
	add esp, 4						; Pop the error code from the stack
	iret							; Exit interrupt


; HEAP


; Enables memory paging and virtual addressing
enable_paging:
	cli								; Disable interrupts
	mov eax, 0x100000				; Set address of page directory (address must match linker)
	mov cr3, eax					; Write to control register 3
	mov eax, cr0					; Read from control register 0
	or eax, 0x80000000				; Enable the paging flag
	mov cr0, eax					; Write to control register 0
	sti								; Enable interrupts
	ret								; Exit function


; Forces the CPU to refresh a memory page
invlpg:
	push ebp						; Push base pointer to the stack
	mov ebp, esp					; Store the stack pointer
	mov eax, [ebp+8]				; Store <addr>
	invlpg [eax]					; Invalidate the page entry
	pop ebp							; Pop base pointer from the stack
	ret								; Exit function
