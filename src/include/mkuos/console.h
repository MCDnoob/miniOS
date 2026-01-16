#ifndef MKUOS_CONSOLE_H
#define MKUOS_CONSOLE_H

#include <mkuos/types.h>

void console_init();
void console_clear();
void console_write(char *buf, u32 count);

#endif