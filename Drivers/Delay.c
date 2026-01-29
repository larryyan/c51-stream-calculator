#include "Delay.h"

/**
  * @brief  延时函数
  * @param  xms: 延时参数，单位为毫秒
  * @retval None
  */
void Delay(unsigned int xms) {
	unsigned char data i, j;

	while(xms --) {
		i = 2;
		j = 239;
		do
		{
			while (--j);
		} while (--i);
	}
}