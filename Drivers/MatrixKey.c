#include <regx52.h>
#include "Delay.h"
#include "MatrixKey.h"

#define GPIO_KEY P1


/**
 * @brief  扫描矩阵键盘
 * @param  None
 * @return 按键值(0~15)，无按键按下时返回-1 
 */
int MatrixKeyDown() {
    char a = 0;
    int KeyValue = -1;
	GPIO_KEY=0x0f;
	if(GPIO_KEY!=0x0f) 		//读取按键是否按下
	{
		Delay(20);			//延时20ms消抖动
		if(GPIO_KEY!=0x0f)  //再次检测键盘是否按下
		{	
			//测试列
			GPIO_KEY=0X0F;
			switch(GPIO_KEY)
			{
				case(0X07):	KeyValue=0;break;
				case(0X0b):	KeyValue=1;break;
				case(0X0d): KeyValue=2;break;
				case(0X0e):	KeyValue=3;break;
			}
			//测试行
			GPIO_KEY=0XF0;
			switch(GPIO_KEY)
			{
				case(0X70):	KeyValue=KeyValue;break;
				case(0Xb0):	KeyValue=KeyValue+4;break;
				case(0Xd0): KeyValue=KeyValue+8;break;
				case(0Xe0):	KeyValue=KeyValue+12;break;
			}
			
		}
	}
	while((a<20)&&(GPIO_KEY!=0xf0))	 //等待按键松开
	{
		Delay(5);
		a++;
	}
    return KeyValue;
}