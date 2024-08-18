section .text
global isr_table
extern _ZN3CPU16InterruptManager20handle_err_interruptEhh
extern _ZN3CPU16InterruptManager22handle_noerr_interruptEh
extern isr_def_32

%macro pushaq 0
    push rax
    push rbx
    push rcx
    push rdx
    push rbp
    push rdi
    push rsi
    push r8
    push r9
    push r10
    push r11
    push r12
    push r13
    push r14
    push r15
%endmacro

%macro popaq 0
    pop r15
    pop r14
    pop r13
    pop r12
    pop r11
    pop r10
    pop r9
    pop r8
    pop rsi
    pop rdi
    pop rbp
    pop rdx
    pop rcx
    pop rbx
    pop rax
%endmacro

%macro error_isr 1
isr_def_%+%1:
    pop rsi
    pushaq
    mov rdi, %1
    call _ZN3CPU16InterruptManager20handle_err_interruptEhh
    popaq
    iretq
%endmacro

%macro no_error_isr 1
isr_def_%+%1:
    pushaq
    mov rdi, %1
    call _ZN3CPU16InterruptManager22handle_noerr_interruptEh
    popaq
    iretq
%endmacro
