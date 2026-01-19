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

void kernel_init()
{
  console_init();
  gdt_init();
  return;
}