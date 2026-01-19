#ifndef MKUOS_GLOBAL_H
#define MKUOS_GLOBAL_H

#include <mkuos/types.h>

#define GDT_SIZE 128

// 全局描述符
typedef struct descriptor_t /*8bytes, 64bits*/
{
    unsigned short limit_low;      // 段界限 0~15 位
    unsigned int base_low : 24;    // 基地址 0 ~ 23位 16M
    unsigned char type : 4;        // 段类型
    unsigned char segment : 1;     // 1 代码段或数据段。 0 系统段
    unsigned char DPL : 2;         // 描述符特权等级 0~3
    unsigned char present : 1;     // 存在位 1 在内存中， 0在磁盘上
    unsigned char limit_high : 4;  // 段界限16-19;
    unsigned char available : 1;   // 空闲位
    unsigned char long_mode : 1;   // 64位扩展标志
    unsigned char big : 1;         // 32位还是16位的标志;
    unsigned char granularity : 1; // 粒度4kb或1B
    unsigned char base_high;       // 基地址24-31位
} _packed descriptor_t;

// 段选择子
typedef struct selector_t /*2bytes, 16bits*/
{
    u8 PRL : 2;
    u8 TI : 1;
    u16 index : 13;
} selector_t;

// 全局描述符指针
typedef struct pointer_t
{
    u16 limit; // 大小
    u32 base;  // 全局描述符表基地址
} _packed pointer_t;

void gdt_init();

#endif