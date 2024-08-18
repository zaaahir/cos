global _start
global stackTop
global gdt64.pointer
global gdt64
global tss64
extern long_mode_start
; GRUB w/ multiboot 2 already puts us in protected mode
bits 32 

; We place the kernel here in virtual address space
KERNEL_V_BASE EQU 0xFFFFFF8000000000

PAGE_PRESENT EQU 0x1
PAGE_WRITABLE EQU 0x2
PAGE_HUGE EQU 0x80

PAGE_4KiB EQU 0x1000
PAGE_2MiB EQU 0x200000
PAGE_1GiB EQU 0x40000000

_start: 
    mov esp, stackTop-KERNEL_V_BASE

    call init_paging_and_long_mode

    jmp gdt64.kernelCodeSegment:long_mode_start-KERNEL_V_BASE

    hlt

; Maps physical memory into virtual address space
setup_page_tables:
    mov eax, pageTableL3-KERNEL_V_BASE
    or eax, PAGE_PRESENT
    or eax, PAGE_WRITABLE
    ; We identity map 512GiB of memory at the start of address space
    mov [pageTableL4-KERNEL_V_BASE], eax
    ; We also map it to the last 512GiB of the address space
    mov [pageTableL4-KERNEL_V_BASE + 511 * 8], eax
    mov ecx, 0
    .loop:
     mov eax, PAGE_1GiB 
     mul ecx
     or eax, PAGE_PRESENT
     or eax, PAGE_WRITABLE
     ; Use 1GiB pages
     or eax, PAGE_HUGE
     mov [pageTableL3 - KERNEL_V_BASE + ecx * 8], eax
     inc ecx
     ; Map 512GiB of physical memory
     cmp ecx, 512
     jne .loop
    ret

init_paging_and_long_mode: 
    call setup_page_tables
    mov eax, pageTableL4-KERNEL_V_BASE
    ; load top level page table
    mov cr3, eax
    mov eax, cr4
    ; enable PAE paging
    or eax, 1 << 5 
    mov cr4, eax

    mov ecx, 0xC0000080
    rdmsr
    ; set long mode bit
    or eax, 1 << 8 
    wrmsr

    mov eax, cr0
    ; enable paging
    or eax, 1 << 31 
    mov cr0, eax

    lgdt [gdt64.pointer-KERNEL_V_BASE]
    ret
