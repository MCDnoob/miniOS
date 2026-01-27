[bits 32]
; 32位代码

extern kernel_init
global _start
_start:
    ;mov byte [0xb8000], 'K' ; 表示进入内核
    call kernel_init
    xchg bx, bx
    ;int 0x80; 调用0x80系统调用
    ; 除0异常测试
    mov bx, 0
    div bx
    jmp $; 阻塞