#include <mkuos/mkuos.h>
#include <mkuos/types.h>
#include <mkuos/io.h>
#include <mkuos/string.h>
#include <mkuos/console.h>

char message[] = "hello mkuos!!!\n";
char buf[1024];

void kernel_init()
{
  console_init();
  while (true)
  {
    console_write(message, sizeof(message) - 1);
  }

  return;
}