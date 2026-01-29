#ifndef __FLOAT2STR_H__
#define __FLOAT2STR_H__

#define PRECISION 6  // 小数位数

#include "Common.h"

/**
 * @brief 将double转换为字符串
 * @param f   传入的浮点数
 * @param buf 输出缓冲区的指针 (建议长度至少为15字节)
 */
void Double2String(f64 f, char *buf);

#endif