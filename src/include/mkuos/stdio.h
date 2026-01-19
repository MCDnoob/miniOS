#ifndef MKUOS_STDIO_H
#define MKUOS_STDIO_H

#include <mkuos/stdarg.h>

int vsprintf(char *buf, const char *fmt, va_list args);
int sprintf(char *buf, const char *fmt, ...);

#endif