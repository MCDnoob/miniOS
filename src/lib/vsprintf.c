#include <mkuos/stdarg.h>
#include <mkuos/string.h>
#include <mkuos/stdio.h>

#define ZEROPAD 0x01 // 填充0
#define SIGN 0x02    // unsigned/signed long
#define PLUS 0x04    // 显示加
#define SPACE 0x08   // 如是加，则置空格
#define LEFT 0x10    // 左调整
#define SPECIAL 0x20 // 0x
#define SMALL 0x40   // 使用小写字母
#define DOUBLE 0x80  // 浮点数

#define is_digit(c) ((c) >= '0' && (c) <= '9')
/*
将字符数字串转换成整数，并将指针后移，如传入"123abc"，输出123并将原本的指针指向'a'
二维指针是因为要调整原本的指针的指向，一维指针是值传递，不会改变原本指针的指向。
*/
static int skip_atoi(const char **s)
{
    int i = 0;
    while (is_digit(**s))
    {
        i = i * 10 + *((*s)++) - '0';
    }
    return i;
}

/*
将整数转换为指定进制的字符串
str - 存储输出字符串指针
num - 带转换的数值指针，支持整数、浮点数，通过DOUBLE区分
base - 转换的进制
size - 最终输出的字符串总宽度
precision - 数值部分的精度
flags - 选项，格式控制标志
输出：str指针，指向填充完成后的下一个字符位置
*/
static char *number(char *str, u32 *num, int base, int size, int precision, int flags)
{
    char pad, sign, tmp[36];
    const char *digits = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ";
    int i;
    int index;
    char *ptr = str;

    // 如果flags指出用小写字母，则定义小写字母集合
    if (flags & SMALL)
    {
        digits = "0123456789abcdefghijklmnopqrstuvwxyz";
    }

    // flags 指出要左对齐，则屏蔽类型中的填零标志
    if (flags & LEFT)
    {
        flags &= ~ZEROPAD;
    }

    // 如果进制基数小于2或大于36，则退出处理。本程序只能处理基数在2~36之间的数
    if (base < 2 || base > 36)
        return 0;

    // flags指出要填零，则置字符变量pad='0'，否则pad等于空格
    pad = (flags & ZEROPAD) ? '0' : ' ';

    // 如果flags指出是带符号的数并且num小于0， 则置符号变量 sign=负号，并使num取绝对值
    if (flags & DOUBLE && (*(double *)num) < 0)
    {
        sign = '-';
        (*num) = -(int)(*num);
    }
    else if (flags & SIGN && !(flags & DOUBLE) && ((int)(*num)) < 0)
    {
        // 带符号，不为DOUBLE，且数值小于零，则sign = '-',数值取绝对值
        sign = '-';
        (*num) = -(int)(*num);
    }
    else // 如果flags指出是加号，则置sign=加号，否则若类型带空格标志，则sign=空格，否则置0
    {
        sign = (flags & PLUS) ? '+' : ((flags & SPACE) ? ' ' : 0);
    }

    // 若带符号，则宽度值减一
    if (sign)
        size--;

    // 若flags指出是特殊转换，对于16进制宽度减少两位(0x)，8进制减少一位(0)
    if (flags & SPECIAL)
    {
        if (base == 16)
            size -= 2;
        else if (base == 8)
            size -= 1;
    }

    i = 0;

    // 如果num =0，则临时字符串 = '0'；否则根据给定的基数将数值num转换成字符形式
    if (flags & DOUBLE)
    {
        u32 ival = (u32)(*(double *)num);
        u32 fval = (u32)(((*(double *)num) - ival) * 1000000);

        int mantissa = 6;
        while (mantissa--)
        {
            index = (fval) % base;
            (fval) /= base;
            tmp[i++] = digits[index];
        }

        tmp[i++] = '.';
        do
        {
            index = (ival) % base;
            (ival) /= base;
            tmp[i++] = digits[index];
        } while (ival);
    }
    else if ((*num) == 0)
    {
        tmp[i++] = '0';
    }
    else
    {
        while ((*num) != 0)
        {
            index = (*num) % base;
            (*num) /= base;
            tmp[i++] = digits[index];
        }
    }

    // 若字符数大于精度值，则精度值扩展为数字个数值
    if (i > precision)
        precision = i;

    // 宽度值size减去用于存放数值字符的个数
    size -= precision;

    // 形成所需要的转换结果，暂存在字符串str中
    // 若flags中没有ZEROPAD和左对齐LEFT，则在str中首席按填放剩余宽度值指出的空格数
    if (!(flags & (ZEROPAD + LEFT)))
        while (size-- > 0)
            *str++ = ' ';

    // 若需带符号位，则存入符号
    if (sign)
        *str++ = sign;

    // 若flags指出是特殊转换
    if (flags & SPECIAL)
    {
        if (base == 8)
            *str++ = '0';
        else if (base == 16)
        {
            *str++ = '0';
            *str++ = digits[33];
        }
    }

    // 若flags中没有LEFT，则在剩余宽度中存放c字符
    if (!(flags & LEFT))
    {
        while (size-- > 0)
        {
            *str++ = pad;
        }
    }

    // 此时i存有数值num的数字个数
    // 若数字个数小于精度值，则str中放入 (精度值-i) 个'0'
    while (i < precision--)
    {
        *str++ = '0';
    }

    // 将数值转换好的数字字符填入str中，共 i 个
    while (i-- > 0)
        *str++ = tmp[i];

    // 若宽度值仍大于零
    // 则表示flags标志中有左对齐标志, 则在剩余宽度中放入空格
    while (size-- > 0)
        *str++ = ' ';

    return str;
}

/*
buf - 目标缓冲区字符串，存储输出字符
fmt - 格式控制字符串，包含普通字符和格式占位字符(%d, %x...)
args - 可变参数列表
return value - 吸入buf的总字符数(不计'\0')
*/
int vsprintf(char *buf, const char *fmt, va_list args)
{
    int len;
    int i;

    // 存放转换过程中的字符串
    char *str;
    char *s;
    int *ip;

    // number使用的标志位
    int flags;

    int field_width; // 输出字段宽度
    int precision;   // min整数数字个数；max字符串字符个数
    int qualifier;   // 'h', 'l'或'L'用于整个整数字段
    u32 num;
    u8 *ptr;

    /*
    字符指针指向buf
    扫描格式字符串，对各个格式转换指示进行相应处理
    */
    for (str = buf; *fmt; ++fmt)
    {
        /*
        格式转换指示字符以 % 开始
        这里从fmt格式字符串中扫描 % ，寻找格式转换字符串的开始
        不是格式指示的一般字符都放入str中
        */
        if (*fmt != '%')
        {
            *str++ = *fmt;
            continue;
        }

        // 下面遇到了格式字符 %，将标志常量放入flags变量中
        flags = 0;
    repeat:
        // 跳过%
        ++fmt;
        switch (*fmt)
        {
        case '-':
            flags |= LEFT;
            goto repeat;
        case '+':
            flags |= PLUS;
            goto repeat;
        case ' ':
            flags |= SPACE;
            goto repeat;
        case '#':
            flags |= SPECIAL;
            goto repeat;
        case '0':
            flags |= ZEROPAD;
            goto repeat;
        }

        // 取当前参数字段宽度域值，放入filed_width变量中
        field_width = -1;

        // 如果宽度域中是数值则直接取其为宽度值
        if (is_digit(*fmt))
            field_width = skip_atoi(&fmt);
        else if (*fmt == '*') // 如果宽度域中是字符 * ，表示下一个参数指定宽度
        {
            ++fmt;
            // 调用va_arg取宽度值
            field_width = va_arg(args, int);

            // 若宽度值小于0，则该负数表示其带有标志域 '-' 标志(左对齐)
            if (field_width < 0)
            {
                field_width = -field_width;
                flags |= LEFT;
            }
        }

        // 取格式转换串的精度域，放入 precision中
        precision = -1;

        // 精度域开始的标志是 '.'，处理过程与宽度域类似
        if (*fmt == '.')
        {
            ++fmt;
            // 精度域中是数值直接取数值
            if (is_digit(*fmt))
                precision = skip_atoi(&fmt);
            else if (*fmt == '*') // 字符'*'表示下一个参数指定精度
            {
                precision = va_arg(args, int);
            }
            // 如果精度值小于0，设置为绝对值/0
            if (precision < 0)
                precision = 0;
        }

        // 分析长度修饰符，存入qualifer
        qualifier = -1;
        if (*fmt == 'h' || *fmt == 'l' || *fmt == 'L')
        {
            qualifier = *fmt;
            ++fmt;
        }

        // 分析转换指示符
        switch (*fmt)
        {
        case 'c': // 表示对应参数应是字符
            // 若此时不是左对齐
            if (!(flags & LEFT))
            {
                // 则该字符前面放入(宽度域值-1)个空格字符，然后放入参数字符
                while (--field_width > 0)
                    *str++ = ' ';
            }
            *str++ = (unsigned char)va_arg(args, int);
            // 若宽度域还大于0，则表示为左对齐，那么再参数字符后面添加(宽度值-1)个空格字符
            while (--field_width > 0)
                *str++ = ' ';
            break;
        case 's': // 表示对应参数是字符串
            s = va_arg(args, char *);
            // 取对应参数字符串的长度
            len = strlen(s);

            if (precision < 0)
                precision = len;
            else if (len > precision)
                len = precision;

            // 此时标志域表示不是左对齐
            if (!(flags & LEFT))
                // 则该字段前放入宽度值-字符串长度个空格字符
                while (len < field_width--)
                    *str++ = ' ';
            // 然后再放入参数字符串
            for (i = 0; i < len; ++i)
                *str++ = *s++;
            // 如果宽度值还大于0，则表示为左对齐
            // 那么在参数字符粗换后面，添加宽度值-字符串长度个空格字符
            while (len < field_width--)
                *str++ = ' ';
            break;
        case 'o': // 表示需将对应的参数转换成八进制字符串
            num = va_arg(args, unsigned long);
            str = number(str, &num, 8, field_width, precision, flags);
            break;
        case 'p': // 指针类型
            // 默认宽度值为8，并且需要添加零
            if (field_width == -1)
            {
                field_width = 8;
                flags |= ZEROPAD;
            }
            num = va_arg(args, unsigned long);
            str = number(str, &num, 16, field_width, precision, flags);
            break;

        case 'x': // 16进制输出
            flags |= SMALL;
        case 'X':
            num = va_arg(args, unsigned long);
            str = number(str, &num, 16, field_width, precision, flags);
            break;

        case 'd':
        case 'i': // d, i代表符号整数，需要加上符号标志
            flags |= SIGN;
        case 'u': // 无符号整数
            num = va_arg(args, unsigned long);
            str = number(str, &num, 10, field_width, precision, flags);
            break;

        case 'n': // 表示要把目前位置转换输出的字符数保存到对应参数指针指定的位置中
            ip = va_arg(args, int *);
            *ip = (str - buf);
            break;

        case 'f':
            flags |= SIGN;
            flags |= DOUBLE;
            double dnum = va_arg(args, double);
            str = number(str, (u32 *)&dnum, 10, field_width, precision, flags);
            break;

        case 'b': // binary
            num = va_arg(args, unsigned long);
            str = number(str, &num, 2, field_width, precision, flags);
            break;

        case 'm': // mac address
            flags |= SMALL | ZEROPAD;
            ptr = va_arg(args, char *);
            for (size_t t = 0; t < 6; t++, ptr++)
            {
                int num = *ptr;
                str = number(str, &num, 16, 2, precision, flags);
                *str = ':';
                str++;
            }
            str--;
            break;
        case 'r': // ip address
            flags |= SMALL;
            ptr = va_arg(args, u8 *);
            for (size_t t = 0; t < 4; t++, ptr++)
            {
                int num = *ptr;
                str = number(str, &num, 10, field_width, precision, flags);
                *str = '.';
                str++;
            }
            str--;
            break;
        default:
            // 若格式转换符不是 % 则表示格式字符串有错
            if (*fmt != '%')
                // 将 % 写入输出串
                *str++ = '%';
            // 若格式转换符的位置处还有字符，则也直接将其写入输出串
            // 然后继续循环处理格式字符串
            if (*fmt)
                *str++ = *fmt;
            else
                --fmt; // 已经处理到格式字符串结尾，退出循环
            break;
        }
    }

    // 字符串结束符标志
    *str = '\0';
    // 返回转换好的字符串长度
    i = str - buf;
    return i;
}

int sprintf(char *buf, const char *fmt, ...)
{
    va_list args;
    va_start(args, fmt);
    int i = vsprintf(buf, fmt, args);
    va_end(args);
    return i;
}