global task_switch
task_switch:
    ; 保存栈帧
    push ebp
    mov ebp, esp

    ; 保存寄存器ebx,esi,edi压入栈中(非易失性寄存器)
    push ebx
    push esi
    push edi

    ; 找到当前人物的tcb基址，并保存当前栈顶指针到tcb
    mov eax, esp       ; esp->eax
    and eax, 0xfffff000; 任务栈基址
    mov [eax], esp     ; 当前esp栈顶保存到当前TCB中([eax] = TCB基址的内容)

    ; 加载下一个任务的栈顶指针，切换esp
    mov eax, [ebp + 8]; next
    mov esp, [eax]  ;next人物的栈顶指针([next])赋值给esp，切换栈

    ; 恢复寄存器
    pop edi
    pop esi
    pop ebx
    pop ebp

    ret