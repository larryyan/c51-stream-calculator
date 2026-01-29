#include "Double2Str.h"

/**
 * @brief 计算一个长整数的位数
 * @param num 输入整数
 * @return 位数 (例如 123 -> 3, 0 -> 1)
 */
static u16 get_int_length(long num) {
    u16 len = 0;
    if (num == 0) return 1;
    // 负数处理（虽然本逻辑中传入的都是绝对值，但保险起见）
    if (num < 0) num = -num; 
    
    while (num > 0) {
        num /= 10;
        len++;
    }
    return len;
}

/**
 * @brief 将 long 转换为字符串并追加到 buf
 * @param num 待转换的数字
 * @param buf 目标缓冲区指针
 * @param current_len 当前缓冲区长度指针 (会被更新)
 * @param min_width 最小显示宽度 (用于小数部分补前导0，整数部分填0即可)
 */
static void long_to_str(long num, char *buf, u16 *current_len, u16 min_width) {
    char xdata temp_buf[12]; // 临时存放翻转前的数字
    u16 temp_idx = 0;
    u16 i;

    // 1. 提取每一位
    if (num == 0) {
        // 如果是0，且要求宽度为0，则什么都不干；否则至少填一个0
        if (min_width == 0) min_width = 1;
        // 这里将在补零逻辑中统一处理
    } 
    
    while (num > 0) {
        temp_buf[temp_idx++] = (num % 10) + '0';
        num /= 10;
    }

    // 2. 补前导零 (对于小数部分 crucial)
    // 例如 frac=5, width=3 -> 应输出 "005"。当前 temp="5"，需要补2个0
    while (temp_idx < min_width) {
        temp_buf[temp_idx++] = '0';
    }
    
    // 如果原数是0且没进入循环，补一个0 (针对整数部分 num=0 的情况)
    if (temp_idx == 0) {
        temp_buf[temp_idx++] = '0';
    }

    // 3. 翻转并追加到主 buffer
    for (i = 0; i < temp_idx; i++) {
        buf[*current_len] = temp_buf[temp_idx - 1 - i];
        (*current_len)++;
    }
}

/**
 * @brief 去除字符串末尾多余的 '0' 和 '.'
 */
static void trim_zeros(char *buf, u16 *len) {
    // 只要最后一位是 '0'，就回退
    while (*len > 0 && buf[*len - 1] == '0') {
        (*len)--;
    }
    // 如果最后一位是小数点，也去掉
    if (*len > 0 && buf[*len - 1] == '.') {
        (*len)--;
    }
    // 添加结束符
    buf[*len] = '\0';
}

// ============================================================
// 主函数实现
// ============================================================
/**
 * @brief 将double转换为字符串 (支持有效数字逻辑)
 * @param f   传入的浮点数
 * @param buf 输出缓冲区的指针 (建议长度至少为15字节)
 */
void Double2String(f64 f, char *buf) {
    long scaled_val;
    long int_part;
    long frac_part;
    u16 len = 0;
    s16 decimal_places = 0;
    long multiplier = 1;
    u16 i;
    
    // 1. 0值处理
    if (f == 0.0) {
        buf[0] = '0'; buf[1] = '\0'; return;
    }

    // 2. 负号处理
    if (f < 0) {
        buf[len++] = '-';
        f = -f;
    }

    // 3. 计算小数位数 (核心修改部分)
    int_part = (long)f;
    
    if (int_part == 0) {
        // --- 针对纯小数 (0.xxxxx) 的特殊处理 ---        
        f64 temp_f = f;
        u16 leading_zeros = 0;
        
        // 统计小数点后紧跟的0的个数
        while (temp_f < 0.1 && leading_zeros < 8) {
            temp_f *= 10.0;
            leading_zeros++;
        }
        
        // 需要的小数位 = 要求的有效位数 + 前导零个数
        decimal_places = PRECISION + leading_zeros;
        
    } else {
        // --- 针对 >= 1.0 的数字，保持原有逻辑 ---
        u16 int_len = get_int_length(int_part);
        decimal_places = PRECISION - int_len;
    }

    // --- 边界与溢出保护 ---
    if (decimal_places < 0) decimal_places = 0;
    
    // C51 long (32位) 最大约为 21亿 (2.1 * 10^9)。
    // multiplier 最大只能是 10^9 (即9位小数)，否则计算 scaled_val 时会溢出。
    // 这里的限制稍微放宽一点到 8 或 9，以支持更精确的小数显示。
    if (decimal_places > 8) decimal_places = 8; 

    // 4. 计算乘数因子
    for (i = 0; i < decimal_places; i++) {
        multiplier *= 10;
    }

    // 5. 整体缩放 + 四舍五入
    // 注意：如果 f 很小 (0.000001)，multiplier 很大，乘积结果通常不会溢出 long
    scaled_val = (long)(f * multiplier + 0.5);

    // 6. 分离整数和小数
    int_part = scaled_val / multiplier;
    frac_part = scaled_val % multiplier;

    // 7. 转换整数部分
    long_to_str(int_part, buf, &len, 0);

    // 8. 处理小数部分
    if (decimal_places > 0) {
        buf[len++] = '.';
        // 传入 decimal_places 确保自动补前导零
        long_to_str(frac_part, buf, &len, decimal_places);
        
        // 9. 清理末尾零
        trim_zeros(buf, &len);
    } else {
        buf[len] = '\0';
    }
}