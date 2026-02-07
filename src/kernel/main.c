#include <mkuos/mkuos.h>
#include <mkuos/types.h>
#include <mkuos/io.h>
#include <mkuos/string.h>
#include <mkuos/console.h>
#include <mkuos/stdarg.h>
#include <mkuos/printk.h>
#include <mkuos/assert.h>
#include <mkuos/debug.h>
#include <mkuos/global.h>
#include <mkuos/task.h>
#include <mkuos/interrupt.h>
#include <mkuos/stdlib.h>

void kernel_init()
{
  console_init();
  gdt_init();
  interrupt_init();
  // task_init();
  /*
    __asm__ volatile(
        "sti\n" // 打开CPU中断
    );
    u32 counter = 0;
    while (true)
    {
      DEBUGK("looping in kernel init %d...\n", counter++);
      delay(100000000);
    }
  */
  return;
}