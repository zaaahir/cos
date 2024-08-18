section .multiboot_header
header_start:
    dd 0xe85250d6 ; magic number to identify the header 
    dd 0 ; start in 32-bit protected mode
    dd header_end - header_start ; length of header
    dd 0x100000000 - (0xe85250d6 + (header_end - header_start)) ; checksum

    ; end tag
    dw 0
    dw 0
    dd 8
header_end:
