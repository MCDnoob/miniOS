#include <mkuos/mkuos.h>
#include <mkuos/types.h>
#include <mkuos/io.h>
#include <mkuos/string.h>
#include <mkuos/console.h>
#include <mkuos/stdarg.h>
#include <mkuos/printk.h>
#include <mkuos/assert.h>

/*void test_args(int cnt, ...)
{
  va_list args;
  va_start(args, cnt);

  int arg;
  while (cnt--)
  {
    arg = va_arg(args, int);
  }
  va_end(args);
}*/

void kernel_init()
{
  console_init();
  // test_args(5, 1, 0xaa, 0x36, 7, 10);
  assert(3 < 5);
  // assert(3 > 5);
  panic("Out of Memory");
  return;
}