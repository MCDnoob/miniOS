# assert

断言，作用
```C
assert(exp)
```
若exp为假，则终止程序并尽可能地打印错误信息。

```C
#define assert(exp) \
    if (exp)        \
        ;           \
    else            \
        assertion_failure(#exp, __FILE__, __BASE_FILE__, __LINE__)
```
`__FILE__`:出错的文件
`__BASE_FILE__`:好像和`__FILE__`没什么区别
`__LINE__`：出错的行数


这次实现没有真正的终止程序，而是直接阻塞：
```C
// 强制阻塞
static void spin(char *name)
{
    printk("spining in %s ...\n", name);
    while (true)
        ;
}
```