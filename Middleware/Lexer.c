/**
 * @file    Lexer.c
 * @author  严嘉哲
 * @brief   词法分析器实现文件，负责将输入字符转换为令牌流
 * @version 1.1
 * @date    2026-01-18
 */

#include "Lexer.h"
#include <ctype.h>

// ============================================================
// 1. 数据与定义
// ============================================================

// 事件类型
typedef enum {
    EVT_DIGIT,    // 0-9
    EVT_DOT,      // .
    EVT_MINUS,    // - (二义性符号)
    EVT_PLUS,     // +
    EVT_MUL,      // *
    EVT_DIV,      // /
    EVT_LPAREN,   // (
    EVT_RPAREN,   // )
    EVT_END,      // = (对应 TOK_END)
    EVT_OTHER     // 其他非法字符
} EventType;

typedef TokenType (*ActionFunc)(char key);

typedef struct {
    InputState cur_state;
    EventType  evt;
    InputState next_state;
    ActionFunc action;
} FSM_Item;

static f64 xdata current_val = 0.0;
static f64 xdata frac_scale = 0.1;
static s8  xdata val_sign = 1;
static InputState xdata fsm_state = STATE_IDLE;

// ============================================================
// 2. 动作回调 (更新返回值)
// ============================================================

// --- 数字类 (返回 TOK_NUM) ---
/** 
 * @brief  初始化数字输入
 * @param  key 输入字符
 * @return 返回 TOK_NUM
 */
static TokenType Act_InitNum(char key) {
    val_sign = 1; current_val = (f64)(key - '0');
    return TOK_NUM;
}

/** 
 * @brief  处理负号，初始化数字输入
 * @param  key 输入字符
 * @return 返回 TOK_NUM
 */
static TokenType Act_SetSign(char key) {
    key=0; val_sign = -1; current_val = 0.0;
    return TOK_NUM;
}

/** 
 * @brief  初始化小数点输入
 * @param  key 输入字符
 * @return 返回 TOK_NUM
 */
static TokenType Act_InitDot(char key) {
    key=0; val_sign = 1; current_val = 0.0; frac_scale = 0.1;
    return TOK_NUM;
}

/** 
 * @brief  在整数部分添加一位数字
 * @param  key 输入字符
 * @return 返回 TOK_NUM
 */
static TokenType Act_AddInt(char key) {
    current_val = current_val * 10.0 + (key - '0');
    return TOK_NUM;
}

/** 
 * @brief  在小数部分添加一位数字
 * @param  key 输入字符
 * @return 返回 TOK_NUM
 */
static TokenType Act_ToDot(char key) {
    key=0; frac_scale = 0.1;
    return TOK_NUM;
}

/** 
 * @brief  在小数部分添加一位数字
 * @param  key 输入字符
 * @return 返回 TOK_NUM
 */
static TokenType Act_AddFrac(char key) {
    current_val = current_val + ((key - '0') * frac_scale);
    frac_scale *= 0.1;
    return TOK_NUM;
}

/** 
 * @brief  忽略无效输入
 * @param  key 输入字符
 * @return 返回 TOK_NUM (保持状态)
 */
static TokenType Act_Ignore(char key) {
    key=0; return TOK_NUM;
}

// --- 算符类 (返回具体 Token) ---
/** 
 * @brief  处理加法运算符
 * @param  key 输入字符
 * @return 返回 TOK_ADD
 */
static TokenType Act_OpAdd(char key) { key=0; return TOK_ADD; }

/** 
 * @brief  处理减法运算符
 * @param  key 输入字符
 * @return 返回 TOK_SUB
 */
static TokenType Act_OpSub(char key) { key=0; return TOK_SUB; }

/** 
 * @brief  处理乘法运算符
 * @param  key 输入字符
 * @return 返回 TOK_MUL
 */
static TokenType Act_OpMul(char key) { key=0; return TOK_MUL; }

/** 
 * @brief  处理除法运算符
 * @param  key 输入字符
 * @return 返回 TOK_DIV
 */
static TokenType Act_OpDiv(char key) { key=0; return TOK_DIV; }

/** 
 * @brief  处理左括号运算符
 * @param  key 输入字符
 * @return 返回 TOK_LPAREN
 */
static TokenType Act_OpLPa(char key) { key=0; return TOK_LPAREN; }

/** 
 * @brief  处理右括号运算符
 * @param  key 输入字符
 * @return 返回 TOK_RPAREN
 */
static TokenType Act_OpRPa(char key) { key=0; return TOK_RPAREN; }

/** 
 * @brief  处理结束符运算符
 * @param  key 输入字符
 * @return 返回 TOK_END
 */
static TokenType Act_OpEnd(char key) { key=0; return TOK_END; } 

/** 
 * @brief  处理错误输入
 * @param  key 输入字符
 * @return 返回 TOK_ERROR
 */
static TokenType Act_Error(char key) { key=0; return TOK_ERROR; }

// ============================================================
// 3. FSM 表
// ============================================================
static FSM_Item code FSM_Table[] = {
    // ----------------------------------------------------------------------
    //  当前状态      事件          下个状态      动作
    // ----------------------------------------------------------------------
    
    // === 1. IDLE 态 (空闲/刚开始) ===
    // 拼数逻辑
    {STATE_IDLE,    EVT_DIGIT,    STATE_INT,    Act_InitNum},   // 0-9 -> 记数
    {STATE_IDLE,    EVT_DOT,      STATE_DOT,    Act_InitDot},   // .   -> 记数(0.)
    {STATE_IDLE,    EVT_MINUS,    STATE_INT,    Act_SetSign},   // -   -> 记数(负号)
    
    // 算符逻辑 (IDLE下直接返回算符，状态不变)
    {STATE_IDLE,    EVT_PLUS,     STATE_IDLE,   Act_OpAdd},
    {STATE_IDLE,    EVT_MUL,      STATE_IDLE,   Act_OpMul},
    {STATE_IDLE,    EVT_DIV,      STATE_IDLE,   Act_OpDiv},
    {STATE_IDLE,    EVT_LPAREN,   STATE_IDLE,   Act_OpLPa},
    {STATE_IDLE,    EVT_RPAREN,   STATE_IDLE,   Act_OpRPa},
    {STATE_IDLE,    EVT_END,      STATE_IDLE,   Act_OpEnd},

    // === 2. INT 态 (正在输整数) ===
    // 拼数逻辑
    {STATE_INT,     EVT_DIGIT,    STATE_INT,    Act_AddInt},    // 0-9 -> 累加
    {STATE_INT,     EVT_DOT,      STATE_DOT,    Act_ToDot},     // .   -> 切模式
    
    // 算符逻辑 (数字结束 -> 返回算符 -> 状态归零)
    {STATE_INT,     EVT_MINUS,    STATE_IDLE,   Act_OpSub},     // - 变减号!
    {STATE_INT,     EVT_PLUS,     STATE_IDLE,   Act_OpAdd},
    {STATE_INT,     EVT_MUL,      STATE_IDLE,   Act_OpMul},
    {STATE_INT,     EVT_DIV,      STATE_IDLE,   Act_OpDiv},
    {STATE_INT,     EVT_LPAREN,   STATE_IDLE,   Act_OpLPa},     // 12( -> 12 * ( ? 暂按普通处理
    {STATE_INT,     EVT_RPAREN,   STATE_IDLE,   Act_OpRPa},
    {STATE_INT,     EVT_END,      STATE_IDLE,   Act_OpEnd},

    // === 3. DOT 态 (刚输完点 "12.") ===
    // 拼数逻辑
    {STATE_DOT,     EVT_DIGIT,    STATE_FRAC,   Act_AddFrac},   // 0-9 -> 小数
    {STATE_DOT,     EVT_DOT,      STATE_DOT,    Act_Ignore},    // .   -> 忽略
    
    // 算符逻辑 (视为 12.0 处理)
    {STATE_DOT,     EVT_MINUS,    STATE_IDLE,   Act_OpSub},
    {STATE_DOT,     EVT_PLUS,     STATE_IDLE,   Act_OpAdd},
    {STATE_DOT,     EVT_MUL,      STATE_IDLE,   Act_OpMul},
    {STATE_DOT,     EVT_DIV,      STATE_IDLE,   Act_OpDiv},
    {STATE_DOT,     EVT_LPAREN,   STATE_IDLE,   Act_OpLPa},
    {STATE_DOT,     EVT_RPAREN,   STATE_IDLE,   Act_OpRPa},
    {STATE_DOT,     EVT_END,      STATE_IDLE,   Act_OpEnd},

    // === 4. FRAC 态 (正在输小数 "12.3") ===
    // 拼数逻辑
    {STATE_FRAC,    EVT_DIGIT,    STATE_FRAC,   Act_AddFrac},   // 0-9 -> 累加
    {STATE_FRAC,    EVT_DOT,      STATE_FRAC,   Act_Ignore},    // .   -> 忽略
    
    // 算符逻辑 (数字结束)
    {STATE_FRAC,    EVT_MINUS,    STATE_IDLE,   Act_OpSub},
    {STATE_FRAC,    EVT_PLUS,     STATE_IDLE,   Act_OpAdd},
    {STATE_FRAC,    EVT_MUL,      STATE_IDLE,   Act_OpMul},
    {STATE_FRAC,    EVT_DIV,      STATE_IDLE,   Act_OpDiv},
    {STATE_FRAC,    EVT_LPAREN,   STATE_IDLE,   Act_OpLPa},
    {STATE_FRAC,    EVT_RPAREN,   STATE_IDLE,   Act_OpRPa},
    {STATE_FRAC,    EVT_END,      STATE_IDLE,   Act_OpEnd},
};

#define TABLE_SIZE (sizeof(FSM_Table) / sizeof(FSM_Item))

// ============================================================
// 4. 驱动函数
// ============================================================
/**
 * @brief  根据输入字符获取事件类型
 * @param  key 输入字符
 * @return 事件类型枚举
 */
static EventType GetEventType(char key) {
    if (isdigit(key)) return EVT_DIGIT;
    switch(key) {
        case '.': return EVT_DOT;
        case '-': return EVT_MINUS; 
        case '+': return EVT_PLUS;
        case '*': return EVT_MUL;
        case '/': return EVT_DIV;
        case '(': return EVT_LPAREN;
        case ')': return EVT_RPAREN;
        case '=': return EVT_END; // = 是终结符事件
        default:  return EVT_OTHER; 
    }
}

/**
 * @brief  处理一个输入字符，驱动状态机
 * @param  key 输入字符
 * @return TokenType 生成的令牌类型
 */
TokenType Lexer_ProcessChar(char key) {
    EventType evt = GetEventType(key);
    u8 i;

    for (i = 0; i < TABLE_SIZE; i++) {
        if (FSM_Table[i].cur_state == fsm_state && FSM_Table[i].evt == evt) {
            TokenType token = FSM_Table[i].action(key);
            fsm_state = FSM_Table[i].next_state;
            return token;
        }
    }
    return Act_Error(key); // 未匹配到，返回错误
}

/**
 * @brief  获取当前拼凑的数字值
 * @param  无
 * @return f64 当前数字值
 */
f64 Lexer_GetCurrentVal(void) {
    return current_val * val_sign;
}

/**
 * @brief  获取当前状态机状态
 * @param  无
 * @return InputState 当前状态
 */
InputState Lexer_GetState(void) {
    return fsm_state;
}

/**
 * @brief  AC 全部重置状态机
 * @param  无
 * @return 无
 */
void Lexer_ResetAll(void) {
    current_val = 0.0;
    frac_scale = 0.1;
    val_sign = 1;
    fsm_state = STATE_IDLE;
}

/**
 * @brief  CE 清除当前数字输入状态
 * @param  无
 * @return 无
 */
void Lexer_ClearCurrent(void) {
    Lexer_ResetAll();
    // 此时状态变回 IDLE，如果 Main 的 Line1 还有符号，逻辑上也是合理的
}