#include <mkuos/debug.h>
#include <mkuos/printk.h>
#include <mkuos/stdarg.h>
#include <mkuos/stdio.h>

static char buf[1024];

void debugk(char *file, int line, const char *fmt, ...)
{
    va_list args;
    va_start(args, fmt);
    vsprintf(buf, fmt, args);
    printk("[%s] [%d] %s", file, line, buf);
}