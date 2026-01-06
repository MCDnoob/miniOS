[bits 32]

section .text; 代码段

global inb; inb导出否则可重定向表无法找到inb函数，就无法链接
inb:
    push ebp
    mov ebp, esp; 保存栈帧 ebp = esp

    xor eax, eax; 清空eax寄存器
    mov edx, [ebp + 8]; 读取端口号参数到edx edx = [ebp + 8]
    in al, dx; 从端口号 dx 的 8 bit输入到al, al: eax的低8位

    ; 这里是可以实现无延迟的函数，这是一个性能优化点
    jmp $+2 ;一点点延迟，硬件操作是需要一些延迟
    jmp $+2
    jmp $+2

    leave
    ret

global outb
outb:
    push ebp
    mov ebp, esp; 保存栈帧 ebp = esp

    mov edx, [ebp + 8]; port
    mov eax, [ebp + 12]; value
    out dx, al; 将al的8 bit输出到端口号 dx

    ; 这里是可以实现无延迟的函数，这是一个性能优化点
    jmp $+2 ;一点点延迟，硬件操作是需要一些延迟
    jmp $+2
    jmp $+2

    leave
    ret

global inw; inb导出否则可重定向表无法找到inb函数，就无法链接
inw:
    push ebp
    mov ebp, esp; 保存栈帧 ebp = esp

    xor eax, eax; 清空eax寄存器
    mov edx, [ebp + 8]; 读取端口号参数到edx edx = [ebp + 8]
    in ax, dx; 从端口号 dx 的 16 bit输入到ax,ax:eax的低16位

    ; 这里是可以实现无延迟的函数，这是一个性能优化点
    jmp $+2 ;一点点延迟，硬件操作是需要一些延迟
    jmp $+2
    jmp $+2

    leave
    ret

global outw
outw:
    push ebp
    mov ebp, esp; 保存栈帧 ebp = esp

    mov edx, [ebp + 8]; port
    mov eax, [ebp + 12]; value
    out dx, ax; 将ax的16 bit输出到端口号 dx

    ; 这里是可以实现无延迟的函数，这是一个性能优化点
    jmp $+2 ;一点点延迟，硬件操作是需要一些延迟
    jmp $+2
    jmp $+2

    leave
    ret