[org 0x1000]

dw 0x55aa; 在0x1000位置开头是55aa，用于判断错误
mov si, loading
call print
;xchg bx, bx
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
    
    ; 实模式测试
    ;xchg bx, bx
    ;mov byte [0xb8000], 'p'

    jmp prepare_protected_mode; 跳转到准备保护模式

; 准备进入保护模式
prepare_protected_mode:
    cli; 关闭中断
    ;打开A20线
    in al, 0x92
    or al, 0b10
    out 0x92, al

    lgdt [gdt_ptr]; 加载gdt

    ; 启动保护模式
    mov eax, cr0
    or eax, 1
    mov cr0, eax
    ; 用跳转来刷新缓存，启用保护模式
    jmp dword code_selector:project_mode

;    xchg bx, bx

;    mov cx, [ards_count]; 结构体数量
;    mov si, 0; 结构体指针

;.show:
;    mov eax, [ards_buffer + si]; 基地址
;    mov ebx, [ards_buffer + si + 8]; 长度
;    mov edx, [ards_buffer + si + 16]; 内存类型
;    add si, 20; 指针移动，到下一个ards结构体
;    xchg bx, bx
;    loop .show

; 阻塞
;jmp $

; 实模式函数print，只能在实模式中使用，因为这个用的是BIOS的中断int 0x10
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

; 保护模式
[bits 32]
project_mode:
    xchg bx, bx; 断点
    mov ax, data_selector
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    mov ss, ax; 初始化段寄存器

    mov esp, 0x10000; 修改栈顶，地址是随便写的，是实模式可用范围

    mov edi, 0x10000; 读取目标内存
    mov ecx, 10; 起始扇区
    mov bl, 200; 扇区数量

    call read_disk

    jmp dword code_selector:0x10000

    ud2; 表示出错，不可能走在这个地方

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

; 段选择子
code_selector equ (1 << 3)
data_selector equ (2 << 3)

memory_base equ 0; 内存开始的位置，基地址
memory_limit equ ((1024 * 1024 * 1024 * 4) / (1024 * 4)) - 1; 内存界限: 4G/4k - 1

; 描述符指针
gdt_ptr:
    dw (gdt_end - gdt_base) - 1
    dd gdt_base

; 全局描述符
gdt_base:
    dd 0, 0; NULL，描述符0，默认为0
; gdt代码段
gdt_code:
    dw memory_limit & 0xffff; 段界限0~15位
    dw memory_base & 0xffff; 基地址 0 ~ 15 位
    db (memory_base >> 16) & 0xff; 基地址 16 ~ 23 位
    ; 存在-dlp 0 - S_ 代码段 - 非依从 - 可读 - 没有被访问过
    db 0b_1_00_1_1_0_1_0
    ; 4k - 32位 - 不是64位 - 段界限16~19
    db 0b1_1_0_0_0000 | (memory_limit >> 16) & 0xf
    db (memory_base >> 24) & 0xff ; 基地址 24~31位
; gdt数据段
gdt_data:
    dw memory_limit & 0xffff; 段界限0~15位
    dw memory_base & 0xffff; 基地址 0 ~ 15 位
    db (memory_base >> 16) & 0xff; 基地址 16 ~ 23 位
    ; 存在-dlp 0 - S_ 数据段 - 向上 - 可写 - 没有被访问过
    db 0b_1_00_1_0_0_1_0
    ; 4k - 32位 - 不是64位 - 段界限16~19
    db 0b1_1_0_0_0000 | (memory_limit >> 16) & 0xf
    db (memory_base >> 24) & 0xff ; 基地址 24~31位
gdt_end:

; 内存检测返回ards结构体的数量
ards_count:
    dw 0
; 内存检测返回ards结构体的缓存，放在最后，以免内存溢出
ards_buffer: