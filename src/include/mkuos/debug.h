#ifndef MKUOS_DEBUG_H
#define MKUOS_DEBUG_H

void debugk(char *file, int line, const char *fmt, ...);

#define BMB __asm__ volatile("xchgw %bx, %bx") // bochs magic breakpoint
#define DEBUGK(fmt, args...) debugk(__BASE_FILE__, __LINE__, fmt, ##args)

#endif