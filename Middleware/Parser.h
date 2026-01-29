#ifndef __PARSER_H__
#define __PARSER_H__
#include "Common.h"

#define MAX_STACK 20

// --- 核心接口 ---
void    Calc_Reset(void);           // 重置计算器状态
void    Calc_PushNum(f64 val);      // 压入一个数字
u8      Calc_PushOp(TokenType op);  // 压入一个运算符
f64     Calc_GetResult(void);       // 获取当前结果
char*   Calc_GetErrorMsg(void);     // 获取错误信息

#endif