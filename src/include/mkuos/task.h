#ifndef MKUOS_TASK_H
#define MKUOS_TASK_H

#include <mkuos/types.h>

typedef u32 target_t();

typedef struct task_t
{
    u32 *stack; // 内核栈
} task_t;

// 要保存的寄存器的值，在切换之前保存，切换之后恢复
typedef struct task_frame_t
{
    u32 edi;
    u32 esi;
    u32 ebx;
    u32 ebp;
    void (*eip)(void);
} task_frame_t;

void task_init();

#endif