#ifndef __LEXER_H__
#define __LEXER_H__

#include "Common.h"

// 词法分析器的处理结果
#define LEX_CONSUMED  1  // 字符被词法分析器吃掉了（是数字或部分）
#define LEX_REJECTED  0  // 字符被拒绝（是运算符，请 Main 处理）

// 暴露状态给 main 用于显示逻辑 (以及给 Lexer.c 定义表大小)
typedef enum {
    STATE_IDLE,    // 空闲/初始状态
    STATE_INT,     // 正在输入整数
    STATE_DOT,     // 刚输入小数点
    STATE_FRAC,    // 正在输入小数
    STATE_MAX      // 定义状态机数组的大小
} InputState;

// --- 核心接口 ---
TokenType   Lexer_ProcessChar(char key);
f64         Lexer_GetCurrentVal(void);
InputState  Lexer_GetState(void);
void        Lexer_ResetAll(void);
void        Lexer_ClearCurrent(void);

#endif