[org 0x7c00]
; 代码的起始位置在内存0x7c00

; 设置屏幕模式为文本模式，清除屏幕
mov ax, 3
int 0x10

; 初始化段寄存器
mov ax, 0
mov ds, ax
mov es, ax
mov ss, ax
mov sp, 0x7c00  ; 设置栈的起始位置就是0x7c00

mov si, booting
call print; 调用函数print

mov edi, 0x1000; 读取目标内存
mov ecx, 2; 起始扇区
mov bl, 4; 扇区数量
call read_disk; 调用函数read_disk

;xchg bx, bx; 魔术断点
cmp word [0x1000], 0x55aa; 判断0x1000头两个字节是否为55aa
jnz error; 不是就跳转到error
jmp 0:0x1002; 是就跳转到1002位置继续运行
; 阻塞，$表示当前这一行
jmp $

read_disk:
    ; 设置读写扇区数量
    mov dx, 0x1f2
    mov al, bl
    out dx, al

    inc dx; 0x1f3
    mov al, cl; 起始扇区的前八位
    out dx, al

    inc dx; 0x1f4
    shr ecx, 8
    mov al, cl; 起始扇区的中八位
    out dx, al

    inc dx; 0x1f5
    shr ecx, 8
    mov al, cl; 起始扇区的高八位
    out dx, al

    inc dx; 0x1f6
    shr ecx, 8
    and cl, 0b1111; 高四位设置为0

    mov al, 0b1110_0000
    or al, cl
    out dx, al; 主盘-LBA模式

    inc dx; 0x1f7
    mov al, 0x20; 读硬盘
    out dx, al

    xor ecx, ecx; 将ecx清空
    mov cl, bl; 得到读写扇区数量

    .read:
        push cx; 保存cx
        call .waits; 等待数据准备完毕
        call .reads; 读取一个扇区
        pop cx; 恢复cx
        loop .read
    
    ret

    .waits:
        mov dx, 0x1f7
        .check:
            in al, dx
            jmp $+2; nop 直接跳转到下一行
            jmp $+2; 一点点延迟
            jmp $+2
            and al, 0b1000_1000
            cmp al, 0b0000_1000
            jnz .check
        ret
    
    .reads:
        mov dx, 0x1f0
        mov cx, 256; 一个扇区256字
        .readw:
            in ax, dx
            jmp $+2; nop 直接跳转到下一行
            jmp $+2; 一点点延迟
            jmp $+2
            mov [edi], ax
            add edi, 2
            loop .readw
        ret
    
error:
    mov si, .msg
    call print
    hlt; 让CPU停止
    jmp $
    .msg db "Booting Error!!!", 10, 13, 0

; 函数print
print:
    mov ah, 0x0e
.next:
    mov al, [si]
    cmp al, 0;  al与0做比较
    jz .done;   如果al是0，就跳转到done，结束
    int 0x10
    inc si
    jmp .next
.done:
    ret

; 定义字符串booting
booting:
    db "Booting os...", 10, 13, 0 ;\n\r 0为字符串结束

; 主引导扇区的其他字节用0填充，$$表示起始行
times 510 - ($ - $$) db 0

; 主引导扇区512字节的最后两个字节必须是0x55和0xaa
db 0x55, 0xaa