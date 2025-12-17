[org 0x1000]

dw 0x55aa; 在0x1000位置开头是55aa，用于判断错误
mov si, loading
call print
xchg bx, bx
; 内存检测
detect_memory:
    xor ebx, ebx; ebx置为0
    ; es:di 结构体的缓存位置
    mov ax, 0
    mov es, ax; 段寄存器es不能mov立即数，只能通过其他寄存器赋值
    mov edi, ards_buffer
    mov edx, 0x534d4150; 固定签名

.next:
    mov eax, 0xe820; 子功能号
    mov ecx, 20; adrs结构体大小
    int 0x15; 调用0x15实模式中断进行内存检测

    jc error; 如果CF置位表示出错，直接跳转

    add di, cx; 将缓存指针指向下一个结构体
    inc word [ards_count]; 结构体数量+1

    cmp ebx, 0
    jnz .next; 如果ebx不为0，就继续检测

    mov si, detecting
    call print

    xchg bx, bx

    mov cx, [ards_count]; 结构体数量
    mov si, 0; 结构体指针

.show:
    mov eax, [ards_buffer + si]; 基地址
    mov ebx, [ards_buffer + si + 8]; 长度
    mov edx, [ards_buffer + si + 16]; 内存类型
    add si, 20; 指针移动，到下一个ards结构体
    xchg bx, bx
    loop .show

; 阻塞
jmp $

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

; 定义字符串loading
loading:
    db "loading os", 10, 13, 0 ;\n\r 0为字符串结束
detecting:; 检测内存成功
    db "Detecting Memory Success...", 10, 13, 0 ;\n\r 0为字符串结束

error:
    mov si, .msg
    call print
    hlt; 让CPU停止
    jmp $
    .msg db "Loading Error!!!", 10, 13, 0

; 内存检测返回ards结构体的数量
ards_count:
    dw 0
; 内存检测返回ards结构体的缓存，放在最后，以免内存溢出
ards_buffer: