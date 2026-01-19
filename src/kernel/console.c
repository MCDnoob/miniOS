#include <mkuos/console.h>
#include <mkuos/io.h>
#include <mkuos/string.h>

#define CRT_ADDR_REG 0x3D4 // CRT(6845)索引寄存器
#define CRT_DATA_REG 0x3D5 // CRT数据寄存器

#define CRT_START_ADDR_H 0xC // 显示内存起始位置-高位
#define CRT_START_ADDR_L 0xD // 显示内存起始位置-低位
#define CRT_CURSOR_H 0xE     // 光标位置-高位
#define CRT_CURSOR_L 0xF     // 光标位置-低位

#define MEM_BASE 0xB8000              // 显卡内存起始位置（显卡做的屏幕内存映射）
#define MEM_SIZE 0x4000               // 显卡内存大小（显卡做的屏幕内存映射）
#define MEM_END (MEM_BASE + MEM_SIZE) // 显卡内存结束位置（显卡做的屏幕内存映射）
#define WIDTH 80                      // 屏幕文本列数
#define HEIGHT 25                     // 屏幕文本行数
#define ROW_SIZE (WIDTH * 2)          // 每行字节数,VGA模式，每个字符在显示屏幕上是两字节
#define SRC_SIZE (ROW_SIZE * HEIGHT)  // 屏幕字节数

#define ASCII_NUL 0x00
#define ASCII_ENQ 0x05
#define ASCII_BEL 0x07
#define ASCII_BS 0x08 // \a
#define ASCII_HT 0x09 // \b
#define ASCII_LF 0x0A // \n
#define ASCII_VT 0x0B // \v
#define ASCII_FF 0x0C // \f
#define ASCII_CR 0x0D // \r
#define ASCII_DEL 0x7F

// 屏幕内存一共4000字节，是由显卡映射的，最小的位置是0xB8000
static u32 screen; // 当前屏幕开始的内存位置，低于此内存位置的内容不在屏幕上显示，仅在当前开始位置向下显示4000字节
static u32 pos;    // 当前光标内存位置
static u32 x, y;   // 当前光标的坐标(WIDTH, HEIGHT)(0~80, 0~25)

static u8 attr = 7;        // 字符样式
static u16 erase = 0x0720; // 07：样式，0x20：空格，CPU小端存储，0x20在前面，7在后面，于是就是空格-样式

// 从硬件中获得当前显示器的开始位置
static void get_screen()
{
    outb(CRT_ADDR_REG, CRT_START_ADDR_H);
    screen = inb(CRT_DATA_REG) << 8;
    outb(CRT_ADDR_REG, CRT_START_ADDR_L);
    screen |= inb(CRT_DATA_REG);

    screen <<= 1;       // scrren*2, 每个字符都是两字节
    screen += MEM_BASE; // 加上基地址
}

// 设置屏幕内存位置
static void set_screen()
{
    // 正确写法：16位的字符序号，高8位右移8位，低8位直接取
    u16 screen_off = (screen - MEM_BASE) >> 1;
    outb(CRT_ADDR_REG, CRT_START_ADDR_H);
    // outb(CRT_DATA_REG, ((screen - MEM_BASE) >> 9) & 0xff);
    outb(CRT_DATA_REG, screen_off >> 8); // 高位
    outb(CRT_ADDR_REG, CRT_START_ADDR_L);
    // outb(CRT_DATA_REG, ((screen - MEM_BASE) >> 1) & 0xff);
    outb(CRT_DATA_REG, screen_off & 0xFF); // 低位
}

// 获得当前光标内存位置
static void get_cursor()
{
    outb(CRT_ADDR_REG, CRT_CURSOR_H);
    pos = inb(CRT_DATA_REG) << 8;
    outb(CRT_ADDR_REG, CRT_CURSOR_L);
    pos |= inb(CRT_DATA_REG);

    get_screen();

    pos <<= 1;
    pos += MEM_BASE;
    // 计算光标在显示器的绝对坐标
    u32 delta = (pos - screen) >> 1;
    x = delta % WIDTH;
    y = delta / WIDTH;
}

// 设置光标内存位置
static void set_cursor()
{
    u16 cursor_off = (pos - MEM_BASE) >> 1;

    outb(CRT_ADDR_REG, CRT_CURSOR_H);
    // outb(CRT_DATA_REG, ((pos - MEM_BASE) >> 9) & 0xff);
    outb(CRT_DATA_REG, cursor_off >> 8); // 光标地址高位
    outb(CRT_ADDR_REG, CRT_CURSOR_L);
    // outb(CRT_DATA_REG, ((pos - MEM_BASE) >> 1) & 0xff);
    outb(CRT_DATA_REG, cursor_off & 0xFF); // 光标地址低位
}

// 处理输入的ASCII-退格
static void command_bs()
{
    if (x)
    {
        x--;
        pos -= 2;
        *(u16 *)pos = erase;
    }
}

// 处理输入的ASCII-删除当前字符
static void command_del()
{
    if (x)
    {
        *(u16 *)pos = erase;
    }
}

// 处理输入的ASCII-光标回到行的开头
static void command_cr()
{
    pos -= (x << 1);
    x = 0;
}

// 向上滚一行
static void scroll_up()
{
    if (screen + SRC_SIZE + ROW_SIZE < MEM_END)
    {
        u32 *ptr = (u32 *)(screen + SRC_SIZE);
        for (size_t i = 0; i < WIDTH; ++i)
        {
            *ptr = erase;
            ptr++;
        }
        screen += ROW_SIZE; // 屏幕内存向下推一行
        pos += ROW_SIZE;    // 这里直接将pos粗暴的向下推一行，但是如果是换行，后面会有cr帮忙调整。
    }
    else
    {
        memcpy((void *)MEM_BASE, (void *)screen, SRC_SIZE); // 当前屏幕的内容拷贝到MEM_BASE
        pos -= (screen - MEM_BASE);                         // 光标位置回到从MEM_BASE出发屏幕内存的底部
        screen = MEM_BASE;
    }
    set_screen();
}

// 处理输入的ASCII-换行
static void command_lf()
{
    if (y + 1 < HEIGHT)
    {
        y++;
        pos += ROW_SIZE;
        return;
    }
    else
    {
        scroll_up();
    }
}

void console_clear()
{
    screen = MEM_BASE;
    pos = MEM_BASE;
    x = 0, y = 0;
    set_screen();
    set_cursor();

    // MEM_BASE->MEM_END字符全都设置为空格
    u16 *ptr = (u16 *)MEM_BASE;
    while (ptr < (u16 *)MEM_END)
    {
        *ptr = erase;
        ptr++;
    }
}

void console_write(char *buf, u32 count)
{
    char ch;
    while (count--)
    {
        ch = *buf;
        buf++;
        switch (ch)
        {
        case ASCII_NUL:
            break;
        case ASCII_ENQ:
            break;
        case ASCII_BEL:
            // todo: '/a'
            break;
        case ASCII_BS:
            command_bs();
            break;
        case ASCII_HT:
            break;
        case ASCII_LF:
            command_lf();
            command_cr();
            break;
        case ASCII_VT:
            break;
        case ASCII_FF:
            command_lf();
            break;
        case ASCII_CR:
            command_cr();
            break;
        case ASCII_DEL:
            command_del();
            break;
        default:
            if (x >= WIDTH)
            {
                x -= WIDTH;
                pos -= ROW_SIZE;
            }

            *((char *)pos) = ch;
            pos++;
            *((char *)pos) = attr;
            pos++;

            x++;
            break;
        }
    }
    set_cursor();
}

void console_init()
{
    console_clear();
}