/**
 * @file Common.h
 * @brief 通用类型与宏定义
 */

#ifndef COMMON_H
#define COMMON_H

// 错误码定义
#define ERR_OK     0
#define ERR_SYNTAX 1
#define ERR_DIV0   2

// Lexer 处理结果
#define LEX_CONSUMED  1  // Lexer处理了这个字符 (它是数字或点)
#define LEX_REJECTED  0  // Lexer不认这个字符 (它是运算符)

// --- 变量类型简写 ---
typedef char           s8;
typedef int            s16;
typedef unsigned char  u8;
typedef unsigned int   u16;
typedef double         f64;

/**
 * @brief Token 类型枚举
 */
typedef enum {
    TOK_NUM,    // 数字 (例如 3.14, -5.0)
    TOK_ADD,    // +
    TOK_SUB,    // - (减法运算符)
    TOK_MUL,    // *
    TOK_DIV,    // /
    TOK_LPAREN, // (
    TOK_RPAREN, // )
    TOK_END,    // # (结束符)
    TOK_ERROR   // 未知字符
} TokenType;

/**
 * @brief Token 结构体定义
 */
typedef struct {
    TokenType type; // 类型决定了一切
    f64 value;   // 仅当 type == TOK_NUM 时有效
} Token;

#endif // COMMON_H