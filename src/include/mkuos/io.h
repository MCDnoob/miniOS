#ifndef MKUOS_IO_H
#define MKUOS_IO_H

#include <mkuos/types.h>

extern u8 inb(u16 port);    // 从指定端口输入一个字节，实际上，是从指定的端口port中读出8位来
extern u16 inw(u16 port);   // 从指定端口输入一个字，从指定的端口port中读出16位来

extern void outb(u16 port, u8 data);   // 输出一个字节到指定端口，自己写进port端口中8位data数据
extern void outw(u16 port, u16 data);  // 输出一个字到指定端口, 自己写进port端口中16位data数据

#endif