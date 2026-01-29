/**
 * @file    Parser.c
 * @author  严嘉哲
 * @brief   解析器实现文件，负责表达式的语法分析和计算
 * @version 1.0
 * @date    2026-01-17
 */
#include "Parser.h"

// 栈和状态
static f64 xdata val_stack[MAX_STACK];
static TokenType xdata op_stack[MAX_STACK];
static s8 val_top = -1;
static s8 op_top  = -1;
static u8 sys_error = ERR_OK;

// --- 内部堆栈操作 ---
static void push_val(f64 v) { if(val_top < MAX_STACK-1) val_stack[++val_top] = v; }
static f64 pop_val()        { return (val_top >= 0) ? val_stack[val_top--] : 0.0; }
static void push_op(TokenType t) { if(op_top < MAX_STACK-1) op_stack[++op_top] = t; }
static TokenType pop_op()   { return (op_top >= 0) ? op_stack[op_top--] : TOK_END; }
static TokenType peek_op()  { return (op_top >= 0) ? op_stack[op_top] : TOK_END; }

// --- 优先表逻辑 ---
static u8 get_idx(TokenType t) {
    switch(t) {
        case TOK_ADD: case TOK_SUB: return 0;
        case TOK_MUL: case TOK_DIV: return 1;
        case TOK_LPAREN: return 2;
        case TOK_RPAREN: return 3;
        case TOK_END:    return 4; // 把 = 看作终结符 #
        default: return 5;
    }
}

// 这里的表略微调整，让 = 号能触发所有计算
u8 code PriorityTable[5][5] = {
    //          +-   */   (    )    =
    /* +- */  {'G', 'L', 'L', 'G', 'G'},
    /* /  */  {'G', 'G', 'L', 'G', 'G'},
    /* (  */  {'L', 'L', 'L', 'E', 'X'},
    /* )  */  {'G', 'G', 'X', 'G', 'G'},
    /* =  */  {'L', 'L', 'L', 'X', 'E'} 
};

/**
 * @brief  执行一次计算，根据栈顶的两个操作数和一个运算符进行计算
 * @param  无
 * @return 无
 */
static void do_calculation() {
    f64 b, a, res = 0.0;
    TokenType op;
    if(val_top < 1) { sys_error = ERR_SYNTAX; return; }
    
    b = pop_val();
    a = pop_val();
    op = pop_op();
    
    switch(op) {
        case TOK_ADD: res = a + b; break;
        case TOK_SUB: res = a - b; break;
        case TOK_MUL: res = a * b; break;
        case TOK_DIV: 
            if(b == 0.0) sys_error = ERR_DIV0; 
            else res = a / b; 
            break;
    }
    push_val(res);
}

// --- 外部接口 ---
/**
 * @brief  重置计算器状态，清空操作数和运算符栈
 * @param  无
 * @return 无
 */
void Calc_Reset(void) {
    val_top = -1;
    op_top = -1;
    sys_error = ERR_OK;
    push_op(TOK_END); // 栈底放个 = (相当于之前的 #)
}

/**
 * @brief  将一个数字压入操作数栈
 * @param  val 数字值
 * @return 无
 */
void Calc_PushNum(f64 val) {
    push_val(val);
}

/**
 * @brief  将一个运算符压入运算符栈，根据优先级进行计算或移进
 * @param  input_op 输入的运算符
 * @return u8       操作是否成功，0 表示出错
 */
u8 Calc_PushOp(TokenType input_op) {
    u8 row, col, relation;
    
    if(sys_error != ERR_OK) return 0;

    while(1) {
        TokenType stack_top = peek_op();
        row = get_idx(stack_top);
        col = get_idx(input_op); // 注意 input_op 不变

        relation = PriorityTable[row][col];

        if (relation == 'L') { // < 移进
            push_op(input_op);
            return 1;
        } 
        else if (relation == 'G') { // > 归约 (计算)
            do_calculation();
            if(sys_error != ERR_OK) return 0;
            // 继续循环！比如栈里是 1+2*3，来了个+，先算*，再算+
        } 
        else if (relation == 'E') { // = 匹配
            if(stack_top == TOK_LPAREN) { // 脱括号
                pop_op();
                return 1;
            }
            if(stack_top == TOK_END && input_op == TOK_END) { // 算完了
                return 1;
            }
        } 
        else { // X 错误
            sys_error = ERR_SYNTAX; 
            return 0;
        }
    }
}

/**
 * @brief  获取计算结果
 * @param  无
 * @return f64 计算结果
 */
f64 Calc_GetResult(void) {
    if(val_top >= 0) return val_stack[val_top];
    return 0.0;
}

/**
 * @brief  获取当前错误信息字符串
 * @param  无
 * @return char* 错误信息字符串
 */
char* Calc_GetErrorMsg(void) {
    switch(sys_error) {
        case ERR_OK:     return "OK";
        case ERR_SYNTAX: return "Syntax Error";
        case ERR_DIV0:   return "Divided By Zero";
        default:         return "Error";
    }
}