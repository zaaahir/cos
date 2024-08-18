bits 64
section .text
global isr_def_32
global start_kernel_task_iretq
global return_to_task
global pit_interrupt_end
global block_task_and_refresh
global send_pit_eoi

extern _ZN4Task11TaskManager12is_executingEv
extern _ZN4Task11TaskManager26is_scheduler_changing_taskEv
extern _ZN4Task11TaskManager11save_kstackEPv
extern _ZN4Task11TaskManager7refreshEv
extern _ZN4Task11TaskManager8pit_tickEv
extern _ZN4Task11TaskManager14get_safe_stackEv
extern _ZN4Task11TaskManager17get_current_stackEv
extern _ZN4Task11TaskManager16get_current_taskEv
extern _ZN4Task11TaskManager20save_kstack_with_tidEPvy
extern _ZN3CPU3PIT4tickEv

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

; PIT ISR
isr_def_32: 
    pushaq
    call _ZN3CPU3PIT4tickEv
    ; End interrupt if the Task Manager is not active.
    call _ZN4Task11TaskManager12is_executingEv
    cmp rax, 0
    je pit_interrupt_end
    ; If the scheduler is changing tasks, we call pit_tick().
    call _ZN4Task11TaskManager26is_scheduler_changing_taskEv
    cmp rax, 1
    je call_pit_tick
    ; Save the current kernel stack and switch to the safe stack.
    mov rdi, rsp
    call _ZN4Task11TaskManager11save_kstackEPv
    call _ZN4Task11TaskManager14get_safe_stackEv
    mov rsp, rax
    jmp _ZN4Task11TaskManager8pit_tickEv
