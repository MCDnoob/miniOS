#include <mkuos/task.h>
#include <mkuos/printk.h>
#include <mkuos/debug.h>
#include <mkuos/types.h>

#define PAGE_SIZE 0x1000 // 一页内存 4k

task_t *a = (task_t *)0x1000; // 任务a的栈基址
task_t *b = (task_t *)0x2000; // b的栈基址

extern void task_switch(task_t *next);

/*获得当前正在运行的任务*/
task_t *running_task()
{
    __asm__ volatile(
        "movl %esp, %eax\n"          // esp->eax
        "andl $0xfffff000, %eax\n"); // eax低12位清零，对齐到4k边界，为当前任务的栈基址
}

void schedule()
{
    task_t *current = running_task();
    task_t *next = current == a ? b : a;
    task_switch(next);
}
// 省略栈帧
u32 _ofp thread_a()
{
    // 为什么要打开中断？因为外中断处理过程中，中断是默认关闭的
    __asm__ volatile("sti\n"); // 打开中断
    while (true)
    {
        printk("A");
    }
}
// 省略栈帧
u32 _ofp thread_b()
{
    __asm__ volatile("sti\n"); // 打开中断
    while (true)
    {
        printk("B");
    }
}

static void task_create(task_t *task, target_t target)
{
    u32 stack = (u32)task + PAGE_SIZE; // task栈基址 + 4k得到栈最高地址(栈底)
    stack -= sizeof(task_frame_t);     // 栈顶向下移动(低地址方向)，留出栈帧空间
    task_frame_t *frame = (task_frame_t *)stack;
    frame->ebx = 0x11111111; // 初始化ebx寄存器，下面也是
    frame->esi = 0x22222222;
    frame->edi = 0x33333333;
    frame->ebp = 0x44444444;
    frame->eip = (void *)target; // 任务入口地址，相当于riscv的pc寄存器

    task->stack = (u32 *)stack; // 构造好的栈顶指针保存到任务控制块中
}

void task_init()
{
    task_create(a, thread_a);
    task_create(b, thread_b);
    schedule();
}