#ifndef MKUOS_STDARG_H
#define MKUOS_STDARG_H

typedef char *va_list;

// v的地址+word，指向下一个参数的位置(参数压栈从右向左，32位机器默认参数占栈大小4bytes，64位则是8bytes)
#define va_start(ap, v) (ap = (va_list) & v + sizeof(char *))
/*
1. ap += sizeof(char *))栈游标指针先向下移动栈帧
2. (ap+=...) - sizeof(char *) ap所在的前一个栈帧的位置
3. (t *) (...) 强制类型转换
4. *(t *) (...) → 解引用取值：拿到这个地址里真正的参数值；
为什么先加后减，而不是更简单的先减后加：
1.C 语言的「宏的语法规则」—— 宏必须是「单个表达式」，不能写多条语句
2. 先加后减一次完成是原子性的、一步到位
注：
这个宏的标准版是：
#define va_arg(ap, t) (*(t *)((ap += sizeof(t)) - sizeof(t)))
这个宏是32 位内核专用简化版，用sizeof(char*)是因为 32 位所有参数都是 4 字节，等价于标准版
*/
#define va_arg(ap, t) (*(t *)((ap += sizeof(char *)) - sizeof(char *)))
#define va_end(ap) (ap = (va_list)0)

#endif