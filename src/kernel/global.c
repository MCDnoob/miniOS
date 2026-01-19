#include <mkuos/global.h>
#include <mkuos/string.h>
#include <mkuos/debug.h>

descriptor_t gdt[GDT_SIZE]; // 内核全局描述符表
pointer_t gdt_ptr;          // 内核全局描述符表指针

// 初始化全局描述符表
void gdt_init()
{
    DEBUGK("init GDT\n");

    __asm__ volatile("sgdt gdt_ptr"); // 将loader中的全局描述符表在GDTR中保存到指针gdt_ptr中

    // 将GDTR中的全局描述符表拷贝到gdt中
    memcpy((void *)&gdt, (void *)gdt_ptr.base, gdt_ptr.limit + 1);

    // 修改gdt的指针为内核gdt
    gdt_ptr.base = (u32)&gdt;
    gdt_ptr.limit = sizeof(gdt) - 1;

    // 再将gdt加载到GDTR
    __asm__ volatile("lgdt gdt_ptr");
}