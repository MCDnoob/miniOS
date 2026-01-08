#include <mkuos/string.h>

char *strcpy(char *dest, const char *src) 
{
    char *ptr = dest;
    while(true) 
    {
        *ptr = *src;
        if(*src == EOS)
            return dest;
        ptr++;
        src++;
    }
}

char *strncpy(char *dest, const char *src, size_t count)
{
    char *ptr = dest;
    size_t i;
    for(i = 0; i < count && *src != EOS; ++i) 
    {
        *ptr++ = *src;
    }

    // 剩余空间填充'\0'
    for(; i < count; ++i)
    {
        *ptr++ = EOS;
    }
    return dest;
}

char *strcat(char *dest, const char *src)
{
    char *ptr = dest;
    while(*ptr != EOS)
    {
        ptr++;
    }

    while(true)
    {
        *ptr = *src;
        if(*src == EOS)
            return dest;
        src++;
        ptr++;
    }
}

size_t strlen(const char *str)
{
    size_t res = 0;
    while(*str != EOS)
    {
        res++;
        str++;
    }
    return res;
}

int strcmp(const char *lhs, const char *rhs)
{
    while(true)
    {
        if(*lhs != *rhs)
            return (*lhs) - (*rhs);

        if(*lhs == EOS)
            return 0;

        lhs++;
        rhs++;
    }
}

char *strchr(const char *str, int ch)
{
    char target = (char)ch;
    char *res = NULL;
    while(true)
    {
        if(*str == target)
        {
            res = (char *)str;
            break;
        }
        if(*str == EOS)
            break;
        str++;
    }
    return res;
}

char *strrchr(const char *str, int ch)
{
    char target = (char)ch;
    char *res = NULL;
    while(true)
    {
        if(*str == target)
            res = (char *)str;
        if(*str == EOS)
            break;
        str++;
    }
    return res;
}

int memcmp(const void *lhs, const void *rhs, size_t count)
{
    if(count == 0)
        return 0;

    const u8 *l = (const u8 *)lhs;
    const u8 *r = (const u8 *)rhs;

    while(count-- > 0)
    {
        if(*l != *r)
            return (*l) - (*r);

        l++;
        r++;
    }
    return 0;
}

void *memset(void *dest, int ch, size_t count)
{
    if(count == 0)
        return dest;

    u8 *ptr = (u8 *)dest;
    for(size_t i = 0; i < count; ++i)
    {
        *ptr++ = (u8)ch;
    }
    return dest;
}

void *memcpy(void *dest, const void *src, size_t count)
{
    if(count == 0)
        return dest;

    u8 *ptr = (u8 *)dest;
    const u8 *s = (const u8 *)src;

    // 要处理内存重叠，如果dest在src后面，要反向拷贝
    if(ptr > s && ptr < s + count)
    {
        ptr += count - 1;
        s += count - 1;
        for(size_t i = 0; i < count; ++i)
        {
            *ptr-- = *s--;
        }
    }
    else
    {
        for(size_t i = 0; i < count; ++i)
        {
            *ptr++ = *s++;
        }
    }

    return dest;
}

void *memchr(const void *ptr, int ch, size_t count)
{
    u8 target = (u8)ch;
    
    void *res = NULL;
    
    const u8 *p = (const u8 *)ptr;
    
    for(size_t i = 0; i < count; ++i)
    {
        if(*p == target)
        {
            res = (void *)p;
            break;
        }
        p++;
    }
    return res;
}