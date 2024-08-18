global long_mode_start
extern call_constructors
extern pre_kernel
extern stackTop
extern gdt64.pointer
extern gdt64
extern tss64
section .text
bits 64

long_mode_start:
    cli
    mov rax, higher_half_switch
    jmp rax
higher_half_switch:
    mov rsp, stackTop
    mov ax, 0
    mov ss, ax
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    call call_constructors
    mov edi, ebx
    ; We pass structure pointers onto the pre_kernel
    mov rsi, gdt64.pointer 
    lgdt [rsi]
    mov rdx, gdt64
    mov rcx, tss64
    call pre_kernel 
    hlt
