#include <regx52.h>
#include "IndependentKey.h"
#include "Delay.h"

#define GPIO_KEY P3

/**
 * @brief  扫描独立按键
 * @param  None
 * @return 按键值(16~23)，无按键按下时返回-1 
 */
int IndependentKeyDown() {
    int key = -1;
    GPIO_KEY = 0xFF; // 置高端口
    switch (GPIO_KEY) {
        case 0xFE: Delay(20); while(GPIO_KEY == 0xFE); Delay(20); key = 16; break;
        case 0xFD: Delay(20); while(GPIO_KEY == 0xFD); Delay(20); key = 17; break;
        case 0xFB: Delay(20); while(GPIO_KEY == 0xFB); Delay(20); key = 18; break;
        case 0xF7: Delay(20); while(GPIO_KEY == 0xF7); Delay(20); key = 19; break;
        case 0xEF: Delay(20); while(GPIO_KEY == 0xEF); Delay(20); key = 20; break;
        case 0xDF: Delay(20); while(GPIO_KEY == 0xDF); Delay(20); key = 21; break;
        case 0xBF: Delay(20); while(GPIO_KEY == 0xBF); Delay(20); key = 22; break;
        case 0x7F: Delay(20); while(GPIO_KEY == 0x7F); Delay(20); key = 23; break;
        default:   key = -1; break; // 无效按键
    }
    return key;
}