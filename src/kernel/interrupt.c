#include <mkuos/interrupt.h>
#include <mkuos/global.h>
#include <mkuos/debug.h>
#include <mkuos/printk.h>
#include <mkuos/io.h>
#include <mkuos/stdlib.h>

#define LOGK(fmt, args...) DEBUGK(fmt, ##args)

#define ENTRY_SIZE 0x30
// 中断控制器的端口
#define PIC_M_CTRL 0x20 // 主片的控制端口
#define PIC_M_DATA 0x21 // 主片的数据端口
#define PIC_S_CTRL 0xa0 // 从片的控制端口
#define PIC_S_DATA 0xa1 // 从片的数据端口
#define PIC_EOI 0x20    // 通知中断控制器中断结束

gate_t idt[IDT_SIZE];
pointer_t idt_ptr;

handler_t handler_table[IDT_SIZE];                // handler_handler_0xxx的中断处理函数
extern handler_t handler_entry_table[ENTRY_SIZE]; // handler.asm

static char *messages[] = {
    "#DE Divide Error\0",
    "#DB RESERVED\0",
    "--  NMI Interrupt\0",
    "#BP Breakpoint\0",
    "#OF Overflow\0",
    "#BR BOUND Range Exceeded\0",
    "#UD Invalid Opcode (Undefined Opcode)\0",
    "#NM Device Not Available (No Math Coprocessor)\0",
    "#DF Double Fault\0",
    "    Coprocessor Segment Overrun (reserved)\0",
    "#TS Invalid TSS\0",
    "#NP Segment Not Present\0",
    "#SS Stack-Segment Fault\0",
    "#GP General Protection\0",
    "#PF Page Fault\0",
    "--  (Intel reserved. Do not use.)\0",
    "#MF x87 FPU Floating-Point Error (Math Fault)\0",
    "#AC Alignment Check\0",
    "#MC Machine Check\0",
    "#XF SIMD Floating-Point Exception\0",
    "#VE Virtualization Exception\0",
    "#CP Control Protection Exception\0",
};

/*通知中断控制器，中断处理结束*/
void send_eoi(int vector)
{
    if (vector >= 0x20 && vector < 0x28)
    {
        outb(PIC_M_CTRL, PIC_EOI);
    }
    if (vector >= 0x28 && vector < 0x30)
    {
        outb(PIC_M_CTRL, PIC_EOI);
        outb(PIC_S_CTRL, PIC_EOI);
    }
}

// u32 counter = 0;
extern void schedule();

void default_handler(int vector)
{
    send_eoi(vector);
    schedule();
    // LOGK("[%d] default interrupt called %d...\n", vector, counter++);
}

// 具体的中断处理函数，将栈中保存的上下文寄存器全都认为是参数传入函数
// 注意参数的顺序，其实就是栈从右向左压入参数的顺序，这个其实就是说明函数执行——
// 有参数列表——从栈中从左向右获取参数
void exception_handler(int vector,
                       u32 edi, u32 esi, u32 ebp, u32 esp,
                       u32 ebx, u32 edx, u32 ecx, u32 eax,
                       u32 gs, u32 fs, u32 es, u32 ds,
                       u32 vector0, u32 error, u32 eip, u32 cs, u32 eflags)
{
    char *message = NULL;
    if (vector < 22)
    {
        message = messages[vector];
    }
    else
    {
        message = messages[15];
    }

    printk("\nException : %s \n", messages[vector]);
    printk("   VECTOR : 0x%02X\n", vector);
    printk("    ERROR : 0x%08X\n", error);
    printk("    FLAGS : 0x%08X\n", eflags);
    printk("       CS : 0x%02X\n", cs);
    printk("      EIP : 0x%08X\n", eip);
    printk("      ESP : 0x%08X\n", esp);
    // 阻塞
    hang();
}

// 初始化中断控制器,这样初始化之后就会生成时钟中断
void pic_init()
{
    // 级联的第一个8259a PIC控制器
    outb(PIC_M_CTRL, 0b00010001); // ICW：边沿触发，级联8259，需要ICW4
    outb(PIC_M_DATA, 0x20);       // ICW2：起始端口号，也就是PIC发送给CPU INRT接口的中断向量号，因为x86保存的中断向量到0x20，所以就从0x20开始中断
    // 注意，0x22中断号是用来级联下面的8259a PIC控制器的，一般就不会用到了
    outb(PIC_M_DATA, 0b00000100); // ICW3: IR2接从片
    outb(PIC_M_DATA, 0b00000001); // ICW4: 8086模式，正常EOI

    // 级联的第二个8259a PIC控制器，一共16个外中断设备
    outb(PIC_S_CTRL, 0b00010001); // ICW1: 边沿触发，级联8259.需要ICW4
    outb(PIC_S_DATA, 0x28);       // ICW2: 起始端口号，因为是级联的两个8259a PIC，前面的是8个中断，所以后面的就从0x28开始
    outb(PIC_S_DATA, 2);          // ICW3 设置从片连接到主片的IR2引擎
    outb(PIC_S_DATA, 0b00000001); // 8086模式，正常EOIs

    outb(PIC_M_DATA, 0b11111110); // 关闭所有中断
    outb(PIC_S_DATA, 0b11111111); // 关闭所有中断
}

/*初始化中断描述符，和中断处理函数数组*/
void idt_init()
{
    for (size_t i = 0; i < IDT_SIZE; ++i)
    {
        gate_t *gate = &idt[i];
        handler_t handler = handler_entry_table[i];

        gate->offset0 = (u32)handler & 0xffff;
        gate->offset1 = ((u32)handler >> 16) & 0xffff;
        gate->selector = 1 << 3; // 代码段
        gate->reserved = 0;      // 保留不用
        gate->type = 0b1110;     // 中断门
        gate->segment = 0;       // 系统段
        gate->DPL = 0;           // 内核态，只有内核可以调用
        gate->present = 1;       // 有效
    }

    for (size_t i = 0; i < 0x20; ++i)
    {
        handler_table[i] = exception_handler;
    }

    for (size_t i = 0x20; i < ENTRY_SIZE; ++i)
    {
        handler_table[i] = default_handler;
    }

    idt_ptr.base = (u32)idt;
    idt_ptr.limit = sizeof(idt) - 1;
    __asm__ volatile("lidt idt_ptr\n"); // 保存IDT到寄存器，使中断描述符表有效
}

void interrupt_init()
{
    pic_init();
    idt_init();
}