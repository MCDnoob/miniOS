#ifndef ONIX_INTERRUPT_H
#define ONIX_INTERRUPT_H

#include <mkuos/types.h>

#define IDT_SIZE 256

/*中断描述符*/
typedef struct gate_t
{
    u16 offset0;    // 段内偏移0~15位
    u16 selector;   // 段选择子，cs位置
    u8 reserved;    // 保留位
    u8 type : 4;    // 任务门/中断门/陷阱门
    u8 segment : 1; // segment=0表示系统段
    u8 DPL : 2;     // 使用int指令访问的最低权限
    u8 present : 1; // 是否有效
    u16 offset1;    // 段内便宜16~31位
} _packed gate_t;

// 中断处理函数
typedef void *handler_t;

void interrupt_init();

#endif