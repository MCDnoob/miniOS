[bits 32]
; 32位代码

global _start
_start:
    mov byte [0xb8000], 'K' ; 表示进入内核
    jmp $; 阻塞