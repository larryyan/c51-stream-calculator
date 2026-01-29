/**
 * @file    main.c
 * @author  严嘉哲
 * @brief   51单片机计算器主程序
 * @version 1.1
 * @date    2026-01-18
 */

// 运算库
#include <string.h>
#include <ctype.h>
#include <regx52.h>
// 外设库
#include "Drivers/LCD1602.h"
#include "Drivers/Delay.h"
#include "Drivers/Buzzer.h"
#include "Drivers/HappyBrithday.h"
#include "Drivers/MatrixKey.h"
#include "Drivers/IndependentKey.h"
// 中间件
#include "Middleware/Common.h"
#include "Middleware/Parser.h"
#include "Middleware/Lexer.h"
#include "Middleware/Double2Str.h"

/**
 * @brief 键盘按键映射表
 */
u8 code KeyTable[] = {
    '7', '8', '9', '/', 
    '4', '5', '6', '*', 
    '1', '2', '3', '-',
    'D', '0', '.', '+',     // D:Double Zero
    '(', ')', '%', '=',
    'A', 'C', 'H', 'B'      // A:AC, C:CE, H:HappyBrithday, B:Backspace
};

// 显示缓存
#define MAX_FORMULA_LEN  30
#define LCD_WIDTH        16

static char xdata Line1_Buf[MAX_FORMULA_LEN + 1]; 
static u8   xdata Line1_Len = 0;
static u8   xdata last_op_index = 0; // 记录上一个 Token 结束的位置 (用于 CE/BS 回退)
static char xdata Line2_Buf[LCD_WIDTH + 2]; 

static bit is_calculated = 0;   // 标记是否刚计算完结果
static bit lexer_was_busy = 0;  // 标记处理按键前，Lexer 是否持有数字

/**
 * @brief  显示与辅助函数
 */
void Update_Line1() {
    LCD_ShowString(1, 1, "                ");
    if (Line1_Len <= LCD_WIDTH) {
        LCD_ShowString(1, 1, Line1_Buf);
    } else {
        LCD_ShowString(1, 1, Line1_Buf + Line1_Len - LCD_WIDTH);
    }
}

/**
 * @brief  更新第二行的当前输入预览
 * @param  无
 * @return 无
 */
void Update_Line2_Input() {
    // 实时预览 Lexer 里的数值
    Double2String(Lexer_GetCurrentVal(), Line2_Buf);
    
    // 视觉优化：正在输小数时，手动补个点
    if (Lexer_GetState() == STATE_DOT) {
        strcat(Line2_Buf, ".");
    }
    
    LCD_ShowString(2, 1, "                ");
    LCD_ShowString(2, 1, Line2_Buf);
}

/**
 * @brief  在 Line1 末尾追加一个字符并更新显示
 * @param  c 追加的字符
 * @return 无
 */
void Line1_Append(char c) {
    if (Line1_Len < MAX_FORMULA_LEN) {
        Line1_Buf[Line1_Len++] = c;
        Line1_Buf[Line1_Len] = '\0';
        Update_Line1();
    }
}

/**
 * @brief  从上一个运算符位置重放 Lexer 状态
 * @param  无
 * @return 无
 */
void Replay_Lexer_From_Op() {
    u8 i;
    Lexer_ClearCurrent(); 
    for (i = last_op_index; i < Line1_Len; i++) {
        Lexer_ProcessChar(Line1_Buf[i]); 
    }
}

/**
 * @brief  系统重置函数 (AC)
 * @param  无
 * @return 无
 */
void System_Reset() {
    Calc_Reset();
    Lexer_ResetAll();
    
    Line1_Len = 0;
    last_op_index = 0;
    Line1_Buf[0] = '\0';
    
    Update_Line1();
    LCD_ShowString(2, 1, "0               ");
    
    is_calculated = 0;
}

/**
 * @brief  主按键处理函数
 * @param  key 按键字符
 * @return 无
 */
void OnKeyPress(char key) {
    TokenType token;

    if (is_calculated) {    // 结果态逻辑
        // 输入数字/点 -> 全局重置，开始新计算
        if (isdigit(key) || key == '.' || key == 'D') {
             System_Reset();
        }
        // 输入符号/CE/BS -> 保留结果，继续操作
        else if (key != 'C' && key != 'B') {
             last_op_index = Line1_Len;
             is_calculated = 0;
             // 注意：Parser 栈里此时已经有结果 Result 了，
             // 不需要额外 PushNum，直接接 PushOp 即可。
        }
    }

    // CE: 清除当前输入 (只清数字)
    if (key == 'C') {
        // 只有当 Lexer 正在拼数时，CE 才有效 (保护运算符不被误删)
        if (Lexer_GetState() != STATE_IDLE) {
            Lexer_ClearCurrent();       // 逻辑层清空
            Line1_Len = last_op_index;  // 视觉层回退到符号后
            Line1_Buf[Line1_Len] = '\0';
            Update_Line1();
            Update_Line2_Input();       // 显示 0
        }        
        return;
    }

    // BS: 退格 (删除数字的最后一位)
    if (key == 'B') {
        // 只有在拼数且有字符可删时有效
        if (Lexer_GetState() != STATE_IDLE && Line1_Len > last_op_index) {
            // 1. 视觉回退
            Line1_Len--;
            Line1_Buf[Line1_Len] = '\0';
            Update_Line1();
            
            // 2. 逻辑同步 (重解析策略)
            Replay_Lexer_From_Op();
            Update_Line2_Input();
        }
        return;
    }

    /* Phase 2: 词法分析 (Token Generation) */
    // 记录按键前 Lexer 状态，以判断是否有数字待压栈
    lexer_was_busy = (Lexer_GetState() != STATE_IDLE);

    // 让 Lexer 决定这是什么 Token
    token = Lexer_ProcessChar(key);

    /* Phase 3: Token 分发与处理 */
    switch (token) {
        // --- 情况 A: 正在拼凑数字 (Number) ---
        case TOK_NUM:
            Line1_Append(key);
            Update_Line2_Input();
            break;
        // --- 情况 B: 终结符 (Terminator, 即 =) ---
        case TOK_END:
            // 1. 如果前面有数字，先压栈
            if (lexer_was_busy) {
                Calc_PushNum(Lexer_GetCurrentVal());
            }
            // 2. 压入终结符，触发 Parser 的最终归约
            if (Calc_PushOp(TOK_END)) {
                // 3. 获取结果并显示
                f64 res = Calc_GetResult();
                Line1_Append('=');
                
                // 格式化结果到第二行
                Line2_Buf[0] = '=';
                Double2String(res, Line2_Buf + 1);
                
                // 右对齐显示
                LCD_ShowString(2, 1, "                ");
                LCD_ShowString(2, 17 - strlen(Line2_Buf), Line2_Buf);
                
                // 4. 将结果回写到 Line1 缓存 (为下一次连续计算做准备)
                strcpy(Line1_Buf, Line2_Buf + 1);
                Line1_Len = strlen(Line1_Buf);
                
                is_calculated = 1;
                Lexer_ResetAll(); // 确保 Lexer 归位
            } else {
                // 语法错误 (例如 1+*)
                Line1_Append('=');
                LCD_ShowString(2, 1, Calc_GetErrorMsg());
                is_calculated = 1;
            }
            break;
        // --- 情况 C: 普通运算符 (+ - * / ( )) ---
        case TOK_ADD:
        case TOK_SUB:
        case TOK_MUL:
        case TOK_DIV:
        case TOK_LPAREN:
        case TOK_RPAREN:
            // 1. 如果前面有数字，先压栈
            if (lexer_was_busy) {
                Calc_PushNum(Lexer_GetCurrentVal());
            }

            // 2. 压入运算符
            if (Calc_PushOp(token)) {
                Line1_Append(key);
                last_op_index = Line1_Len; // 更新符号位置，供 CE/BS 使用
                
                // 3. 辅助显示
                LCD_ShowString(2, 1, "OP:             ");
                LCD_ShowChar(2, 5, key);                
                // 注意：Lexer 返回 Operator 时已自动 Reset，无需手动 Clear
            } else {
                // 压栈失败 (语法错误)
                Line1_Append(key);
                LCD_ShowString(2, 1, Calc_GetErrorMsg());
                is_calculated = 1;
            }
            break;
        // --- 情况 D: 错误/未知（Error/Unknown）忽略 ---
        case TOK_ERROR:
        default: break;         // 忽略无效按键
    }
}

void main() {
    int key_val;
    char k;
    
    LCD_Init();
    Buzzer_Init();
    
    // 显示startup信息
    LCD_ShowString(1, 4, "Calculator");
    LCD_ShowString(2, 11, "By YJZ");
    Delay(1000);
    System_Reset(); 
    
    while(1) {
        // 扫描矩阵按键
        key_val = MatrixKeyDown();
        // 扫描独立按键
        if(key_val < 0) key_val = IndependentKeyDown();
        if(key_val < 0) continue; // 无按键
        
        Buzzer_KeySound(key_val);

        k = KeyTable[key_val];
        
        // 快捷键处理
        if (k == 'D') {         // Double Zero (00)
            OnKeyPress('0'); OnKeyPress('0');
        } else if(k == '%') {   // 百分号 (除以100)
            OnKeyPress('/'); OnKeyPress('1'); OnKeyPress('0'); OnKeyPress('0');
        } else if(k == 'H') {   // Happy Birthday 彩蛋
            HappyBrithday();
        } else if(k == 'A') {   // AC 全部重置
            System_Reset();
        } else {                // 标准按键处理
            OnKeyPress(k);
        }
    }
}