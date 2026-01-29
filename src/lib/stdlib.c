#include <mkuos/stdlib.h>

/*阻塞count个时钟周期*/
void delay(u32 count)
{
    while (count--)
        ;
}
/*阻塞*/
void hang()
{
    while (true)
        ;
}