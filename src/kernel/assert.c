#include <mkuos/assert.h>
#include <mkuos/stdio.h>
#include <mkuos/stdarg.h>
#include <mkuos/printk.h>
#include <mkuos/types.h>

static u8 buf[1024];

// 强制阻塞
static void spin(char *name)
{
    printk("spining in %s ...\n", name);
    while (true)
        ;
}

void assertion_failure(char *exp, char *file, char *base, int line)
{
    printk(
        "\n--> assert(%s) failed!\n"
        "-->file: %s \n"
        "-->base: %s \n"
        "-->line: %d \n",
        exp, file, base, line);

    spin("assertion_failure()");

    // 不可能走到这里，否则出错
    __asm__ volatile("ud2");
}

void panic(const char *fmt, ...)
{
    va_list args;
    va_start(args, fmt);
    int i = vsprintf(buf, fmt, args);
    va_end(args);

    printk("!!! panic !!!\n-->%s \n", buf);
    spin("panic()");

    __asm__ volatile("ud2");
}